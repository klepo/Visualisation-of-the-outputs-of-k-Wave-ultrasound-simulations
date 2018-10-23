/**
 * @file        settings3ddockwidget.cpp
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        9  October   2018 (created) <br>
 *              10 October   2018 (updated)
 *
 * @brief       The implementation file containing Settings3DDockWidget class definition.
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

#include <settings3ddockwidget.h>

Settings3DDockWidget::Settings3DDockWidget(QWidget *parent) :
    QDockWidget(parent),
    ui(new Ui::Settings3DDockWidget)
{
    ui->setupUi(this);

    // 3D interpolation mode
    connect(ui->comboBoxInterpolationMode, SIGNAL(currentIndexChanged(int)), this, SLOT(setInterpolationMode(int)));

    // VR mode
    connect(ui->comboBoxVolumeRenderingMode, SIGNAL(currentIndexChanged(int)), this, SLOT(setVolumeRenderingMode(int)));

    // VR slices count
    connect(ui->horizontalSliderVRSlices, SIGNAL(valueChanged(int)), this, SLOT(setVolumeRenderingSlices(int)));
}

Settings3DDockWidget::~Settings3DDockWidget()
{
    delete ui;
}

int Settings3DDockWidget::getInterpolationMode() const
{
    return ui->comboBoxInterpolationMode->currentIndex();
}

int Settings3DDockWidget::getVolumeRenderingMode() const
{
    return ui->comboBoxVolumeRenderingMode->currentIndex();
}

int Settings3DDockWidget::getVolumeRenderingSlices() const
{
    return ui->horizontalSliderVRSlices->value();
}

void Settings3DDockWidget::setInterpolationMode(int value)
{
    ui->comboBoxInterpolationMode->setCurrentIndex(value);
    emit interpolationModeChanged(value);
}

void Settings3DDockWidget::setVolumeRenderingMode(int value)
{
    ui->comboBoxVolumeRenderingMode->setCurrentIndex(value);
    emit volumeRenderingModeChanged(value);
}

void Settings3DDockWidget::setVolumeRenderingSlices(int value)
{
    ui->horizontalSliderVRSlices->setValue(value);
    emit volumeRenderingSlicesChanged(value);
}
