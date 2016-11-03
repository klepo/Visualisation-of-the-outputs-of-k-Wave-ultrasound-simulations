/**
 * @file        cvimagewidget.h
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        30 July      2014 (created)
 *              6  December  2015 (updated)
 *              8  September 2016 (updated)
 *              3  November  2016 (updated)
 *
 * @brief       The header file with CVImageWidget class declaration.
 *
 *
 * @license     This file is part of k-Wave visualiser application
 * for visualizing HDF5 data created by the k-Wave toolbox - http://www.k-wave.org.
 *
 * @copyright   Copyright © 2016, Petr Kleparnik, VUT FIT Brno. All Rights Reserved.
 *
 * k-Wave visualiser is free software.
 */

#ifndef CVIMAGEWIDGET_H
#define CVIMAGEWIDGET_H

#include <QtCore>
#include <QWidget>
#include <QFileDialog>
#include <QMouseEvent>
#include <QPainter>

class LoadingThread;

class CVImageWidget : public QWidget
{
    Q_OBJECT

public:
    explicit CVImageWidget(QWidget *parent = 0);
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

    QImage qimage;
    QPoint point;
    QString fileName;
    QImage tmpMat;
    bool clearFlag;
    bool adjustFlag;
    bool isSetImage;
};

#endif // CVIMAGEWIDGET_H

