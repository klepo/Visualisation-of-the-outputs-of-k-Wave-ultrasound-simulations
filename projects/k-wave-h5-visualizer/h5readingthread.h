/**
 * @file        h5readingthread.h
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        30 July      2014 (created) <br>
 *              22 November  2018 (updated)
 *
 * @brief       The header file with H5ReadingThread and Request class declaration.
 *
 * @license     This file is part of the k-Wave-h5-visualizer tool for processing the HDF5 data
 *              created by the k-Wave toolbox - http://www.k-wave.org. This file may be used,
 *              distributed and modified under the terms of the LGPL version 3 open source
 *              license. A copy of the LGPL license should have been received with this file.
 *              Otherwise, it can be found at: http://www.gnu.org/copyleft/lesser.html.
 *
 * @copyright   Copyright © 2018, Petr Kleparnik, VUT FIT Brno. All Rights Reserved.
 *
 */

#ifndef H5READINGTHREAD_H
#define H5READINGTHREAD_H

#include <QtCore>

#include <k-wave-h5-helper.h>

/**
 * @brief The Request class represents wrapper for the HDF5 files reading requests
 */
class Request
{
public:
    Request(H5Helper::Dataset *dataset, H5Helper::Vector offset, H5Helper::Vector count, float *data, float *dataLC = nullptr, float *dataCC = nullptr);
    Request(H5Helper::Dataset *dataset, hsize_t step, float *data, float *dataLC = nullptr, float *dataCC = nullptr);
    ~Request();
    QString toQString();

    /// Offset
    H5Helper::Vector offset;
    /// Count
    H5Helper::Vector count;
    /// Read full 3D dataset flag
    bool full = false;
    /// Step
    hsize_t step = 0;
    /// Dataset
    H5Helper::Dataset *dataset = nullptr;
    /// External data
    float *data = nullptr;
    /// External dataLC
    float *dataLC = nullptr;
    /// External dataCC
    float *dataCC = nullptr;
    /// DataLC changed flag
    bool dataLCChanged = false;
    /// DataCC changed flag
    bool dataCCChanged = false;
};

/**
 * @brief The H5ReadingThread class represents wrapper for HDF5 reading threads
 */
class H5ReadingThread : public QThread
{
    Q_OBJECT
public:
    H5ReadingThread(QObject *parent = nullptr);
    ~H5ReadingThread();

    hsize_t getLocalStep() const;
    void setCompressHelper(const H5Helper::CompressHelper *compressHelper);

public slots:
    void createRequest(H5Helper::Dataset *dataset, H5Helper::Vector offset, H5Helper::Vector count, float *data, float *dataLC = nullptr, float *dataCC = nullptr);
    void createRequest(H5Helper::Dataset *dataset, hsize_t step, float *data, float *dataLC = nullptr, float *dataCC = nullptr);
    void stopCurrentBlockReading();
    void clearDoneRequests();
    void clearRequests();
    void deleteDoneRequest(Request *request);

protected:
    virtual void run();

signals:
    /**
     * @brief Request done signal
     */
    void requestDone(Request *);

private:
    Q_DISABLE_COPY(H5ReadingThread)

    /// Mutex (static) for synchronization of HDF5 reading
    static QMutex mutex;
    /// Mutex for queue
    QMutex queueMutex;
    /// Mutex for request
    QMutex requestMutex;
    /// Queue
    QQueue<Request *> queue;
    /// Done requests
    QList<Request *> doneRequests;
    /// Last compress coefficient data
    bool initLCFlag = true;
    /// Current compress coefficient data
    bool initCCFlag = true;
    /// Last compress coefficient offset
    H5Helper::Vector4D offsetLC;
    /// Current compress coefficient offset
    H5Helper::Vector4D offsetCC;
    /// Local step
    hsize_t localStep = 0;
    /// Compress helper
    const H5Helper::CompressHelper *compressHelper = nullptr;
    /// Mean read time
    double meanTime = 0;
    /// Read counter
    hsize_t readCount = 0;
    /// Read time sum
    double timeSum = 0;
    /// Stop flag
    bool stopFlag = false;

};

#endif // H5READINGTHREAD_H
