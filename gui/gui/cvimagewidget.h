#ifndef CVIMAGEWIDGET_H
#define CVIMAGEWIDGET_H

#include <QScrollArea>
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

signals:
    void imageResized(int width, int height);
    void hoveredPointInImage(int x, int y);

public slots:
    void showImage(const cv::Mat& image, QPoint = QPoint(0,0), QString fileName = "");
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

