#ifndef HDF5READINGTHREAD_H
#define HDF5READINGTHREAD_H

#include <QThread>
#include <HDF5File.h>
#include <HDF5Dataset.h>
#include <QMutex>
#include <QQueue>

class Request
{
public:
    Request(HDF5File::HDF5Dataset *dataset, hsize_t, hsize_t, hsize_t, hsize_t, hsize_t, hsize_t);
    float zO, yO, xO, zC, yC, xC;
    HDF5File::HDF5Dataset *dataset;
};

class HDF5ReadingThread : public QThread
{
    Q_OBJECT
public:
    HDF5ReadingThread(QObject *parent = 0);
    ~HDF5ReadingThread();

    void setParams(HDF5File::HDF5Dataset *, hsize_t, hsize_t, hsize_t, hsize_t, hsize_t, hsize_t, int limit = 0);
    void clearRequests();

protected:
    virtual void run();

signals:
    void dataBlockLoaded(hsize_t, hsize_t, hsize_t, hsize_t, hsize_t, hsize_t, float *, float, float);

public slots:

private:
    static QMutex mutex;
    QMutex mutexQueue;
    float *_data;
    QQueue<Request *> queue;
};

#endif // HDF5READINGTHREAD_H
