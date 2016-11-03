/**
 * @file        cvimagewidget.cpp
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        30 July      2014 (created)
 *              6  December  2015 (updated)
 *              8  September 2016 (updated)
 *              3  November  2016 (updated)
 *
 * @brief       The implementation file containing CVImageWidget class definition.
 *              CVImageWidget class is for drawing OpenCV image to QWidget.
 *
 *
 * @license     This file is part of k-Wave visualiser application
 * for visualizing HDF5 data created by the k-Wave toolbox - http://www.k-wave.org.
 *
 * @copyright   Copyright © 2016, Petr Kleparnik, VUT FIT Brno. All Rights Reserved.
 *
 * k-Wave visualiser is free software.
 */

#include "cvimagewidget.h"

/**
 * @brief CVImageWidget::CVImageWidget Constructor with initialization
 * @param parent
 */
CVImageWidget::CVImageWidget(QWidget *parent) : QWidget(parent)
{
    clearFlag = false;
    adjustFlag = true;
    point = QPoint(0, 0);
    isSetImage = false;
}

QSize CVImageWidget::sizeHint() const
{
    return qimage.size();
}

QSize CVImageWidget::minimumSizeHint() const
{
    return qimage.size();
}

/**
 * @brief CVImageWidget::setAdjust Enable/disable image adjust
 * @param adjust true/false
 */
void CVImageWidget::setAdjust(bool adjust)
{
    // Set adjusting of image size
    adjustFlag = adjust;
    refreshImage();
}

/**
 * @brief CVImageWidget::refreshImage Recompute image with new params (width, height, adjustFlag)
 */
void CVImageWidget::refreshImage()
{
    // If image is set
    if (isSetImage) {
        // Create new QImage
        qimage = QImage(tmpMat);
        // if is adjusting enabled
        if (point.x() == 0 && point.y() == 0 && adjustFlag) {
            // Scale to width or height accodring to widget size
            if (double(tmpMat.width()) / tmpMat.height() >= double(width()) / height())
                qimage = qimage.scaledToWidth(width(), Qt::SmoothTransformation);
            else
                qimage = qimage.scaledToHeight(height(), Qt::SmoothTransformation);
            //emit imageResized(_qimage.width(),_qimage.height());
            setMinimumWidth(10);
            setMinimumHeight(10);
        } else {
            //  Original image size
            if ((tmpMat.width() + point.x()) > width())
                setMinimumWidth(tmpMat.width() + point.x());
            if ((tmpMat.height() + point.y()) > height())
                setMinimumHeight(tmpMat.height() + point.y());
        }
    }
    repaint();
}

/**
 * @brief CVImageWidget::showImage Set Opencv image to widget
 * @param image image (cv::Mat)
 * @param point position of sensor mask image (unused)
 * @param fileName Name for png image save
 */
void CVImageWidget::showImage(const QImage &image, QPoint point, QString fileName)
{
    // Save some info
    this->fileName =  fileName;
    this->point = point;
    tmpMat = image;
    isSetImage = true;
    clearFlag = false;
    refreshImage();
}

/**
 * @brief CVImageWidget::saveImage Save image as png file
 */
void CVImageWidget::saveImage()
{
    if (isSetImage) {
        QString fileName = QFileDialog::getSaveFileName(this, "Save image", this->fileName + ".png", "Image (*.png)");
        if (fileName != NULL)
            qimage.save(fileName, 0, 100);
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
        // Compute mouse position on image
        p.setX((event->pos().x() - point.x()) * tmpMat.width() / qimage.width());
        p.setY((event->pos().y() - point.y()) * tmpMat.height() / qimage.height());
        //qDebug() << p.x() << p.y();
        // If mouse is not out of image send event
        if (p.x() >= 0 && p.x() < tmpMat.width() && p.y() >= 0 && p.y() < tmpMat.height()) {
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
    // Clear image
    if (!clearFlag)
        painter.drawImage(point, qimage);
    painter.end();
}
