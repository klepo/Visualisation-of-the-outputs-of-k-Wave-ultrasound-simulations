/**
 * @file        datasetsdockwidget.cpp
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        9  October   2018 (created) <br>
 *              30 October   2018 (updated)
 *
 * @brief       The implementation file containing DatasetsDockWidget class definition.
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

#include "datasetsdockwidget.h"

/**
 * @brief Creates DatasetsDockWidget object
 * @param[in] parent Parent (optional)
 */
DatasetsDockWidget::DatasetsDockWidget(QWidget *parent) :
    QDockWidget(parent),
    ui(new Ui::DatasetsDockWidget)
{
    ui->setupUi(this);
}

/**
 * @brief Destructor of DatasetsDockWidget
 *
 * Deletes ui and clears widget content.
 */
DatasetsDockWidget::~DatasetsDockWidget()
{
    clear();
    delete ui;
}

/**
 * @brief Sets file
 * @param[in] file H5OpenedFile
 */
void DatasetsDockWidget::setFile(const H5OpenedFile *file)
{
    clear();
    ui->formLayout->addRow("Dataset", new QLabel("Show"));

    qRadioButtonGroup = new QButtonGroup(this);
    qCheckBoxGroup = new QButtonGroup(this);
    qCheckBoxGroup->setExclusive(false);

    int id = 0;
    // Load objects to visualize from file
    foreach (H5ObjectToVisualize *object, file->getObjects()) {
        // Create radioButton and checkbox
        QRadioButton *radioButton = new QRadioButton(object->getName());
        QCheckBox *checkBox = new QCheckBox();
        checkBox->setAccessibleName(object->getName());
        qRadioButtonGroup->addButton(radioButton, id);
        qCheckBoxGroup->addButton(checkBox, id);
        ui->formLayout->addRow(radioButton, checkBox);
        id++;
    }
    connect(qRadioButtonGroup, SIGNAL(buttonToggled(int, bool)), this, SIGNAL(datasetSelectionChanged(int, bool)));
    connect(qCheckBoxGroup, SIGNAL(buttonToggled(int, bool)), this, SLOT(showDataset(int, bool)));

    qRadioButtonGroup->button(0)->setChecked(true);
    //adjustSize();
    this->setMaximumHeight(16777215);
}

/**
 * @brief Clears datasets table and layout
 */
void DatasetsDockWidget::clear()
{
    if (qRadioButtonGroup) {
        delete qRadioButtonGroup;
        qRadioButtonGroup = nullptr;
    }
    if (qCheckBoxGroup) {
        delete qCheckBoxGroup;
        qCheckBoxGroup = nullptr;
    }
    clearLayout(ui->formLayout);
    this->setMaximumHeight(20);
    //adjustSize();
}

/**
 * @brief Shows dataset
 * @param[in] id Dataset index in table
 * @param[in] flag Selection flag
 *
 * Not yet used.
 * TODO Implement show fuction
 */
void DatasetsDockWidget::showDataset(int id, bool flag)
{
    Q_UNUSED(id);
    Q_UNUSED(flag);
}
