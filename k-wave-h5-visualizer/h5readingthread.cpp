/**
 * @file        h5readingthread.cpp
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        30 July      2014 (created) <br>
 *              27 March     2019 (updated)
 *
 * @brief       The implementation file containing H5ReadingThread and Request
 *              class definition.
 *
 * This class is used for loading HDF5 data from other threads than GUI of application
 * is running.
 * *
 * @license     This file is part of the k-Wave-h5-visualizer tool for processing the HDF5 data
 *              created by the k-Wave toolbox - http://www.k-wave.org. This file may be used,
 *              distributed and modified under the terms of the LGPL version 3 open source
 *              license. A copy of the LGPL license should have been received with this file.
 *              Otherwise, it can be found at: http://www.gnu.org/copyleft/lesser.html.
 *
 * @copyright   Copyright © 2019, Petr Kleparnik, VUT FIT Brno. All Rights Reserved.
 *
 */

#include "h5readingthread.h"

/**
 * @brief Creates request for 3D dataset with given offset and count
 * @param[in] dataset Dataset
 * @param[in] offset Offset
 * @param[in] count Count
 * @param[in] data Destination memory for reading
 * @param[in] dataLC Destination memory for LC reading
 * @param[in] dataCC Destination memory for CC reading
 */
Request::Request(H5Helper::Dataset *dataset, H5Helper::Vector offset, H5Helper::Vector count, float *data, float *dataLC, float *dataCC)
{
    this->dataset = dataset;
    this->offset = offset;
    this->count = count;
    this->full = false;
    this->data = data;
    this->dataLC = dataLC;
    this->dataCC = dataCC;
}

/**
 * @brief Creates request for full 3D dataset reading with given step
 * @param[in] dataset Dataset
 * @param[in] step Step
 * @param[in] data Destination memory for reading
 * @param[in] dataLC Destination memory for LC reading
 * @param[in] dataCC Destination memory for CC reading
 */
Request::Request(H5Helper::Dataset *dataset, hsize_t step, float *data, float *dataLC, float *dataCC)
{
    this->dataset = dataset;
    if (dataset->getRank() == 3) {
        this->offset = H5Helper::Vector3D(0, 0, 0);
        this->count = dataset->getDims();
    } else if (dataset->getRank() == 4) {
        this->offset = H5Helper::Vector4D(step, 0, 0, 0);
        this->count = H5Helper::Vector4D(1, dataset->getDims());
    }
    this->full = true;
    this->step = step;
    this->data = data;
    this->dataLC = dataLC;
    this->dataCC = dataCC;
}

/**
 * @brief Destructor of Request object
 *
 * Deletes allocated data by request
 */
Request::~Request()
{

}

/**
 * @brief Creates string from request
 * @return Dataset name with offset and count
 */
QString Request::toQString()
{
    return QString::fromStdString(dataset->getName()) + " " + QString::fromStdString(offset) + " " + QString::fromStdString(count);
}

/**
 * @brief Creates H5ReadingThread object
 * @param[in] parent
 */
H5ReadingThread::H5ReadingThread(QObject *parent) : QThread(parent)
{
    // Unused
    setTerminationEnabled(true);
    //stopFlag = false;
}

/**
 * @brief Destructor of H5ReadingThread object
 */
H5ReadingThread::~H5ReadingThread()
{
    clearRequests();
    clearDoneRequests();
}

/**
 * @brief Returns local step
 * @return Local step
 */
hsize_t H5ReadingThread::getLocalStep() const
{
    return localStep;
}

/**
 * @brief Sets compress helper
 * @param[in] compressHelper Compress helper
 */
void H5ReadingThread::setCompressHelper(const H5Helper::CompressHelper *compressHelper)
{
    this->compressHelper = compressHelper;
}

/**
 * @brief Creates request in thread
 * @param[in] dataset Dataset
 * @param[in] offset Offset
 * @param[in] count Count
 * @param[in] data Destination memory for reading
 * @param[in] dataLC Destination memory for LC reading
 * @param[in] dataCC Destination memory for CC reading
 */
void H5ReadingThread::createRequest(H5Helper::Dataset *dataset, H5Helper::Vector offset, H5Helper::Vector count, float *data, float *dataLC, float *dataCC)
{
    QMutexLocker locker(&queueMutex);
    while (!queue.isEmpty()) {
        Request *r = queue.dequeue();
        delete r;
        r = nullptr;
    }
    queue.enqueue(new Request(dataset, offset, count, data, dataLC, dataCC));
}

/**
 * @brief Creates request for full dataset read in thread with given step
 * @param[in] dataset Dataset
 * @param[in] step Step
 * @param[in] data Destination memory for reading
 * @param[in] dataLC Destination memory for LC reading
 * @param[in] dataCC Destination memory for CC reading
 */
