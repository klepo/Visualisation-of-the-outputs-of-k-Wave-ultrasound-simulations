/**
 * @file        imagewidget.h
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        30 July      2014 (created) \n
 *              27 February  2017 (updated)
 *
 * @brief       The header file with ImageWidget class declaration.
 *
 * @license     This file is part of the k-Wave-h5-visualizer tool for processing the HDF5 data
 *              created by the k-Wave toolbox - http://www.k-wave.org. This file may be used,
 *              distributed and modified under the terms of the LGPL version 3 open source
 *              license. A copy of the LGPL license should have been recieved with this file.
 *              Otherwise, it can be found at: http://www.gnu.org/copyleft/lesser.html.
 *
 * @copyright   Copyright © 2017, Petr Kleparnik, VUT FIT Brno. All Rights Reserved.
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
    explicit ImageWidget(QWidget *parent = 0);
    QSize sizeHint() const;
    QSize minimumSizeHint() const;

signals:
    void imageResized(int width, int height);
    void hoveredPointInImage(int x, int y);

public slots:
    void showImage(const QImage &image, QPoint = QPoint(0,0), QString fileName = "");
    void clearImage();
    void saveImage();
    void setAdjust(bool adjust);
    void refreshImage();

private:
    void resizeEvent(QResizeEvent *);
    void paintEvent(QPaintEvent *);
    void mouseMoveEvent(QMouseEvent *);

    QImage originalImage;
    QImage scaledImage;
    QPoint point;
    QString fileName;
    bool adjustFlag;
    bool isSetImage;
};

#endif // IMAGEWIDGET_H

