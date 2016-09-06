/*
 * @file        hdf5readingthread.h
 * @author      Petr Kleparnik, VUT FIT Brno, xklepa01@stud.fit.vutbr.cz
 * @version     0.0
 * @date        30 July      2014 (created)
 *              6  December  2015 (updated)
 *
 * @brief       The header file with HDF5ReadingThread and Request class declaration.
 *
 * @section     Licence
 * This file is part of k-Wave visualiser application
 * for visualizing HDF5 data created by the k-Wave toolbox - http://www.k-wave.org.
 * Copyright © 2014, Petr Kleparnik, VUT FIT Brno.
 * k-Wave visualiser is free software.
 */

#ifndef HDF5READINGTHREAD_H
#define HDF5READINGTHREAD_H

#include <QtCore>

#include <hdf5helper.h>

class Request
{
public:
    Request(HDF5Helper::HDF5Dataset *dataset, HDF5Helper::HDF5Vector offset, HDF5Helper::HDF5Vector count);
    Request(HDF5Helper::HDF5Dataset *dataset, hsize_t step);
    ~Request();
    QString toQString();

    HDF5Helper::HDF5Vector offset;
    HDF5Helper::HDF5Vector count;
    float min, max;
    bool full;
    hsize_t step;
    HDF5Helper::HDF5Dataset *dataset;
    float *data;
};

class HDF5ReadingThread : public QThread
{
    Q_OBJECT
public:
    HDF5ReadingThread(QObject *parent = 0);
    ~HDF5ReadingThread();

public slots:
    void createRequest(HDF5Helper::HDF5Dataset *dataset, HDF5Helper::HDF5Vector offset, HDF5Helper::HDF5Vector count, int limit = 0);
    void createRequest(HDF5Helper::HDF5Dataset *dataset, hsize_t step);
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
    QMutex stopMutex;

};

#endif // HDF5READINGTHREAD_H