void H5ReadingThread::createRequest(H5Helper::Dataset *dataset, hsize_t step, float *data, float *dataLC, float *dataCC)
{
    QMutexLocker locker(&queueMutex);
    while (!queue.isEmpty()) {
        Request *r = queue.dequeue();
        delete r;
        r = nullptr;
    }
    Request *r = new Request(dataset, step, data, dataLC, dataCC);
    if (compressHelper)
        r->count[r->count.getLength() - 1] = r->count[r->count.getLength() - 1] / compressHelper->getStride();
    queue.enqueue(r);
}

/**
 * @brief Stops current block reading
 */
void H5ReadingThread::stopCurrentBlockReading()
{
    stopFlag = true;
}

/**
 * @brief Clears all done requests
 */
void H5ReadingThread::clearDoneRequests()
{
    QMutexLocker locker(&requestMutex);
    qDeleteAll(doneRequests);
    doneRequests.clear();
}

/**
 * @brief Clears requests which are waiting in queue
 */
void H5ReadingThread::clearRequests()
{
    QMutexLocker locker(&queueMutex);
    while (!queue.isEmpty()) {
        Request *r = queue.dequeue();
        delete r;
        r = nullptr;
    }
}

/**
 * @brief Deletes one done request
 * @param[in] r Request
 */
void H5ReadingThread::deleteDoneRequest(Request *r)
{
    QMutexLocker locker(&requestMutex);
    if (doneRequests.contains(r)) {
        doneRequests.removeOne(r);
        delete r;
        r = nullptr;
    }
}

/**
 * @brief Thread work
 */
void H5ReadingThread::run()
{
    //stopFlag = false;
    while (1) {
        // Reading mutex
        QMutexLocker lock(&mutex);
        Request *r = nullptr;
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

#ifdef QT_DEBUG
        bool log =  true;
#else
        bool log =  false;
#endif

        if (r) {
            try {
//#ifdef QT_DEBUG
                QElapsedTimer elapsedTimer;
                elapsedTimer.restart();
//#endif

                if (compressHelper) {
                    hsize_t xStride = compressHelper->getStride();
                    hsize_t oSize = compressHelper->getOSize();
                    hsize_t step = H5Helper::Vector4D(r->offset).t();
                    hsize_t stepC = step / oSize;
                    localStep = step - stepC * oSize;
                    hsize_t steps = H5Helper::Vector4D(r->dataset->getDims()).t();
                    H5Helper::Vector4D offsetLC = r->offset;
                    H5Helper::Vector4D offsetCC = r->offset;
                    H5Helper::Vector4D count = r->count;
                    count.x(count.x() * xStride);

                    offsetLC.t(stepC);
                    if (offsetLC.t() >= steps)
                        offsetLC.t(steps - 1);
                    offsetLC.x(offsetLC.x() * xStride);

                    offsetCC.t(stepC + 1);
                    if (offsetCC.t() >= steps)
                        offsetCC.t(steps - 1);
                    offsetCC.x(offsetCC.x() * xStride);

                    if (initLCFlag || this->offsetLC != offsetLC) {
                        r->dataset->readDataset(offsetLC, count, r->dataLC, log);
                        this->offsetLC = offsetLC;
                        initLCFlag = false;
                        r->dataLCChanged = true;
                    }

                    if (initCCFlag || this->offsetCC != offsetCC) {
                        r->dataset->readDataset(offsetCC, count, r->dataCC, log);
                        this->offsetCC = offsetCC;
                        initCCFlag = false;
                        r->dataCCChanged = true;
                    }

                    if (!r->full) {
#pragma omp parallel for
                        for (hssize_t p = 0; p < hssize_t(r->count.getSize()); p++) {
                            r->data[p] = compressHelper->computeTimeStep(&r->dataCC[p * hssize_t(xStride)], &r->dataLC[p * hssize_t(xStride)], localStep);
                        }
                    }
                } else {
                    r->dataset->readDataset(r->offset, r->count, r->data, log);
                }

                QMutexLocker locker(&requestMutex);

                r->nsecsElapsed = elapsedTimer.nsecsElapsed();
#ifdef QT_DEBUG
                // Time measuring
                timeSum += r->nsecsElapsed;
                readCount++;
                meanTime = timeSum / readCount;
                if (r->full) {
                    qDebug() << QString::fromStdString(r->count) << "(3D): " << double(meanTime) / 1000000 << "ms";
                } else {
                    qDebug() << QString::fromStdString(r->count) << "(slice): "<< double(meanTime) / 1000000 << "ms";
                }
#endif

                doneRequests.append(r);
                emit requestDone(r);
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

/**
 * @brief Mutex (static) for synchronization of HDF5 reading
 */
QMutex H5ReadingThread::mutex;
