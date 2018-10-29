/**
 * @file        settings3ddockwidget.cpp
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        9  October   2018 (created) <br>
 *              29 October   2018 (updated)
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

#include "settings3ddockwidget.h"

/**
 * @brief Creates Settings3DDockWidget object
 * @param[in] parent Parent (optional)
 */
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

/**
 * @brief Destructor of Settings3DDockWidget
 *
 * Deletes ui.
 */
Settings3DDockWidget::~Settings3DDockWidget()
{
    delete ui;
}

/**
 * @brief Returns interpolation mode
 * @return Interpolation mode
 */
int Settings3DDockWidget::getInterpolationMode() const
{
    return ui->comboBoxInterpolationMode->currentIndex();
}

/**
 * @brief Returns volume rendering mode
 * @return Volume rendering mode
 */
int Settings3DDockWidget::getVolumeRenderingMode() const
{
    return ui->comboBoxVolumeRenderingMode->currentIndex();
}

/**
 * @brief Returns volume rendering slices count
 * @return Volume rendering slices count
 */
int Settings3DDockWidget::getVolumeRenderingSlices() const
{
    return ui->horizontalSliderVRSlices->value();
}

/**
 * @brief Sets interpolation mode
 * @param[in] value Interpolation mode
 */
void Settings3DDockWidget::setInterpolationMode(int value)
{
    ui->comboBoxInterpolationMode->setCurrentIndex(value);
    emit interpolationModeChanged(value);
}

/**
 * @brief Sets volume rendering mode
 * @param[in] value Volume rendering mode
 */
void Settings3DDockWidget::setVolumeRenderingMode(int value)
{
    ui->comboBoxVolumeRenderingMode->setCurrentIndex(value);
    emit volumeRenderingModeChanged(value);
}

/**
 * @brief Sets volume rendering slices count
 * @param[in] value Volume rendering slices count
 */
void Settings3DDockWidget::setVolumeRenderingSlices(int value)
{
    ui->horizontalSliderVRSlices->setValue(value);
    emit volumeRenderingSlicesChanged(value);
}
