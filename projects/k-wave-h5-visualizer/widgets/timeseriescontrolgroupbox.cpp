/**
 * @file        timeseriescontrolgroupbox.cpp
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        9  October   2018 (created) <br>
 *              10 October   2018 (updated)
 *
 * @brief       The implementation file containing TimeSeriesControlGroupBox class definition.
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

#include "timeseriescontrolgroupbox.h"

TimeSeriesControlGroupBox::TimeSeriesControlGroupBox(QWidget *parent) :
    QGroupBox(parent),
    ui(new Ui::TimeSeriesControlGroupBox)
{
    ui->setupUi(this);

    // Create timer for animation of data series and connect it
    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(updateStep()));
}

TimeSeriesControlGroupBox::~TimeSeriesControlGroupBox()
{
    delete ui;
    delete timer;
    timer = nullptr;
}

void TimeSeriesControlGroupBox::setSteps(int steps)
{
    clear();
    ui->spinBoxSelectedDatasetStep->setMaximum(steps - 1);
    ui->horizontalSliderSelectedDatasetStep->setMaximum(steps - 1);
    ui->spinBoxTMIncrement->setMaximum(steps - 1);
    ui->spinBoxTMIncrement->setValue(1);
    ui->spinBoxSelectedDatasetStep->setValue(0);
}

void TimeSeriesControlGroupBox::setCurrentStep(int step)
{
    ui->spinBoxSelectedDatasetStep->setValue(step);
}

void TimeSeriesControlGroupBox::continuePlaying()
{
    if (playing && !timer->isActive()) {
        timer->start(ui->spinBoxTMInterval->value());
    }
}

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


void TimeSeriesControlGroupBox::on_spinBoxSelectedDatasetStep_valueChanged(int step)
{
    emit stepChanged(step);
}

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

void TimeSeriesControlGroupBox::on_toolButtonStart_clicked()
{
    timer->stop();
    playing = false;
    ui->toolButtonPlay->setChecked(false);
    ui->spinBoxSelectedDatasetStep->setValue(0);
}

void TimeSeriesControlGroupBox::on_toolButtonEnd_clicked()
{
    timer->stop();
    playing = false;
    ui->toolButtonPlay->setChecked(false);
    ui->spinBoxSelectedDatasetStep->setValue(ui->spinBoxSelectedDatasetStep->maximum());
}

void TimeSeriesControlGroupBox::on_spinBoxTMInterval_valueChanged(int value)
{
    timer->setInterval(value);
}
