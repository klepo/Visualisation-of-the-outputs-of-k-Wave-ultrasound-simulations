/**
 * @file        imagewidget.cpp
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        30 July      2014 (created) \n
 *              27 February  2017 (updated)
 *
 * @brief       The implementation file containing ImageWidget class definition.
 *              ImageWidget class is for drawing QImage into QWidget.
 *
 *
 * @license     This file is part of k-Wave visualiser application
 * for visualizing HDF5 data created by the k-Wave toolbox - http://www.k-wave.org.
 *
 * @copyright   Copyright © 2017, Petr Kleparnik, VUT FIT Brno. All Rights Reserved.
 *
 * k-Wave visualiser is free software.
 */

#include "imagewidget.h"

/**
 * @brief ImageWidget::ImageWidget Constructor with initialization
 * @param parent
 */
ImageWidget::ImageWidget(QWidget *parent) : QWidget(parent)
{
    adjustFlag = true;
    point = QPoint(0, 0);
    isSetImage = false;
}

QSize ImageWidget::sizeHint() const
{
    return scaledImage.size();
}

QSize ImageWidget::minimumSizeHint() const
{
    return scaledImage.size();
}

/**
 * @brief ImageWidget::setAdjust Enable/disable image adjust
 * @param adjust true/false
 */
void ImageWidget::setAdjust(bool adjust)
{
    // Set adjusting of image size
    adjustFlag = adjust;
    refreshImage();
}

/**
 * @brief ImageWidget::refreshImage Recompute image with new params (width, height, adjustFlag)
 */
void ImageWidget::refreshImage()
{
    // If image is set
    if (isSetImage) {
        // Create new QImage
        scaledImage = QImage(originalImage);
        // if is adjusting enabled
        if (point.x() == 0 && point.y() == 0 && adjustFlag) {
            // Scale to width or height accodring to widget size
            if (double(originalImage.width()) / originalImage.height() >= double(width()) / height())
                scaledImage = scaledImage.scaledToWidth(width(), Qt::SmoothTransformation);
            else
                scaledImage = scaledImage.scaledToHeight(height(), Qt::SmoothTransformation);
            //emit imageResized(_qimage.width(),_qimage.height());
            setMinimumWidth(10);
            setMinimumHeight(10);
        } else {
            //  Original image size
            if ((originalImage.width() + point.x()) > width())
                setMinimumWidth(originalImage.width() + point.x());
            if ((originalImage.height() + point.y()) > height())
                setMinimumHeight(originalImage.height() + point.y());
        }
    }
    repaint();
}

/**
 * @brief ImageWidget::showImage Set Opencv image to widget
 * @param image image (cv::Mat)
 * @param point position of sensor mask image (unused)
 * @param fileName Name for png image save
 */
void ImageWidget::showImage(const QImage &image, QPoint point, QString fileName)
{
    // Save some info
    this->fileName = fileName;
    this->point = point;
    isSetImage = true;
    originalImage = image;
    refreshImage();
}

/**
 * @brief ImageWidget::saveImage Save image as png file
 */
void ImageWidget::saveImage()
{
    if (isSetImage) {
        QString fileName = QFileDialog::getSaveFileName(this, "Save image", this->fileName + ".png", "Image (*.png)");
        if (fileName != 0)
            scaledImage.save(fileName, 0, 100);
    }
}

void ImageWidget::resizeEvent(QResizeEvent *)
{
    refreshImage();
}

void ImageWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (isSetImage) {
        //qDebug() << "orig:" << event->pos().x() << event->pos().y();
        QPoint p;
        // Compute mouse position on image
        p.setX((event->pos().x() - point.x()) * originalImage.width() / scaledImage.width());
        p.setY((event->pos().y() - point.y()) * originalImage.height() / scaledImage.height());
        //qDebug() << p.x() << p.y();
        // If mouse is not out of image send event
        if (p.x() >= 0 && p.x() < originalImage.width() && p.y() >= 0 && p.y() < originalImage.height()) {
            //QToolTip::showText(event->globalPos(), QString::number(p.x()) + " x " + QString::number(p.y()), this, rect());
            emit hoveredPointInImage(p.x(), p.y());
        }
    }
    QWidget::mouseMoveEvent(event);
}

void ImageWidget::clearImage()
{
    this->fileName.clear();
    point = QPoint(0, 0);
    isSetImage = false;
    originalImage = QImage();
    scaledImage = QImage();
    repaint();
}

void ImageWidget::paintEvent(QPaintEvent *)
{
    // Display the image
    QPainter painter(this);
    painter.eraseRect(0, 0, width(), height());
    // Clear image
    if (isSetImage)
        painter.drawImage(point, scaledImage);
    painter.end();
}
