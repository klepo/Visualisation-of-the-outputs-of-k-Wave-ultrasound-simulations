/**
 * @file        selecteddatasetdockwidget.cpp
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        9  October   2018 (created) <br>
 *              29 October   2018 (updated)
 *
 * @brief       The implementation file containing SelectedDatasetDockWidget class definition.
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

#include "selecteddatasetdockwidget.h"

/**
 * @brief Creates SelectedDatasetDockWidget object
 * @param[in] parent Parent (optional)
 */
SelectedDatasetDockWidget::SelectedDatasetDockWidget(QWidget *parent) :
    QDockWidget(parent),
    ui(new Ui::SelectedDatasetDockWidget)
{
    ui->setupUi(this);
}

/**
 * @brief Destructor of SelectedDatasetDockWidget
 *
 * Deletes ui and clears selected dataset settings.
 */
SelectedDatasetDockWidget::~SelectedDatasetDockWidget()
{
    clear();
    delete ui;
}

/**
 * @brief Sets H5ObjectToVisualize object
 * @param[in] object H5ObjectToVisualize
 */
void SelectedDatasetDockWidget::setObject(H5ObjectToVisualize *object)
{
    clear();

    // Set time series series control
    ui->groupBoxTimeSeriesControl->setSteps(int(object->getSteps()));
    ui->groupBoxTimeSeriesControl->setCurrentStep(int(object->getCurrentStep()));
    connect(ui->groupBoxTimeSeriesControl, SIGNAL(stepChanged(int)), object, SLOT(setCurrentStep(int)));
    connect(object, SIGNAL(stepChanged(int)), ui->groupBoxTimeSeriesControl, SLOT(setCurrentStep(int)));
    connect(object, SIGNAL(currentStepLoaded()), ui->groupBoxTimeSeriesControl, SLOT(continuePlaying()));

    // Set opacity controls
    ui->groupBoxOpacity->setValue(object->getOpacity());
    connect(ui->groupBoxOpacity, SIGNAL(valueChanged(QVector<float>)), object, SLOT(setOpacity(QVector<float>)));

    // Set colormap controls
    ui->comboBoxColormap->setCurrentIndex(object->getColormap());
    connect(ui->comboBoxColormap, SIGNAL(currentIndexChanged(int)), object, SLOT(setColormap(int)));

    // Set min and max controls
    ui->groupBoxMinMax->setRange(object->getOriginalMinValue(), object->getOriginalMaxValue());
    ui->groupBoxMinMax->setValues(object->getMinValue(), object->getMaxValue());
    ui->groupBoxMinMax->setMinMaxValuesTrim(object->getMinMaxValuesTrim());
    connect(ui->groupBoxMinMax, SIGNAL(minValueChanged(float)), object, SLOT(setMinValue(float)));
    connect(ui->groupBoxMinMax, SIGNAL(maxValueChanged(float)), object, SLOT(setMaxValue(float)));
    connect(ui->groupBoxMinMax, SIGNAL(minMaxValuesTrimChanged(bool)), object, SLOT(setMinMaxValuesTrim(bool)));

    connect(ui->pushButtonMax, SIGNAL(clicked()), object, SLOT(setToMaxValuePosition()));
    connect(ui->pushButtonMin, SIGNAL(clicked()), object, SLOT(setToMinValuePosition()));

    // Set dataset info
    ui->groupBoxSelectedDatasetInfo->setInfo(object->getInfo());
}

/**
 * @brief Clears selected dataset settings and disconnects signals
 */
void SelectedDatasetDockWidget::clear()
{
    // Reset time series control
    ui->groupBoxTimeSeriesControl->clear();

    // Reset opacity controls
    ui->groupBoxOpacity->clear();

    // Reset colormap controls
    disconnect(ui->comboBoxColormap, SIGNAL(currentIndexChanged(int)), nullptr, nullptr);
    ui->comboBoxColormap->setCurrentIndex(ColorMap::Type::JET);

    // Reset min and max controls
    ui->groupBoxMinMax->clear();

    disconnect(ui->pushButtonMax, SIGNAL(clicked()), nullptr, nullptr);
    disconnect(ui->pushButtonMin, SIGNAL(clicked()), nullptr, nullptr);

    // Reset dataset info
    ui->groupBoxSelectedDatasetInfo->clear();
}
