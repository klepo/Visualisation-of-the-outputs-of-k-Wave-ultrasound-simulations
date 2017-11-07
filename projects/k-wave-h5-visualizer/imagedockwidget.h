#ifndef IMAGEDOCKWIDGET_H
#define IMAGEDOCKWIDGET_H

#include <QDockWidget>
#include <QMovie>

namespace Ui {
class ImageDockWidget;
}

class ImageDockWidget : public QDockWidget
{
    Q_OBJECT

public:
    explicit ImageDockWidget(QWidget *parent = 0);
    ~ImageDockWidget();
    void clearGUI();
    void clearImage();
    void setMaximum(int value);
    void setValue(int value);
    int getValue();
    void setVisibleLoading(bool value);
    void showImage(const QImage &image, QPoint point, QString fileName);

private:
    Ui::ImageDockWidget *ui;
    QMovie *movie;

};

#endif // IMAGEDOCKWIDGET_H
