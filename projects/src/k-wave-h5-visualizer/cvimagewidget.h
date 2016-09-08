/**
 * @file        cvimagewidget.h
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.0
 * @date        30 July      2014 (created)
 *              6  December  2015 (updated)
 *              8  September 2015 (updated)
 *
 * @brief       The header file with CVImageWidget class declaration.
 *

 * @license     This file is partof k-Wave visualiser application
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

#include <opencv2/opencv_modules.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#ifdef HAVE_OPENCV_CONTRIB
    #include <opencv2/contrib/contrib.hpp>
#endif

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
    void showImage(const cv::Mat &image, QPoint = QPoint(0,0), QString fileName = "");
    void clearImage();
    void saveImage();
    void setAdjust(bool adjust);
    void refreshImage();

private:
    void resizeEvent(QResizeEvent *);
    void paintEvent(QPaintEvent *);
    void mouseMoveEvent(QMouseEvent *);

    int _dim;
    QImage _qimage;
    cv::Mat _tmp;
    bool clearFlag;
    QPoint point;
    bool adjustFlag;
    bool isSetImage;
    QString fileName;

};

#endif // CVIMAGEWIDGET_H

