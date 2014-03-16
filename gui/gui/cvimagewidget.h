#ifndef CVIMAGEWIDGET_H
#define CVIMAGEWIDGET_H

#include <QWidget>
#include <QImage>
#include <QPainter>
#include <opencv2/opencv.hpp>

class LoadingThread;

class CVImageWidget : public QWidget
{
    Q_OBJECT

public:
    explicit CVImageWidget(QWidget *parent = 0);
    QSize sizeHint() const;
    QSize minimumSizeHint() const;

public slots:
    void showImage(const cv::Mat& image, QPoint = QPoint(0,0));
    void clearImage();

private:
    int _dim;
    QImage _qimage;
    cv::Mat _tmp;
    bool clearFlag;
    QPoint point;
    void resizeEvent(QResizeEvent *);
    void paintEvent(QPaintEvent *);
};

#endif // CVIMAGEWIDGET_H

