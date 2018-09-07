/**
 * @file        hdf5readingthread.h
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        30 July      2014 (created) \n
 *              11 September 2017 (updated)
 *
 * @brief       The header file with HDF5ReadingThread and Request class declaration.
 *
 * @license     This file is part of the k-Wave-h5-visualizer tool for processing the HDF5 data
 *              created by the k-Wave toolbox - http://www.k-wave.org. This file may be used,
 *              distributed and modified under the terms of the LGPL version 3 open source
 *              license. A copy of the LGPL license should have been received with this file.
 *              Otherwise, it can be found at: http://www.gnu.org/copyleft/lesser.html.
 *
 * @copyright   Copyright © 2017, Petr Kleparnik, VUT FIT Brno. All Rights Reserved.
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
    Request(H5Helper::Dataset *dataset, H5Helper::Vector offset, H5Helper::Vector count);
    Request(H5Helper::Dataset *dataset, hsize_t step);
    ~Request();
    QString toQString();

    /// Offset
    H5Helper::Vector offset;
    /// Count
    H5Helper::Vector count;
    /// Minimum
    //float min;
    /// Maximum
    //float max;
    /// Minimal index
    //hsize_t minIndex;
    /// Maximal index
    //hsize_t maxIndex;
    /// Read full 3D dataset flag
    bool full = false;
    /// Step
    hsize_t step = 0;
    /// Dataset
    H5Helper::Dataset *dataset = nullptr;
    /// Data
    float *data = nullptr;
};

/**
 * @brief The HDF5ReadingThread class represents wrapper for HDF5 reading threads
 */
class H5ReadingThread : public QThread
{
    Q_OBJECT
public:
    H5ReadingThread(QObject *parent = 0);
    ~H5ReadingThread();
    void setCompressHelper(H5Helper::CompressHelper *compressHelper);

public slots:
    void createRequest(H5Helper::Dataset *dataset, H5Helper::Vector offset, H5Helper::Vector count);
    void createRequest(H5Helper::Dataset *dataset, hsize_t step);
    void stopCurrentBlockReading();
    void clearRequests();
    void clearDoneRequests();
    void deleteDoneRequest(Request *request);
    //void stop();

protected:
    virtual void run();

signals:
    /// Request done signal
    void requestDone(Request *);

private:
    static QMutex mutex;
    QMutex queueMutex, requestMutex;
    QQueue<Request *> queue;
    QList<Request *> doneRequests;
    Request *doneRequestCC = 0;
    Request *doneRequestLC = 0;
    H5Helper::CompressHelper *compressHelper = 0;
    double meanTime = 0;
    hsize_t readCount = 0;
    double timeSum = 0;
    //bool stopFlag;
    bool stopFlag = false;

};

#endif // H5READINGTHREAD_H
