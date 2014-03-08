#ifndef LOADINGTHREAD_H
#define LOADINGTHREAD_H

#include "dataset.h"
#include "cvimagewidget.h"

#include <opencv2/opencv.hpp>

#include <QThread>

class LoadingThread : public QThread {

    Q_OBJECT
    public:
        LoadingThread(Dataset *dataset, int dim, int index);

    signals:
        void showImage(const cv::Mat &);

    private:
        void run();
        Dataset *_dataset;
        int _dim;
        int _index;
};

#endif // LOADINGTHREAD_H
