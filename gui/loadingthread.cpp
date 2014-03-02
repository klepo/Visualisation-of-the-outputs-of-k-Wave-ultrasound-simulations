#include "loadingthread.h"

#include "cvimagewidget.h"
#include "dataset.h"

#include <opencv2/opencv.hpp>

#include <QThread>

LoadingThread::LoadingThread(Dataset *dataset, int dim, int index) {
   _dataset = dataset;
   _dim = dim;
   _index = index;
}

void LoadingThread::run() {
    std::cout << "running" << std::endl;
    cv::Mat img = _dataset->getImage(_dim, _index);
    emit showImage(img);
}
