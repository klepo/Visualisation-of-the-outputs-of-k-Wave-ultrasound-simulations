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
}

Request::Request(HDF5File::HDF5Dataset *_dataset)
{
    dataset = _dataset;
    full = true;
}

HDF5ReadingThread::HDF5ReadingThread(QObject *parent) : QThread(parent)
{
    setTerminationEnabled(true);
    //data = NULL;
}

void HDF5ReadingThread::setParams(HDF5File::HDF5Dataset *dataset, hsize_t zO, hsize_t yO, hsize_t xO, hsize_t zC, hsize_t yC, hsize_t xC, int limit)
{
    mutexQueue.lock();
    if (queue.size() > limit) {
        while (!queue.isEmpty()) {
            Request *r = queue.dequeue();
            delete r;
        }
    }
    queue.enqueue(new Request(dataset, zO, yO, xO, zC, yC, xC));
    mutexQueue.unlock();
}

void HDF5ReadingThread::setParams(HDF5File::HDF5Dataset *dataset)
{
    mutexQueue.lock();
    while (!queue.isEmpty()) {
        Request *r = queue.dequeue();
        delete r;
    }
    queue.enqueue(new Request(dataset));
    mutexQueue.unlock();
}

HDF5ReadingThread::~HDF5ReadingThread()
{
    delete [] _data;
}

void HDF5ReadingThread::clearRequests()
{
    mutexQueue.lock();
    while (!queue.isEmpty()) {
        //if (queue.size() == 0) break;
        Request *r = queue.dequeue();
        delete r;
    }
    mutexQueue.unlock();
}

QMutex HDF5ReadingThread::mutex;

void HDF5ReadingThread::run()
{
    while (1) {
        QMutexLocker lock(&mutex);
        mutexQueue.lock();
        if (queue.isEmpty()) {
            mutexQueue.unlock();
            break;
        }
        Request *r = queue.dequeue();
        mutexQueue.unlock();
        try {
            float minV, maxV;
            if (r->full) {
                hsize_t xO, yO, zO, xC, yC, zC;
                r->dataset->initBlockReading();
                do {
                    float *data = NULL;
                    r->dataset->readBlock(zO, yO, xO, zC, yC, xC, data, minV, maxV);
                    _data = data;
                    emit dataBlockLoaded(zO, yO, xO, zC, yC, xC, data);
                } while (!r->dataset->isLastBlock());
            } else {
                float *data = NULL;
                //HDF5File::HDF5Dataset *dataset = r->file->openDataset(r->datasetName);
                r->dataset->read3DDataset(r->zO, r->yO, r->xO, r->zC, r->yC, r->xC, data, minV, maxV);
                //r->file->closeDataset(r->datasetName);
                _data = data;
                emit dataLoaded(r->zO, r->yO, r->xO, r->zC, r->yC, r->xC, data, minV, maxV);
            }
            delete r;
        } catch(std::exception &e) {
            std::cerr << e.what() << std::endl;
        }
    }
}
