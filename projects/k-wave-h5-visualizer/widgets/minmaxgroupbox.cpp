/**
 * @file        minmaxgroupbox.cpp
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        9  October   2018 (created) <br>
 *              10 October   2018 (updated)
 *
 * @brief       The implementation file containing MinMaxGroupBox class definition.
 *
 * @license     This file is part of the k-Wave-h5-visualizer tool for processing the HDF5 data
 *              created by the k-Wave toolbox - http://www.k-wave.org. This file may be used,
 *              distributed and modified under the terms of the LGPL version 3 open source
 *              license. A copy of the LGPL license should have been received with this file.
 *              Otherwise, it can be found at: http://www.gnu.org/copyleft/lesser.html.
 *
 * @copyright   Copyright © 2018, Petr Kleparnik, VUT FIT Brno. All Rights Reserved.
 *
 */

#include <minmaxgroupbox.h>

MinMaxGroupBox::MinMaxGroupBox(QWidget *parent) :
    QGroupBox(parent),
    ui(new Ui::MinMaxGroupBox)
{
    ui->setupUi(this);

    connect(ui->checkBoxTrim, SIGNAL(toggled(bool)), this, SIGNAL(minMaxValuesTrimChanged(bool)));
}

MinMaxGroupBox::~MinMaxGroupBox()
{
    clear();
    delete ui;
}

float MinMaxGroupBox::getMinValue() const
{
    return float(ui->doubleSpinBoxMinGlobal->value());
}

float MinMaxGroupBox::getMaxValue() const
{
    return float(ui->doubleSpinBoxMaxGlobal->value());
}

bool MinMaxGroupBox::getMinMaxValuesTrim() const
{
    return ui->checkBoxTrim->isChecked();
}

void MinMaxGroupBox::setRange(float min, float max)
{
    clear();
    ui->doubleSpinBoxMinGlobal->setRange(double(min), double(max));
    ui->doubleSpinBoxMaxGlobal->setRange(double(min), double(max));
    ui->doubleSpinBoxMinGlobal->setSingleStep(double(max - min) / 1000);
    ui->doubleSpinBoxMaxGlobal->setSingleStep(double(max - min) / 1000);
    ui->doubleSpinBoxMinGlobal->setValue(double(min));
    ui->doubleSpinBoxMaxGlobal->setValue(double(max));
}

void MinMaxGroupBox::setValues(float min, float max)
{
    ui->doubleSpinBoxMinGlobal->setValue(double(min));
    ui->doubleSpinBoxMaxGlobal->setValue(double(max));
}

void MinMaxGroupBox::setMinValue(float min)
{
    ui->doubleSpinBoxMinGlobal->setValue(double(min));
}

void MinMaxGroupBox::setMaxValue(float max)
{
    ui->doubleSpinBoxMaxGlobal->setValue(double(max));
}

void MinMaxGroupBox::setMinMaxValuesTrim(bool value)
{
    ui->checkBoxTrim->setChecked(value);
}

void MinMaxGroupBox::clear()
{
    disconnect(this, SIGNAL(minValueChanged(float)), nullptr, nullptr);
    disconnect(this, SIGNAL(maxValueChanged(float)), nullptr, nullptr);
    disconnect(this, SIGNAL(minMaxValuesTrimChanged(bool)), nullptr, nullptr);
    ui->doubleSpinBoxMinGlobal->setRange(0, 0);
    ui->doubleSpinBoxMaxGlobal->setRange(0, 0);
    ui->doubleSpinBoxMinGlobal->setSingleStep(0);
    ui->doubleSpinBoxMaxGlobal->setSingleStep(0);
    ui->doubleSpinBoxMinGlobal->setValue(0);
    ui->doubleSpinBoxMaxGlobal->setValue(0);
}

void MinMaxGroupBox::on_horizontalSliderGlobalMin_valueChanged(int value)
{
    // Recompute doubles to integers...
    ui->doubleSpinBoxMinGlobal->setValue(double(value) / 1000 * (ui->doubleSpinBoxMinGlobal->maximum() - ui->doubleSpinBoxMinGlobal->minimum()) + ui->doubleSpinBoxMinGlobal->minimum());
}

void MinMaxGroupBox::on_horizontalSliderGlobalMax_valueChanged(int value)
{
    // Recompute doubles to integers...
    ui->doubleSpinBoxMaxGlobal->setValue(double(value) / 1000 * (ui->doubleSpinBoxMaxGlobal->maximum() - ui->doubleSpinBoxMaxGlobal->minimum()) + ui->doubleSpinBoxMaxGlobal->minimum());
}

void MinMaxGroupBox::on_doubleSpinBoxMinGlobal_valueChanged(double value)
{
    // Recompute integers to doubles
    ui->horizontalSliderGlobalMin->setTracking(false);
    ui->horizontalSliderGlobalMin->setSliderPosition(qRound(1000 * (value - ui->doubleSpinBoxMinGlobal->minimum()) / (ui->doubleSpinBoxMinGlobal->maximum() - ui->doubleSpinBoxMinGlobal->minimum())));
    ui->horizontalSliderGlobalMin->setTracking(true);
    emit minValueChanged(float(value));
}

void MinMaxGroupBox::on_doubleSpinBoxMaxGlobal_valueChanged(double value)
{
    // Recompute integers to doubles
    ui->horizontalSliderGlobalMax->setTracking(false);
    ui->horizontalSliderGlobalMax->setSliderPosition(qRound(1000 * (value - ui->doubleSpinBoxMaxGlobal->minimum()) / (ui->doubleSpinBoxMaxGlobal->maximum() - ui->doubleSpinBoxMaxGlobal->minimum())));
    ui->horizontalSliderGlobalMax->setTracking(true);
    emit maxValueChanged(float(value));
}
