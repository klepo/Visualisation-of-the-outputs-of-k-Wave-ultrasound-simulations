/**
 * @file        selecteddatasetdockwidget.h
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        9  October   2018 (created) <br>
 *              10 October   2018 (updated)
 *
 * @brief       The header file with SelectedDatasetDockWidget class declaration.
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

#ifndef SELECTEDDATASETDOCKWIDGET_H
#define SELECTEDDATASETDOCKWIDGET_H

#include <QDockWidget>

#include "ui_selecteddatasetdockwidget.h"

#include "abstractwidget.h"
#include "h5objecttovisualize.h"

namespace Ui {
class SelectedDatasetDockWidget;
}

class SelectedDatasetDockWidget : public QDockWidget, public AbstractWidget
{
    Q_OBJECT

public:
    explicit SelectedDatasetDockWidget(QWidget *parent = nullptr);
    ~SelectedDatasetDockWidget();

public slots:
    void setObject(H5ObjectToVisualize *object);
    void clear();

private:
    Q_DISABLE_COPY(SelectedDatasetDockWidget)

    Ui::SelectedDatasetDockWidget *ui;
};

#endif // SELECTEDDATASETDOCKWIDGET_H
