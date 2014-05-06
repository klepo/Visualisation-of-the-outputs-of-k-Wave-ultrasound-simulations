#include "cvimagewidget.h"

#include <QWidget>
#include <QImage>
#include <QPainter>
#include <opencv2/opencv.hpp>
#include <QDebug>
#include <QMouseEvent>
#include <QToolTip>
#include <QFileDialog>


CVImageWidget::CVImageWidget(QWidget *parent) : QWidget(parent)
{
    clearFlag = false;
    adjustFlag = true;
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

void CVImageWidget::setAdjust(bool adjust)
{
    adjustFlag = adjust;
    refreshImage();
}

void CVImageWidget::refreshImage()
{
    if (isSetImage) {
        _qimage = QImage(_tmp.data, _tmp.cols, _tmp.rows, _tmp.cols*3, QImage::Format_RGB888);
        if (point.x() == 0 && point.y() == 0 && adjustFlag) {
            if ((double) _tmp.cols / _tmp.rows >= (double) width() / height())
                _qimage = _qimage.scaledToWidth(width(), Qt::SmoothTransformation);
            else
                _qimage = _qimage.scaledToHeight(height(), Qt::SmoothTransformation);
            //emit imageResized(_qimage.width(),_qimage.height());
            setMinimumWidth(10);
            setMinimumHeight(10);
        } else {
            if ((_tmp.cols + point.x()) > width())
                setMinimumWidth(_tmp.cols + point.x());
            if ((_tmp.rows + point.y()) > height())
                setMinimumHeight(_tmp.rows + point.y());
        }
    }
    repaint();
}

void CVImageWidget::showImage(const cv::Mat &image, QPoint point, QString fileName)
{
    this->fileName =  fileName;
    this->point = point;
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
    //_qimage = QImage(_tmp.data, _tmp.cols, _tmp.rows, _tmp.cols*3, QImage::Format_RGB888);
    isSetImage = true;
    clearFlag = false;
    refreshImage();
}

void CVImageWidget::saveImage()
{
    if (isSetImage) {
        QString fileName = QFileDialog::getSaveFileName(this, "Save image", this->fileName + ".png", "Image (*.png)");
        if (fileName != NULL)
            _qimage.save(fileName, 0, 100);
    }
}

void CVImageWidget::resizeEvent(QResizeEvent *)
{
    refreshImage();
}

void CVImageWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (isSetImage) {
        //qDebug() << "orig:" << event->pos().x() << event->pos().y();
        QPoint p;
        p.setX((event->pos().x() - point.x()) * _tmp.cols / _qimage.width());
        p.setY((event->pos().y() - point.y()) * _tmp.rows / _qimage.height());
        //qDebug() << p.x() << p.y();
        if (p.x() >= 0 && p.x() < _tmp.cols && p.y() >= 0 && p.y() < _tmp.rows) {
            //QToolTip::showText(event->globalPos(), QString::number(p.x()) + " x " + QString::number(p.y()), this, rect());
            emit hoveredPointInImage(p.x(), p.y());
        }
    }
    QWidget::mouseMoveEvent(event);
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
