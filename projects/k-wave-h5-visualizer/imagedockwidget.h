#ifndef IMAGEDOCKWIDGET_H
#define IMAGEDOCKWIDGET_H

#include <QDockWidget>
#include <QMovie>

#include <h5subobjecttovisualize.h>
#include <gwindow.h>

namespace Ui {
class ImageDockWidget;
}

class ImageDockWidget : public QDockWidget
{
    Q_OBJECT

public:
    /// XY, XZ, YZ slice type
    typedef enum SliceType {
        XY,
        XZ,
        YZ
    } SliceType;

    explicit ImageDockWidget(QWidget *parent = 0);
    ~ImageDockWidget();
    void clearGUI();
    void clearImage();
    void setMaximum(int value);
    void setValue(int value);
    int getValue();
    void setVisibleLoading(bool value);
    void showImage(const QImage &image, QPoint point, QString fileName);
    void setColor(QColor color);
    OpenedH5File::H5SubobjectToVisualize *getSubobject() const;
    void setSubobject(OpenedH5File::H5SubobjectToVisualize *value);
    GWindow *getGWindow() const;
    void setGWindow(GWindow *value);
    SliceType getSliceType() const;
    void setSliceType(const SliceType &value);

public slots:
    void repaintImage(QImage image, hsize_t index);

signals:
    void valueChanged(int value);
    void hoveredPointInImage(int x, int y);

private slots:
    void on_verticalSlider_valueChanged(int value);
    void on_imageWidget_hoveredPointInImage(int x, int y);

private:
    Ui::ImageDockWidget *ui;
    QMovie *movie;
    GWindow *gWindow = 0;
    OpenedH5File::H5SubobjectToVisualize *subobject = 0;
    SliceType sliceType = XY;

};

#endif // IMAGEDOCKWIDGET_H
