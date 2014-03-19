#include "hdf5readingthread.h"

#include <QDebug>

#include <HDF5File.h>
#include <HDF5Dataset.h>

HDF5ReadingThread::HDF5ReadingThread(HDF5File *_file, std::string _datasetName, hsize_t _zO, hsize_t _yO, hsize_t _xO, hsize_t _zC, hsize_t _yC, hsize_t _xC, QObject *parent) : QThread(parent)
{
    setTerminationEnabled(true);
    file = _file;
    datasetName = _datasetName;
    zO = _zO;
    yO = _yO;
    xO = _xO;
    zC = _zC;
    yC = _yC;
    xC = _xC;
    connect(this, SIGNAL(finished()), this, SLOT(deleteLater()));
    data = NULL;
}

HDF5ReadingThread::~HDF5ReadingThread()
{
    delete [] data;
}

QMutex HDF5ReadingThread::mutex;

void HDF5ReadingThread::run()
{
    QMutexLocker lock(&mutex);
    try {
        float min;
        float max;
        HDF5File::HDF5Dataset *dataset = file->openDataset(datasetName);
        dataset->read3DDataset(zO, yO, xO, zC, yC, xC, data, min, max);
        file->closeDataset(datasetName);
        emit sliceLoaded(zO, yO, xO, zC, yC, xC, data, min, max);
    } catch(std::exception &e) {
        std::cerr << e.what() << std::endl;
    }
}
