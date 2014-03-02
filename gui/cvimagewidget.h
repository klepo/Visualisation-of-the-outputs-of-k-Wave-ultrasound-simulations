#ifndef CVIMAGEWIDGET_H
#define CVIMAGEWIDGET_H

#include <QWidget>
#include <QImage>
#include <QPainter>
#include <opencv2/opencv.hpp>

#include "dataset.h"
//#include "loadingthread.h"

class LoadingThread;

class CVImageWidget : public QWidget {
    Q_OBJECT

    public:
        explicit CVImageWidget(QWidget *parent = 0);
        QSize sizeHint() const;
        QSize minimumSizeHint() const;

    public:
        void setDataset(Dataset *dataset, int dim);
        void unsetDataset();
        void resizeEvent(QResizeEvent * event);

    public slots:
        void showImage(const cv::Mat& image);
        void reloadImage(int value);

    private:
        int _dim;
        Dataset *_dataset;
        QImage _qimage;
        cv::Mat _tmp;
        bool isSetDataset;
        LoadingThread *thread;

        void paintEvent(QPaintEvent* /*event*/);
};

#endif // CVIMAGEWIDGET_H

