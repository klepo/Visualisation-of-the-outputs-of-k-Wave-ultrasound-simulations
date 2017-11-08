#ifndef GROUPBOXOPACITY_H
#define GROUPBOXOPACITY_H

#include <QGroupBox>

#include <gwindow.h>

namespace Ui {
class GroupBoxOpacity;
}

class GroupBoxOpacity : public QGroupBox
{
    Q_OBJECT

public:
    explicit GroupBoxOpacity(QWidget *parent = 0);
    ~GroupBoxOpacity();
    GWindow *getGWindow() const;
    void setGWindow(GWindow *value);

public slots:
    void on_doubleSpinBox_0_valueChanged(double value);
    void on_verticalSlider_0_valueChanged(int value);
    void on_doubleSpinBox_1_valueChanged(double value);
    void on_verticalSlider_1_valueChanged(int value);
    void on_doubleSpinBox_2_valueChanged(double value);
    void on_verticalSlider_2_valueChanged(int value);
    void on_doubleSpinBox_3_valueChanged(double value);
    void on_verticalSlider_3_valueChanged(int value);
    void on_doubleSpinBox_4_valueChanged(double value);
    void on_verticalSlider_4_valueChanged(int value);

private:
    Ui::GroupBoxOpacity *ui;
    GWindow *gWindow = 0;
    QVector<float> opacity;
};

#endif // GROUPBOXOPACITY_H
