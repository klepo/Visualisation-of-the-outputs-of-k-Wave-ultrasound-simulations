/**
 * @file        datasetinfogroupbox.cpp
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        9  October   2018 (created) <br>
 *              9  October   2018 (updated)
 *
 * @brief       The implementation file containing DatasetInfoGroupBox class definition.
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

#include <datasetinfogroupbox.h>

DatasetInfoGroupBox::DatasetInfoGroupBox(QWidget *parent) :
    QGroupBox(parent),
    ui(new Ui::DatasetInfoGroupBox)
{
    ui->setupUi(this);
}

DatasetInfoGroupBox::~DatasetInfoGroupBox()
{
    clear();
    delete ui;
}

QList<QPair<QString, QString> > DatasetInfoGroupBox::getInfo() const
{
    return info;
}

void DatasetInfoGroupBox::setInfo(QList<QPair<QString, QString>> info)
{
    clear();
    this->info = info;
    for (int i = 0; i < info.count(); ++i)
        ui->formLayoutSelectedDatasetInfo->addRow(new QLabel(info[i].first + ":"), new QLabel(info[i].second));
}

void DatasetInfoGroupBox::clear()
{
    info = QList<QPair<QString, QString>>();
    clearLayout(ui->formLayoutSelectedDatasetInfo);
}

