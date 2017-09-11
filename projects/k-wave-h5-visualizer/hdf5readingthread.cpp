/**
 * @file        hdf5readingthread.cpp
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        30 July      2014 (created) \n
 *              11 September 2017 (updated)
 *
 * @brief       The implementation file containing HDF5ReadingThread and Request
 *              class definition.
 *
 * This class is used for loading HDF5 data from other threads than GUI of application
 * is running.
 * *
 * @license     This file is part of the k-Wave-h5-visualizer tool for processing the HDF5 data
 *              created by the k-Wave toolbox - http://www.k-wave.org. This file may be used,
 *              distributed and modified under the terms of the LGPL version 3 open source
 *              license. A copy of the LGPL license should have been recieved with this file.
 *              Otherwise, it can be found at: http://www.gnu.org/copyleft/lesser.html.
 *
 * @copyright   Copyright © 2017, Petr Kleparnik, VUT FIT Brno. All Rights Reserved.
 *
 */

#include "hdf5readingthread.h"

/**
 * @brief Creates request for full 3D dataset with given offset and count
 * @param[in] dataset Dataset
 * @param[in] offset Offset
 * @param[in] count Count
 */
Request::Request(HDF5Helper::Dataset *dataset, HDF5Helper::Vector offset, HDF5Helper::Vector count)
{
    this->dataset = dataset;
    this->offset = offset;
    this->count = count;
    this->full = false;
    this->data = 0;
}

/**
 * @brief Creates request for full 3D dataset reading with given step
 * @param[in] dataset Dataset
 * @param[in] step Step
 */
Request::Request(HDF5Helper::Dataset *dataset, hsize_t step)
{
    this->dataset = dataset;
    this->full = true;
    this->data = 0;
    this->step = step;
}

/**
 * @brief Destructor of Request object
 *
 * Deletes allocated data do request
 */
Request::~Request()
{
    delete[] data;
}

/**
 * @brief Creates string from request
 * @return Dataset name with offset nad count
 */
QString Request::toQString()
{
    return QString::fromStdString(dataset->getName()) + " " + QString::fromStdString(offset) + " " + QString::fromStdString(count);
}

/**
 * @brief HDF5ReadingThread::HDF5ReadingThread
 * @param[in] parent
 */
HDF5ReadingThread::HDF5ReadingThread(QObject *parent) : QThread(parent)
{
    // Unused
    setTerminationEnabled(true);
    //stopFlag = false;
}

/**
 * @brief Creates request in thread
 * @param[in] dataset Dataset
 * @param[in] offset Offset
 * @param[in] count Count
 * @param[in] limit Length of waiting queue (optional)
 */
void HDF5ReadingThread::createRequest(HDF5Helper::Dataset *dataset, HDF5Helper::Vector offset, HDF5Helper::Vector count, int limit)
{
    QMutexLocker locker(&queueMutex);
    if (queue.size() > limit) {
        while (!queue.isEmpty()) {
            Request *r = queue.dequeue();
            delete r;
        }
    }
    queue.enqueue(new Request(dataset, offset, count));
}

/**
 * @brief Creates request for full dataset read in thread with givne step
 * @param[in] dataset Dataset
 * @param[in] step Step
 */
void HDF5ReadingThread::createRequest(HDF5Helper::Dataset *dataset, hsize_t step)
{
    QMutexLocker locker(&queueMutex);
    while (!queue.isEmpty()) {
        Request *r = queue.dequeue();
        delete r;
    }
    queue.enqueue(new Request(dataset, step));
}

/**
 * @brief Stops current block reading
 */
void HDF5ReadingThread::stopCurrentBlockReading()
{
    stopFlag = true;
}

/**
 * @brief Destructor of HDF5ReadingThread object
 */
HDF5ReadingThread::~HDF5ReadingThread()
{
    clearRequests();
    clearDoneRequests();
}

/**
 * @brief Clears all done requests
 */
void HDF5ReadingThread::clearDoneRequests()
{
    QMutexLocker locker(&requestMutex);
    qDeleteAll(doneRequests);
    doneRequests.clear();
}

/**
 * @brief Clears requests which are waiting in queue
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
 * @brief Mutex (static) for synchronization if reading
 */
QMutex HDF5ReadingThread::mutex;

/**
 * @brief Deletes one done request
 * @param[in] r Request
 */
void HDF5ReadingThread::deleteDoneRequest(Request *r)
{
    QMutexLocker locker(&requestMutex);
    if (doneRequests.contains(r)) {
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
 * @brief Thread work
 */
void HDF5ReadingThread::run()
{
    //stopFlag = false;
    while (1) {
        // Reading mutex
        QMutexLocker lock(&mutex);
        Request *r = 0;
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

        if (r != 0) {
            try {
                //usleep(1000000);
                if (r->full) {
                    // Reading of full dataset with block reading
                    r->dataset->setMaxNumberOfElmsToLoad(HDF5Helper::Vector3D(r->dataset->getDims()).getSize());
                    hsize_t c = HDF5Helper::Vector3D(r->dataset->getNumberOfBlocksInDims()).z();
                    for (hsize_t i = 0; i < c; i++) {
                        if (stopFlag) {
                            stopFlag = false;
                            break;
                        }
                        // Request for returning part of 3D data (block)
                        Request *newR = new Request(r->dataset, r->step);
                        //qDebug() << "start reading block... ";
                        r->dataset->readBlock(c * newR->step + i, newR->offset, newR->count, newR->data, newR->min, newR->max, newR->minIndex, newR->maxIndex);
                        QMutexLocker locker(&requestMutex);
                        doneRequests.append(newR);
                        emit requestDone(newR);
                    }
                    // Delete original request
                    delete r;
                } else {
                    // One block data reading (slice)
                    //qDebug() << "start reading a slice... ";
                    r->dataset->readDataset(r->offset, r->count, r->data, r->min, r->max, r->minIndex, r->maxIndex);
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
