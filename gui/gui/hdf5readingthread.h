#ifndef HDF5READINGTHREAD_H
#define HDF5READINGTHREAD_H

#include <QThread>
#include <HDF5File.h>
#include <HDF5Dataset.h>
#include <QMutex>

class HDF5ReadingThread : public QThread
{
    Q_OBJECT
public:
    HDF5ReadingThread(HDF5File *_file, std::string _datasetName, hsize_t, hsize_t, hsize_t, hsize_t, hsize_t, hsize_t, QObject *parent = 0);
    ~HDF5ReadingThread();

protected:
    virtual void run();

signals:
    void sliceLoaded(hsize_t, hsize_t, hsize_t, hsize_t, hsize_t, hsize_t, float *, float, float);

public slots:

private:
    float zO, yO, xO, zC, yC, xC;
    HDF5File *file;
    std::string datasetName;
    static QMutex mutex;
    float *data;
};

#endif // HDF5READINGTHREAD_H
