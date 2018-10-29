/**
 * @file        datasetinfogroupbox.h
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        9  October   2018 (created) <br>
 *              29 October   2018 (updated)
 *
 * @brief       The header file with DatasetInfoGroupBox class declaration.
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

#ifndef DATASETINFOGROUPBOX_H
#define DATASETINFOGROUPBOX_H

#include <QGroupBox>
#include <QLabel>

#include "ui_datasetinfogroupbox.h"

#include "abstractwidget.h"

namespace Ui {
class DatasetInfoGroupBox;
}

/**
 * @brief The DatasetInfoGroupBox class represents wrapper for the dataset info group box
 */
class DatasetInfoGroupBox : public QGroupBox, public AbstractWidget
{
    Q_OBJECT

public:
    explicit DatasetInfoGroupBox(QWidget *parent = nullptr);
    ~DatasetInfoGroupBox();

    QList<QPair<QString, QString>> getInfo() const;

public slots:
    void setInfo(QList<QPair<QString, QString>> info);
    void clear();

private:
    Q_DISABLE_COPY(DatasetInfoGroupBox)

    /// User interface
    Ui::DatasetInfoGroupBox *ui;
    /// Dataset info list
    QList<QPair<QString, QString>> info;
};

#endif // DATASETINFOGROUPBOX_H
