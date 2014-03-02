#include "cvimagewidget.h"

#include <QWidget>
#include <QImage>
#include <QPainter>
#include <opencv2/opencv.hpp>

#include "dataset.h"
#include "loadingthread.h"


CVImageWidget::CVImageWidget(QWidget *parent) : QWidget(parent)
{
    isSetDataset = false;
    thread = NULL;
}

QSize CVImageWidget::sizeHint() const
{
    return _qimage.size();
}

QSize CVImageWidget::minimumSizeHint() const
{
    return _qimage.size();
}

void CVImageWidget::showImage(const cv::Mat &image)
{
    // Convert the image to the RGB888 format
    switch (image.type()) {
    case CV_8UC1:
        cvtColor(image, _tmp, CV_GRAY2RGB);
        break;
    case CV_8UC3:
        cvtColor(image, _tmp, CV_BGR2RGB);
        break;
    }

    // QImage needs the data to be stored continuously in memory
    assert(_tmp.isContinuous());
    // Assign OpenCV's image buffer to the QImage. Note that the bytesPerLine parameter
    // (http://qt-project.org/doc/qt-4.8/qimage.html#QImage-6) is 3*width because each pixel
    // has three bytes.
    _qimage = QImage(_tmp.data, _tmp.cols, _tmp.rows, _tmp.cols*3, QImage::Format_RGB888);
    if (width() > height())
        _qimage = _qimage.scaledToWidth(height(), Qt::SmoothTransformation);
    else
        _qimage = _qimage.scaledToWidth(width(), Qt::SmoothTransformation);

    repaint();
}

void CVImageWidget::resizeEvent(QResizeEvent * event)
{

    _qimage = QImage(_tmp.data, _tmp.cols, _tmp.rows, _tmp.cols*3, QImage::Format_RGB888);
    if (width() > height())
        _qimage = _qimage.scaledToWidth(height(), Qt::SmoothTransformation);
    else
        _qimage = _qimage.scaledToWidth(width(), Qt::SmoothTransformation);
}

void CVImageWidget::setDataset(Dataset *dataset, int dim)
{
    _dataset = dataset;
    isSetDataset = true;
    _dim = dim; // TODO - check dim
    if (thread != NULL && thread->isRunning()) {
        //thread->terminate();
        //thread->wait();
        return;
    }
    thread = new LoadingThread(_dataset, _dim, 0);
    qRegisterMetaType< cv::Mat >("cv::Mat");
    connect(thread, SIGNAL(showImage(const cv::Mat &)), this, SLOT(showImage(const cv::Mat &)), Qt::QueuedConnection);
    thread->start();
}

void CVImageWidget::unsetDataset()
{
    isSetDataset = false;
}

void CVImageWidget::reloadImage(int value)
{
    if (isSetDataset) {
        if (thread != NULL && thread->isRunning()) {
            //thread->terminate();
            //thread->wait();
            return;
        }
        thread = new LoadingThread(_dataset, _dim, value);
        qRegisterMetaType< cv::Mat >("cv::Mat");
        connect(thread, SIGNAL(showImage(const cv::Mat &)), this, SLOT(showImage(const cv::Mat &)), Qt::QueuedConnection);
        thread->start();
    }
}

void CVImageWidget::paintEvent(QPaintEvent* /*event*/)
{
    // Display the image
    QPainter painter(this);
    painter.drawImage(QPoint(0,0), _qimage);
    painter.end();
}
