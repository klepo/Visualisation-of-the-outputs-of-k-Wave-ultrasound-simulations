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
    Request(HDF5File::HDF5Dataset *dataset);
    ~Request();
    QString toQString();

    hsize_t zO, yO, xO, zC, yC, xC;
    float min, max;
    bool full;
    HDF5File::HDF5Dataset *dataset;
    float *data;
};

class HDF5ReadingThread : public QThread
{
    Q_OBJECT
public:
    HDF5ReadingThread(QObject *parent = 0);
    ~HDF5ReadingThread();

protected:
    virtual void run();

signals:
    void requestDone(Request *);

public slots:
    void createRequest(HDF5File::HDF5Dataset *, hsize_t, hsize_t, hsize_t, hsize_t, hsize_t, hsize_t, int limit = 0);
    void createRequest(HDF5File::HDF5Dataset *);
    void clearRequests();
    void clearDoneRequests();
    void deleteDoneRequest(Request *);
    //void stop();

private:
    static QMutex mutex;
    QMutex queueMutex, requestMutex;
    QQueue<Request *> queue;
    QList<Request *> doneRequests;
    //bool stopFlag;
    QMutex stopMutex;

};

#endif // HDF5READINGTHREAD_H
