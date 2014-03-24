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
            if (queue.size() == 1) break;
            Request *r = queue.dequeue();
            delete r;
        }
    }
    queue.enqueue(new Request(dataset, zO, yO, xO, zC, yC, xC));
    mutexQueue.unlock();
}

HDF5ReadingThread::~HDF5ReadingThread()
{
    //delete [] data;
}

void HDF5ReadingThread::clearRequests()
{
    mutexQueue.lock();
    while (!queue.isEmpty()) {
        if (queue.size() == 1) break;
        Request *r = queue.dequeue();
        delete r;
    }
    mutexQueue.unlock();
}

QMutex HDF5ReadingThread::mutex;

void HDF5ReadingThread::run()
{
    while (!queue.isEmpty()) {
        QMutexLocker lock(&mutex);
        mutexQueue.lock();
        Request *r = queue.dequeue();
        mutexQueue.unlock();
        try {
            float min;
            float max;
            float *data = NULL;
            //HDF5File::HDF5Dataset *dataset = r->file->openDataset(r->datasetName);
            r->dataset->read3DDataset(r->zO, r->yO, r->xO, r->zC, r->yC, r->xC, data, min, max);
            //r->file->closeDataset(r->datasetName);
            emit sliceLoaded(r->zO, r->yO, r->xO, r->zC, r->yC, r->xC, data, min, max);
            delete r;
        } catch(std::exception &e) {
            std::cerr << e.what() << std::endl;
        }
    }
}
