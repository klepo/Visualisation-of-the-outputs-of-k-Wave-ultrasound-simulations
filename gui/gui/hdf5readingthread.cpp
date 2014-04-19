#include "hdf5readingthread.h"

#include <QDebug>
#include <QQueue>

#include <HDF5File.h>
#include <HDF5Dataset.h>

Request::Request(HDF5File::HDF5Dataset *_dataset, hsize_t _zO, hsize_t _yO, hsize_t _xO, hsize_t _zC, hsize_t _yC, hsize_t _xC)
{
    dataset = _dataset;
    zO = _zO;
    yO = _yO;
    xO = _xO;
    zC = _zC;
    yC = _yC;
    xC = _xC;
    full = false;
    data = NULL;
}

Request::Request(HDF5File::HDF5Dataset *_dataset)
{
    dataset = _dataset;
    zO = 0;
    yO = 0;
    xO = 0;
    zC = 0;
    yC = 0;
    xC = 0;
    full = true;
    data = NULL;
}

Request::~Request()
{
    delete [] data;
}

QString Request::toQString()
{
    return QString::fromStdString(dataset->getName()) + "    " + QString::number(zO) + " x " + QString::number(yO) + " x " + QString::number(xO) + "    " + QString::number(zC) + " x " + QString::number(yC) + " x " + QString::number(xC);
}

HDF5ReadingThread::HDF5ReadingThread(QObject *parent) : QThread(parent)
{
    setTerminationEnabled(true);
    //stopFlag = false;
}

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

void HDF5ReadingThread::createRequest(HDF5File::HDF5Dataset *dataset)
{
    QMutexLocker locker(&queueMutex);
    while (!queue.isEmpty()) {
        Request *r = queue.dequeue();
        delete r;
    }
    queue.enqueue(new Request(dataset));
}

HDF5ReadingThread::~HDF5ReadingThread()
{
    clearRequests();
    clearDoneRequests();
}

void HDF5ReadingThread::clearDoneRequests()
{
    QMutexLocker locker(&requestMutex);
    qDeleteAll(doneRequests);
    doneRequests.clear();
}

void HDF5ReadingThread::clearRequests()
{
    QMutexLocker locker(&queueMutex);
    while (!queue.isEmpty()) {
        Request *r = queue.dequeue();
        delete r;
    }
}

QMutex HDF5ReadingThread::mutex;

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

void HDF5ReadingThread::run()
{
    //stopFlag = false;
    while (1) {
        QMutexLocker lock(&mutex);
        Request *r = NULL;
        queueMutex.lock();
        if (!queue.isEmpty())
            r = queue.dequeue();
        else {
            queueMutex.unlock();
            break;
        }
        queueMutex.unlock();

        if (r != NULL) {
            try {
                if (r->full) {
                    r->dataset->initBlockReading();
                    do {
                        Request *newR = new Request(r->dataset);
                        qDebug() << "start reading block... ";
                        r->dataset->readBlock(newR->zO, newR->yO, newR->xO, newR->zC, newR->yC, newR->xC, newR->data, newR->min, newR->max);
                        QMutexLocker locker(&requestMutex);
                        doneRequests.append(newR);
                        emit requestDone(newR);
                    } while (!r->dataset->isLastBlock());
                    delete r;
                } else {
                    qDebug() << "start reading 3D dataset... ";
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
