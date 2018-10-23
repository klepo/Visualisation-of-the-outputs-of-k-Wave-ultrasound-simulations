/**
 * @file        datasetsdockwidget.h
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        9  October   2018 (created) <br>
 *              9  October   2018 (updated)
 *
 * @brief       The header file with DatasetsDockWidget class declaration.
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

#ifndef DATASETSDOCKWIDGET_H
#define DATASETSDOCKWIDGET_H

#include <QDockWidget>
#include <QButtonGroup>
#include <QRadioButton>
#include <QCheckBox>
#include <QLabel>

#include <ui_datasetsdockwidget.h>

#include <abstractwidget.h>
#include <h5objecttovisualize.h>

namespace Ui {
class DatasetsDockWidget;
}

class DatasetsDockWidget : public QDockWidget, public AbstractWidget
{
    Q_OBJECT

public:
    explicit DatasetsDockWidget(QWidget *parent = nullptr);
    ~DatasetsDockWidget();

signals:
    void datasetSelectionChanged(int id, bool flag);

public slots:
    void setFile(H5OpenedFile *file);
    void clear();

private slots:
    void showDataset(int id, bool flag);

private:
    Q_DISABLE_COPY(DatasetsDockWidget)

    QButtonGroup *qRadioButtonGroup = nullptr;
    QButtonGroup *qCheckBoxGroup = nullptr;
    Ui::DatasetsDockWidget *ui;
};

#endif // DATASETSDOCKWIDGET_H
