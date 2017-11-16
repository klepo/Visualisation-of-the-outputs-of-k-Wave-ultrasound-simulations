#ifndef MINMAXGROUPBOX_H
#define MINMAXGROUPBOX_H

#include <QGroupBox>

#include <abstractobjectwidget.h>

namespace Ui {
class MinMaxGroupBox;
}

class MinMaxGroupBox : public QGroupBox, public AbstractObjectWidget
{
    Q_OBJECT

public:
    explicit MinMaxGroupBox(QWidget *parent = 0);
    ~MinMaxGroupBox();

signals:
    void setTrim(bool);

public slots:
    void setObject(H5ObjectToVisualize *value);
    void clear();

private slots:
    void on_horizontalSliderGlobalMin_valueChanged(int value);
    void on_horizontalSliderGlobalMax_valueChanged(int value);
    void on_doubleSpinBoxMinGlobal_valueChanged(double value);
    void on_doubleSpinBoxMaxGlobal_valueChanged(double value);

private:
    Ui::MinMaxGroupBox *ui;
};

#endif // MINMAXGROUPBOX_H
