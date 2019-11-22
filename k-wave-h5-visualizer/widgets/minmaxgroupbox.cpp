/**
 * @file        minmaxgroupbox.cpp
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        9  October   2018 (created) <br>
 *              27 March     2019 (updated)
 *
 * @brief       The implementation file containing MinMaxGroupBox class definition.
 *
 * @license     This file is part of the k-Wave-h5-visualizer tool for processing the HDF5 data
 *              created by the k-Wave toolbox - http://www.k-wave.org. This file may be used,
 *              distributed and modified under the terms of the LGPL version 3 open source
 *              license. A copy of the LGPL license should have been received with this file.
 *              Otherwise, it can be found at: http://www.gnu.org/copyleft/lesser.html.
 *
 * @copyright   Copyright © 2019, Petr Kleparnik, VUT FIT Brno. All Rights Reserved.
 *
 */

#include "minmaxgroupbox.h"

/**
 * @brief Creates MinMaxGroupBox object
 * @param[in] parent Parent (optional)
 */
MinMaxGroupBox::MinMaxGroupBox(QWidget *parent) :
    QGroupBox(parent),
    ui(new Ui::MinMaxGroupBox)
{
    ui->setupUi(this);
    connect(ui->checkBoxTrim, SIGNAL(toggled(bool)), this, SIGNAL(minMaxValuesTrimChanged(bool)));
}

/**
 * @brief Destructor of MinMaxGroupBox
 *
 * Deletes ui and sets default values.
 */
MinMaxGroupBox::~MinMaxGroupBox()
{
    clear();
    delete ui;
}

/**
 * @brief Returns minimal value
 * @return Minimal value
 */
float MinMaxGroupBox::getMinValue() const
{
    return float(ui->doubleSpinBoxMinGlobal->value());
}

/**
 * @brief Returns maximal value
 * @return Maximal value
 */
float MinMaxGroupBox::getMaxValue() const
{
    return float(ui->doubleSpinBoxMaxGlobal->value());
}

/**
 * @brief Returns min/max values trim
 * @return Min/max values trim
 */
bool MinMaxGroupBox::getMinMaxValuesTrim() const
{
    return ui->checkBoxTrim->isChecked();
}

/**
 * @brief Sets range
 * @param[in] min Minimum
 * @param[in] max Maximum
 */
void MinMaxGroupBox::setRange(float min, float max)
{
    clear();
    ui->doubleSpinBoxMinGlobal->setRange(double(min), double(max));
    ui->doubleSpinBoxMaxGlobal->setRange(double(min), double(max));
    ui->doubleSpinBoxMinGlobal->setSingleStep(double(max - min) / DIVIDER);
    ui->doubleSpinBoxMaxGlobal->setSingleStep(double(max - min) / DIVIDER);
    ui->doubleSpinBoxMinGlobal->setValue(double(min));
    ui->doubleSpinBoxMaxGlobal->setValue(double(max));
}

/**
 * @brief Sets minimal and maximal values
 * @param[in] min Minimal value
 * @param[in] max Maximal value
 */
void MinMaxGroupBox::setValues(float min, float max)
{
    ui->doubleSpinBoxMinGlobal->setValue(double(min));
    ui->doubleSpinBoxMaxGlobal->setValue(double(max));
}

/**
 * @brief Sets minimal value
 * @param[in] min Minimal value
 */
void MinMaxGroupBox::setMinValue(float min)
{
    ui->doubleSpinBoxMinGlobal->setValue(double(min));
}

/**
 * @brief Sets maximal value
 * @param[in] max Maximal value
 */
void MinMaxGroupBox::setMaxValue(float max)
{
    ui->doubleSpinBoxMaxGlobal->setValue(double(max));
}

/**
 * @brief Sets minimal and maximal values trim
 * @param[in] value True/False
 */
void MinMaxGroupBox::setMinMaxValuesTrim(bool value)
{
    ui->checkBoxTrim->setChecked(value);
}

/**
 * @brief Sets defualt values a disconnects signals
 */
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

/**
 * @brief On horizontal slider min value changed
 * @param[in] value Slider value
 */
void MinMaxGroupBox::on_horizontalSliderGlobalMin_valueChanged(int value)
{
    // Recompute doubles to integers...
    ui->doubleSpinBoxMinGlobal->setValue(double(value) / DIVIDER * (ui->doubleSpinBoxMinGlobal->maximum() - ui->doubleSpinBoxMinGlobal->minimum()) + ui->doubleSpinBoxMinGlobal->minimum());
}

/**
 * @brief On horizontal slider max value changed
 * @param[in] value Slider value
 */
void MinMaxGroupBox::on_horizontalSliderGlobalMax_valueChanged(int value)
{
    // Recompute doubles to integers...
    ui->doubleSpinBoxMaxGlobal->setValue(double(value) / DIVIDER * (ui->doubleSpinBoxMaxGlobal->maximum() - ui->doubleSpinBoxMaxGlobal->minimum()) + ui->doubleSpinBoxMaxGlobal->minimum());
}

/**
 * @brief On double spin box min value changed
 * @param[in] value Spin box value
 */
void MinMaxGroupBox::on_doubleSpinBoxMinGlobal_valueChanged(double value)
{
    // Recompute integers to doubles
    ui->horizontalSliderGlobalMin->setTracking(false);
    ui->horizontalSliderGlobalMin->setSliderPosition(qRound(DIVIDER * (value - ui->doubleSpinBoxMinGlobal->minimum()) / (ui->doubleSpinBoxMinGlobal->maximum() - ui->doubleSpinBoxMinGlobal->minimum())));
    ui->horizontalSliderGlobalMin->setTracking(true);
    emit minValueChanged(float(value));
}

/**
 * @brief On double spin box max value changed
 * @param[in] value Spin box value
 */
void MinMaxGroupBox::on_doubleSpinBoxMaxGlobal_valueChanged(double value)
{
    // Recompute integers to doubles
    ui->horizontalSliderGlobalMax->setTracking(false);
    ui->horizontalSliderGlobalMax->setSliderPosition(qRound(DIVIDER * (value - ui->doubleSpinBoxMaxGlobal->minimum()) / (ui->doubleSpinBoxMaxGlobal->maximum() - ui->doubleSpinBoxMaxGlobal->minimum())));
    ui->horizontalSliderGlobalMax->setTracking(true);
    emit maxValueChanged(float(value));
}
