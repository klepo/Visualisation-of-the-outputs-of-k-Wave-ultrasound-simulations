#include "groupboxopacity.h"
#include "ui_groupboxopacity.h"

GroupBoxOpacity::GroupBoxOpacity(QWidget *parent) :
    QGroupBox(parent),
    ui(new Ui::GroupBoxOpacity)
{
    ui->setupUi(this);
    opacity = QVector<float>(5, 1);
}

GroupBoxOpacity::~GroupBoxOpacity()
{
    delete ui;
}

void GroupBoxOpacity::on_doubleSpinBox_0_valueChanged(double value)
{
    ui->verticalSlider_0->setValue(int(value * 1000));
    opacity[0] = float(value);
    if (gWindow != 0)
        gWindow->changeOpacity(opacity);
}

void GroupBoxOpacity::on_verticalSlider_0_valueChanged(int value)
{
    ui->doubleSpinBox_0->setValue(double(value) / 1000);
}

void GroupBoxOpacity::on_doubleSpinBox_1_valueChanged(double value)
{
    ui->verticalSlider_1->setValue(int(value * 1000));
    opacity[1] = float(value);
    if (gWindow != 0)
        gWindow->changeOpacity(opacity);
}

void GroupBoxOpacity::on_verticalSlider_1_valueChanged(int value)
{
    ui->doubleSpinBox_1->setValue(double(value) / 1000);
}

void GroupBoxOpacity::on_doubleSpinBox_2_valueChanged(double value)
{
    ui->verticalSlider_2->setValue(int(value * 1000));
    opacity[2] = float(value);
    if (gWindow != 0)
        gWindow->changeOpacity(opacity);
}

void GroupBoxOpacity::on_verticalSlider_2_valueChanged(int value)
{
    ui->doubleSpinBox_2->setValue(double(value) / 1000);
}

void GroupBoxOpacity::on_doubleSpinBox_3_valueChanged(double value)
{
    ui->verticalSlider_3->setValue(int(value * 1000));
    opacity[3] = float(value);
    if (gWindow != 0)
        gWindow->changeOpacity(opacity);
}

void GroupBoxOpacity::on_verticalSlider_3_valueChanged(int value)
{
    ui->doubleSpinBox_3->setValue(double(value) / 1000);
}

void GroupBoxOpacity::on_doubleSpinBox_4_valueChanged(double value)
{
    int nv = int(value * 1000);
    ui->verticalSlider_4->setValue(nv);
    opacity[4] = float(value);
    if (gWindow != 0)
        gWindow->changeOpacity(opacity);
}

void GroupBoxOpacity::on_verticalSlider_4_valueChanged(int value)
{
    ui->doubleSpinBox_4->setValue(double(value) / 1000);
}

GWindow *GroupBoxOpacity::getGWindow() const
{
    return gWindow;
}

void GroupBoxOpacity::setGWindow(GWindow *value)
{
    gWindow = value;
}
