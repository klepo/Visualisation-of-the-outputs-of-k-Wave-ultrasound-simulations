/**
 * @file        imagewidget.cpp
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        30 July      2014 (created) <br>
 *              10 October   2018 (updated)
 *
 * @brief       The implementation file containing ImageWidget class definition.
 *
 * ImageWidget class is for drawing QImage into QWidget.
 *
 * @license     This file is part of the k-Wave-h5-visualizer tool for processing the HDF5 data
 *              created by the k-Wave toolbox - http://www.k-wave.org. This file may be used,
 *              distributed and modified under the terms of the LGPL version 3 open source
 *              license. A copy of the LGPL license should have been received with this file.
 *              Otherwise, it can be found at: http://www.gnu.org/copyleft/lesser.html.
 *
 * @copyright   Copyright © 2018, Petr Kleparnik, VUT FIT Brno. All Rights Reserved.
 *
 */

#include <imagewidget.h>

/**
 * @brief Constructor with initialization
 * @param[in] parent Parent (optional)
 */
ImageWidget::ImageWidget(QWidget *parent) : QWidget(parent)
{
    adjustFlag = true;
    point = QPoint(0, 0);
    hasImage = false;
}

/**
 * @brief Size hint
 * @return Size
 */
QSize ImageWidget::sizeHint() const
{
    return scaledImage.size();
}

/**
 * @brief Minimum size hint
 * @return SIze
 */
QSize ImageWidget::minimumSizeHint() const
{
    return scaledImage.size();
}

/**
 * @brief Enable/disable image adjust
 * @param[in] adjust True/False
 */
void ImageWidget::setAdjust(bool adjust)
{
    // Set adjusting of image size
    adjustFlag = adjust;
    refreshImage();
}

/**
 * @brief Clears image
 */
void ImageWidget::clearImage()
{
    this->filename.clear();
    point = QPoint(0, 0);
    hasImage = false;
    originalImage = QImage();
    scaledImage = QImage();
    repaint();
}

/**
 * @brief Recomputes image with new params (width, height, adjustFlag)
 */
void ImageWidget::refreshImage()
{
    // If image is set
    if (hasImage) {
        // Create new QImage
        scaledImage = QImage(originalImage);
        // if is adjusting enabled
        if (point.x() == 0 && point.y() == 0 && adjustFlag) {
            // Scale to width or height according to widget size
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
 * @brief Sets image to widget
 * @param[in] image Image
 * @param[in] point Position of sensor mask image (unused)
 * @param[in] fileName Name for png image saving
 */
void ImageWidget::showImage(const QImage &image, QPoint point, QString fileName)
{
    // Save some info
    this->filename = fileName;
    this->point = point;
    hasImage = true;
    originalImage = image;
    refreshImage();
}

/**
 * @brief Saves image as png file
 */
void ImageWidget::saveImage()
{
    if (hasImage) {
        QString filename = QFileDialog::getSaveFileName(this, "Save image", this->filename + ".png", "Image (*.png)");
        if (!filename.isEmpty())
            scaledImage.save(filename, nullptr, 100);
    }
}

/**
 * @brief Resize event
 */
void ImageWidget::resizeEvent(QResizeEvent *)
{
    refreshImage();
}

/**
 * @brief Paint event
 */
void ImageWidget::paintEvent(QPaintEvent *)
{
    // Display the image
    QPainter painter(this);
    painter.eraseRect(0, 0, width(), height());
    // Clear image
    if (hasImage)
        painter.drawImage(point, scaledImage);
    painter.end();
}

/**
 * @brief ImageWidget::mouseMoveEvent
 * @param[in] event Event
 */
void ImageWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (hasImage) {
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
