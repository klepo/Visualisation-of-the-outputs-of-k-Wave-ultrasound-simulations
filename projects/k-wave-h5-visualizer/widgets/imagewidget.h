/**
 * @file        imagewidget.h
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        30 July      2014 (created) <br>
 *              29 October   2018 (updated)
 *
 * @brief       The header file with ImageWidget class declaration.
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

#ifndef IMAGEWIDGET_H
#define IMAGEWIDGET_H

#include <QtCore>
#include <QWidget>
#include <QFileDialog>
#include <QMouseEvent>
#include <QPainter>

/**
 * @brief The ImageWidget class represents wrapper for image widgets
 */
class ImageWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ImageWidget(QWidget *parent = nullptr);
    QSize sizeHint() const;
    QSize minimumSizeHint() const;

signals:
    /**
     * @brief Image resized signal
     * @param[in] width Image width
     * @param[in] height Image height
     */
    void imageResized(int width, int height);
    /**
     * @brief Hovered point in image
     * @param[in] x X position
     * @param[in] y Y position
     */
    void hoveredPointInImage(int x, int y);

public slots:
    void setAdjust(bool adjust);
    void clearImage();
    void refreshImage();
    void setFilename(QString filename = "");
    void showImage(const QImage &image, QPoint = QPoint(0,0));
    void saveImage();

private:
    Q_DISABLE_COPY(ImageWidget)

    void resizeEvent(QResizeEvent *);
    void paintEvent(QPaintEvent *);
    void mouseMoveEvent(QMouseEvent *);

    /// Original image
    QImage originalImage;
    /// Scaled image
    QImage scaledImage;
    /// Point
    QPoint point;
    /// Filename
    QString filename;
    /// Adjust flag
    bool adjustFlag;
    /// Has image flag
    bool hasImage;
};

#endif // IMAGEWIDGET_H

