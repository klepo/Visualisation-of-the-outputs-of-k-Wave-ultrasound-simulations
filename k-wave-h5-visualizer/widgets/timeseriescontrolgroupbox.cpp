/**
 * @file        timeseriescontrolgroupbox.cpp
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        9  October   2018 (created) <br>
 *              10 February  2023 (updated)
 *
 * @brief       The implementation file containing TimeSeriesControlGroupBox class definition.
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

#include "timeseriescontrolgroupbox.h"

/**
 * @brief Creates TimeSeriesControlGroupBox object
 * @param[in] parent Parent (optional)
 */
TimeSeriesControlGroupBox::TimeSeriesControlGroupBox(QWidget *parent)
    : QGroupBox(parent)
    , ui(new Ui::TimeSeriesControlGroupBox)
{
    ui->setupUi(this);

    // Create timer for animation of data series and connect it
    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(updateStep()));
}

/**
 * @brief Destructor of TimeSeriesControlGroupBox
 *
 * Deletes ui and timer.
 */
TimeSeriesControlGroupBox::~TimeSeriesControlGroupBox()
{
    delete ui;
    delete timer;
    timer = nullptr;
}

/**
 * @brief Set total number of steps
 * @param[in] steps Number of steps
 */
void TimeSeriesControlGroupBox::setSteps(int steps)
{
    clear();
    ui->spinBoxSelectedDatasetStep->setMaximum(steps - 1);
    ui->horizontalSliderSelectedDatasetStep->setMaximum(steps - 1);
    ui->spinBoxTMIncrement->setMaximum(steps - 1);
    ui->spinBoxTMIncrement->setValue(1);
    ui->spinBoxSelectedDatasetStep->setValue(0);
}

/**
 * @brief Sets current step
 * @param[in] step Step
 */
void TimeSeriesControlGroupBox::setCurrentStep(int step)
{
    ui->spinBoxSelectedDatasetStep->setValue(step);
}

/**
 * @brief Continues playing if is active
 */
void TimeSeriesControlGroupBox::continuePlaying()
{
    if (playing && !timer->isActive()) {
        timer->start(ui->spinBoxTMInterval->value());
    }
}

/**
 * @brief Disconnects signals, stops playing a resets values
 */
void TimeSeriesControlGroupBox::clear()
{
    disconnect(this, SIGNAL(stepChanged(int)), nullptr, nullptr);

    // Stop animation
    timer->stop();
    playing = false;
    ui->toolButtonPlay->setChecked(false);

    // Reset time series control
    ui->spinBoxSelectedDatasetStep->setMaximum(0);
    ui->horizontalSliderSelectedDatasetStep->setMaximum(0);
    ui->spinBoxTMIncrement->setMaximum(1);
    ui->spinBoxTMInterval->setMaximum(5000);
    ui->spinBoxTMInterval->setMinimum(0);
    ui->spinBoxSelectedDatasetStep->setValue(0);
    ui->spinBoxTMIncrement->setValue(1);
    ui->spinBoxTMInterval->setValue(0);
}

/**
 * @brief Updates step
 */
void TimeSeriesControlGroupBox::updateStep()
{
    // Get current step
    int step = ui->spinBoxSelectedDatasetStep->value();
    // Increment of step
    step += ui->spinBoxTMIncrement->value();
    // End of time series
    if (step > ui->spinBoxSelectedDatasetStep->maximum()) {
        // Stop timer and playing
        timer->stop();
        playing = false;
        ui->toolButtonPlay->setChecked(false);
        // Set last step
        ui->spinBoxSelectedDatasetStep->setValue(ui->spinBoxSelectedDatasetStep->maximum());
    } else {
        // Stop timer and update step
        timer->stop();
        ui->spinBoxSelectedDatasetStep->setValue(step);
    }
}

/**
 * @brief Spin box selected dataset step value changed slot
 * @param[in] step Spin box value
 */
void TimeSeriesControlGroupBox::on_spinBoxSelectedDatasetStep_valueChanged(int step)
{
    emit stepChanged(step);
}

/**
 * @brief Tool button play clicked slot
 * @param[in] checked Play button state
 */
void TimeSeriesControlGroupBox::on_toolButtonPlay_clicked(bool checked)
{
    if (checked) {
        timer->start(ui->spinBoxTMInterval->value());
        playing = true;
    } else {
        timer->stop();
        playing = false;
    }
}

/**
 * @brief Tool button start clicked slot
 */
void TimeSeriesControlGroupBox::on_toolButtonStart_clicked()
{
    timer->stop();
    playing = false;
    ui->toolButtonPlay->setChecked(false);
    ui->spinBoxSelectedDatasetStep->setValue(0);
}

/**
 * @brief Tool button end clicked slot
 */
void TimeSeriesControlGroupBox::on_toolButtonEnd_clicked()
{
    timer->stop();
    playing = false;
    ui->toolButtonPlay->setChecked(false);
    ui->spinBoxSelectedDatasetStep->setValue(ui->spinBoxSelectedDatasetStep->maximum());
}

/**
 * @brief Spin box time interval value changed slot
 * @param[in] value Spin box value
 */
void TimeSeriesControlGroupBox::on_spinBoxTMInterval_valueChanged(int value)
{
    timer->setInterval(value);
}
