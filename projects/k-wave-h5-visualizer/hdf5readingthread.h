/**
 * @file        hdf5readingthread.h
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        30 July      2014 (created) \n
 *              27 February  2017 (updated)
 *
 * @brief       The header file with HDF5ReadingThread and Request class declaration.
 *
 * @license     This file is part of the k-Wave-h5-visualizer tool for processing the HDF5 data
 *              created by the k-Wave toolbox - http://www.k-wave.org. This file may be used,
 *              distributed and modified under the terms of the LGPL version 3 open source
 *              license. A copy of the LGPL license should have been recieved with this file.
 *              Otherwise, it can be found at: http://www.gnu.org/copyleft/lesser.html.
 *
 * @copyright   Copyright © 2017, Petr Kleparnik, VUT FIT Brno. All Rights Reserved.
 *
 */

#ifndef HDF5READINGTHREAD_H
#define HDF5READINGTHREAD_H

#include <QtCore>

#include <hdf5helper.h>

/**
 * @brief The Request class represents wrapper for the HDF5 files reading requests
 */
class Request
{
public:
    Request(HDF5Helper::Dataset *dataset, HDF5Helper::Vector offset, HDF5Helper::Vector count);
    Request(HDF5Helper::Dataset *dataset, hsize_t step);
    ~Request();
    QString toQString();

    HDF5Helper::Vector offset;
    HDF5Helper::Vector count;
    float min, max;
    hsize_t minIndex, maxIndex;
    bool full;
    hsize_t step;
    HDF5Helper::Dataset *dataset;
    float *data;
};

/**
 * @brief The HDF5ReadingThread class represents wrapper for HDF5 reading threads
 */
class HDF5ReadingThread : public QThread
{
    Q_OBJECT
public:
    HDF5ReadingThread(QObject *parent = 0);
    ~HDF5ReadingThread();

public slots:
    void createRequest(HDF5Helper::Dataset *dataset, HDF5Helper::Vector offset, HDF5Helper::Vector count, int limit = 0);
    void createRequest(HDF5Helper::Dataset *dataset, hsize_t step);
    void stopCurrentBlockReading();
    void clearRequests();
    void clearDoneRequests();
    void deleteDoneRequest(Request *request);
    //void stop();

protected:
    virtual void run();

signals:
    void requestDone(Request *);

private:
    static QMutex mutex;
    QMutex queueMutex, requestMutex;
    QQueue<Request *> queue;
    QList<Request *> doneRequests;
    //bool stopFlag;
    bool stopFlag = false;

};

#endif // HDF5READINGTHREAD_H
