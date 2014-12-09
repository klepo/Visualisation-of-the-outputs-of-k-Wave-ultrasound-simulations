/*
 * @file        hdf5readingthread.cpp
 * @author      Petr Kleparnik, VUT FIT Brno, xklepa01@stud.fit.vutbr.cz
 * @version     0.0
 * @date        30 July 2014
 *
 * @brief       The implementation file containing the HDF5ReadingThread and Request class. This class
 *              is for loading HDF5 data from other threads than GUI of application is running.
 *
 * @section     Licence
 * This file is part of k-Wave visualiser application
 * for visualizing HDF5 data created by the k-Wave toolbox - http://www.k-wave.org.
 * Copyright © 2014, Petr Kleparnik, VUT FIT Brno.
 * k-Wave visualiser is free software.
 */

#include "hdf5readingthread.h"

#include <QDebug>
#include <QQueue>

#include <HDF5File.h>
#include <HDF5Dataset.h>

/**
 * @brief Request::Request Create request for block of dataset reading with offset and count (size) of data.
 * @param dataset
 * @param zO
 * @param yO
 * @param xO
 * @param zC
 * @param yC
 * @param xC
 */
Request::Request(HDF5File::HDF5Dataset *dataset, hsize_t zO, hsize_t yO, hsize_t xO, hsize_t zC, hsize_t yC, hsize_t xC)
{
    this->dataset = dataset;
    this->zO = zO;
    this->yO = yO;
    this->xO = xO;
    this->zC = zC;
    this->yC = yC;
    this->xC = xC;
    this->full = false;
    this->data = NULL;
}

/**
 * @brief Request::Request Create request for full 3D dataset reading.
 * @param dataset
 */
Request::Request(HDF5File::HDF5Dataset *dataset)
{
    this->dataset = dataset;
    this->zO = 0;
    this->yO = 0;
    this->xO = 0;
    this->zC = 0;
    this->yC = 0;
    this->xC = 0;
    this->full = true;
    this->data = NULL;
}

/**
 * @brief Request::~Request
 */
Request::~Request()
{
    delete [] data;
}

/**
 * @brief Request::toQString Helper function
 * @return dataset name with offset nad count
 */
QString Request::toQString()
{
    return QString::fromStdString(dataset->getName()) + "    " + QString::number(zO) + " x " + QString::number(yO) + " x " + QString::number(xO) + "    " + QString::number(zC) + " x " + QString::number(yC) + " x " + QString::number(xC);
}

/**
 * @brief HDF5ReadingThread::HDF5ReadingThread
 * @param parent
 */
HDF5ReadingThread::HDF5ReadingThread(QObject *parent) : QThread(parent)
{
    // Unused
    setTerminationEnabled(true);
    //stopFlag = false;
}

/**
 * @brief HDF5ReadingThread::createRequest Create request in thread
 * @param dataset
 * @param zO
 * @param yO
 * @param xO
 * @param zC
 * @param yC
 * @param xC
 * @param limit (volatile) lenght of waiting queue
 */
void HDF5ReadingThread::createRequest(HDF5File::HDF5Dataset *dataset, hsize_t zO, hsize_t yO, hsize_t xO, hsize_t zC, hsize_t yC, hsize_t xC, int limit)
{
    QMutexLocker locker(&queueMutex);
    if (queue.size() > limit) {
        while (!queue.isEmpty()) {
            Request *r = queue.dequeue();
            delete r;
        }
    }
    queue.enqueue(new Request(dataset, zO, yO, xO, zC, yC, xC));
}

/**
 * @brief HDF5ReadingThread::createRequest Create request for full dataset read in thread
 * @param dataset
 */
void HDF5ReadingThread::createRequest(HDF5File::HDF5Dataset *dataset)
{
    QMutexLocker locker(&queueMutex);
    while (!queue.isEmpty()) {
        Request *r = queue.dequeue();
        delete r;
    }
    queue.enqueue(new Request(dataset));
}

/**
 * @brief HDF5ReadingThread::~HDF5ReadingThread
 */
HDF5ReadingThread::~HDF5ReadingThread()
{
    clearRequests();
    clearDoneRequests();
}

/**
 * @brief HDF5ReadingThread::clearDoneRequests Clear all done requests
 */
void HDF5ReadingThread::clearDoneRequests()
{
    QMutexLocker locker(&requestMutex);
    qDeleteAll(doneRequests);
    doneRequests.clear();
}

/**
 * @brief HDF5ReadingThread::clearRequests Clear requests which are waiting in queue
 */
void HDF5ReadingThread::clearRequests()
{
    QMutexLocker locker(&queueMutex);
    while (!queue.isEmpty()) {
        Request *r = queue.dequeue();
        delete r;
    }
}

/**
 * @brief HDF5ReadingThread::mutex Mutex (static) for synchronization if reading
 */
QMutex HDF5ReadingThread::mutex;

/**
 * @brief HDF5ReadingThread::deleteDoneRequest Delete one done request
 * @param r request
 */
void HDF5ReadingThread::deleteDoneRequest(Request *r)
{
    QMutexLocker locker(&requestMutex);
    if(doneRequests.contains(r)) {
        doneRequests.removeOne(r);
        delete r;
    }
}

/*void HDF5ReadingThread::stop()
{
    QMutexLocker locker(&stopMutex);
    stopFlag = true;
}*/

/**
 * @brief HDF5ReadingThread::run Thread work
 */
void HDF5ReadingThread::run()
{
    //stopFlag = false;
    while (1) {
        // Reading mutex
        QMutexLocker lock(&mutex);
        Request *r = NULL;
        // Queue mutex
        queueMutex.lock();
        if (!queue.isEmpty())
            // Get waiting request
            r = queue.dequeue();
        else {
            queueMutex.unlock();
            break;
        }
        queueMutex.unlock();

        if (r != NULL) {
            try {
                if (r->full) {
                    // Reading of full dataset with block reading
                    r->dataset->initBlockReading();
                    do {
                        // Request for returning part of 3D data (block)
                        Request *newR = new Request(r->dataset);
                        //qDebug() << "start reading block... ";
                        r->dataset->readBlock(newR->zO, newR->yO, newR->xO, newR->zC, newR->yC, newR->xC, newR->data, newR->min, newR->max);
                        QMutexLocker locker(&requestMutex);
                        doneRequests.append(newR);
                        emit requestDone(newR);
                    } while (!r->dataset->isLastBlock());
                    // Delete original request
                    delete r;
                } else {
                    // One block data reading (slice)
                    //qDebug() << "start reading 3D dataset... ";
                    r->dataset->read3DDataset(r->zO, r->yO, r->xO, r->zC, r->yC, r->xC, r->data, r->min, r->max);
                    QMutexLocker locker(&requestMutex);
                    doneRequests.append(r);
                    emit requestDone(r);
                }
                //delete r;
            } catch(std::exception &e) {
                std::cerr << e.what() << std::endl;
            }
        }

        /*QMutexLocker locker(&stopMutex);
        if (stopFlag)
            break;

        usleep(1000);*/
    }
}
