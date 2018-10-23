/**
 * @file        hdf5readingthread.cpp
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        30 July      2014 (created) <br>
 *              10 October   2018 (updated)
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
 * @copyright   Copyright © 2018, Petr Kleparnik, VUT FIT Brno. All Rights Reserved.
 *
 */

#include <h5readingthread.h>

/**
 * @brief Creates request for 3D dataset with given offset and count
 * @param[in] dataset Dataset
 * @param[in] offset Offset
 * @param[in] count Count
 */
Request::Request(H5Helper::Dataset *dataset, H5Helper::Vector offset, H5Helper::Vector count, float *data)
{
    this->dataset = dataset;
    this->offset = offset;
    this->count = count;
    this->full = false;
    if (data) {
        this->data = data;
        this->extData = true;
    }
}

/**
 * @brief Creates request for full 3D dataset reading with given step
 * @param[in] dataset Dataset
 * @param[in] step Step
 */
Request::Request(H5Helper::Dataset *dataset, hsize_t step, float *data)
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
    if (data) {
        this->data = data;
        this->extData = true;
    }
}

/**
 * @brief Destructor of Request object
 *
 * Deletes allocated data by request
 */
Request::~Request()
{
    if (!this->extData) {
        delete[] data;
        data = nullptr;
    }
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
 * @brief H5ReadingThread::H5ReadingThread
 * @param[in] parent
 */
H5ReadingThread::H5ReadingThread(QObject *parent) : QThread(parent)
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
void H5ReadingThread::createRequest(H5Helper::Dataset *dataset, H5Helper::Vector offset, H5Helper::Vector count, float *data)
{
    QMutexLocker locker(&queueMutex);
    while (!queue.isEmpty()) {
        Request *r = queue.dequeue();
        delete r;
        r = nullptr;
    }
    queue.enqueue(new Request(dataset, offset, count, data));
}

/**
 * @brief Creates request for full dataset read in thread with given step
 * @param[in] dataset Dataset
 * @param[in] step Step
 */
void H5ReadingThread::createRequest(H5Helper::Dataset *dataset, hsize_t step, float *data)
{
    QMutexLocker locker(&queueMutex);
    while (!queue.isEmpty()) {
        Request *r = queue.dequeue();
        delete r;
        r = nullptr;
    }
    Request *r = new Request(dataset, step, data);
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
 * @brief Destructor of H5ReadingThread object
 */
H5ReadingThread::~H5ReadingThread()
{
    clearRequests();
    clearDoneRequests();
    if (dataLC) {
        delete[] dataLC;
        dataLC = nullptr;
    }
    if (dataCC) {
        delete[] dataCC;
        dataCC = nullptr;
    }
}

void H5ReadingThread::setCompressHelper(H5Helper::CompressHelper *compressHelper)
{
    this->compressHelper = compressHelper;
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
 * @brief Mutex (static) for synchronization if reading
 */
QMutex H5ReadingThread::mutex;

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

/*void H5ReadingThread::stop()
{
    QMutexLocker locker(&stopMutex);
    stopFlag = true;
}*/

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

        bool log =  false;

        if (r) {
            try {
#ifdef QT_DEBUG
                QElapsedTimer elapsedTimer;
                elapsedTimer.restart();
#endif

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

                    offsetLC.t((stepC) - 1);
                    if (hssize_t(stepC) - 1 < 0)
                        offsetLC.t(0);
                    if (offsetLC.t() >= steps)
                        offsetLC.t(steps - 1);
                    offsetLC.x(offsetLC.x() * xStride);

                    offsetCC.t(stepC);
                    if (offsetCC.t() >= steps)
                        offsetCC.t(steps - 1);
                    offsetCC.x(offsetCC.x() * xStride);

                    if (!this->dataLC || this->offsetLC != offsetLC) {
                        r->dataset->readDataset(offsetLC, count, this->dataLC, log);
                        this->offsetLC = offsetLC;
                    }

                    if (!this->dataCC || this->offsetCC != offsetCC) {
                        r->dataset->readDataset(offsetCC, count, this->dataCC, log);
                        this->offsetCC = offsetCC;
                    }

                    if (!r->full) {
                        if (!r->extData) {
                            r->data = new float[r->count.getSize()]();
                        }

#pragma omp parallel for
                        for (hssize_t p = 0; p < hssize_t(r->count.getSize()); p++) {
                            r->data[p] = compressHelper->computeTimeStep(&this->dataCC[p * xStride], &this->dataLC[p * xStride], localStep);
                        }
                    }
                } else {
                    if (!r->extData) {
                        r->data = new float[r->count.getSize()]();
                    }
                    r->dataset->readDataset(r->offset, r->count, r->data, log);
                }

                QMutexLocker locker(&requestMutex);

#ifdef QT_DEBUG
                // Time measuring
                timeSum += elapsedTimer.nsecsElapsed();
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

float *H5ReadingThread::getDataCC() const
{
    return dataCC;
}

float *H5ReadingThread::getDataLC() const
{
    return dataLC;
}

hsize_t H5ReadingThread::getLocalStep() const
{
    return localStep;
}
