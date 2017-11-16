#ifndef SLICEDOCKWIDGET_H
#define SLICEDOCKWIDGET_H

#include <QDockWidget>
#include <QMovie>

#include <abstractobjectwidget.h>

namespace Ui {
class SliceDockWidget;
}

class SliceDockWidget : public QDockWidget, public AbstractObjectWidget
{
    Q_OBJECT

public:
    /// XY, XZ, YZ slice type
    typedef enum SliceType {
        XY,
        XZ,
        YZ
    } SliceType;

    explicit SliceDockWidget(QWidget *parent = 0);
    ~SliceDockWidget();
    SliceType getSliceType() const;
    void setSliceType(const SliceType &value);

public slots:
    void clear();
    void setObject(H5ObjectToVisualize *value);

signals:
    void hoveredPointInImage(int x, int y);

private slots:
    void on_verticalSlider_valueChanged(int value);
    void on_imageWidget_hoveredPointInImage(int x, int y);
    void repaintImage(QImage image);

private:
    Ui::SliceDockWidget *ui;
    SliceType sliceType = XY;

    QMovie *movie;

    void setMaximum(int value);
    void setValue(int value);
    void setColor(QColor color);
};

#endif // SLICEDOCKWIDGET_H
