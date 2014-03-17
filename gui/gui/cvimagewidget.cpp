#include "cvimagewidget.h"

#include <QWidget>
#include <QImage>
#include <QPainter>
#include <opencv2/opencv.hpp>
#include <QDebug>

CVImageWidget::CVImageWidget(QWidget *parent) : QWidget(parent)
{
    clearFlag = false;
    point = QPoint(0, 0);
    isSetImage = false;
}

QSize CVImageWidget::sizeHint() const
{
    return _qimage.size();
}

QSize CVImageWidget::minimumSizeHint() const
{
    return _qimage.size();
}

void CVImageWidget::showImage(const cv::Mat &image, QPoint _point)
{
    point = _point;
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
    isSetImage = true;
    if (point.x() == 0 && point.y() == 0) {
        if ((double) _tmp.cols / _tmp.rows >= (double) width() / height())
            _qimage = _qimage.scaledToWidth(width(), Qt::SmoothTransformation);
        else
            _qimage = _qimage.scaledToHeight(height(), Qt::SmoothTransformation);
    }
    clearFlag = false;
    repaint();
}

void CVImageWidget::resizeEvent(QResizeEvent *)
{
    if (isSetImage) {
        if (point.x() == 0 && point.y() == 0) {
            _qimage = QImage(_tmp.data, _tmp.cols, _tmp.rows, _tmp.cols*3, QImage::Format_RGB888);
            //qDebug() << (double) _tmp.cols / _tmp.rows << "   " <<  (double) width() / height();
            if ((double) _tmp.cols / _tmp.rows >= (double) width() / height())
                _qimage = _qimage.scaledToWidth(width(), Qt::SmoothTransformation);
            else
                _qimage = _qimage.scaledToHeight(height(), Qt::SmoothTransformation);
            //emit imageResized(_qimage.width(),_qimage.height());
        }
    }
}

void CVImageWidget::clearImage()
{
    clearFlag = true;
    repaint();
}

void CVImageWidget::paintEvent(QPaintEvent *)
{
    // Display the image
    QPainter painter(this);
    painter.eraseRect(0, 0, width(), height());
    if (!clearFlag)
        painter.drawImage(point, _qimage);
    painter.end();
}
