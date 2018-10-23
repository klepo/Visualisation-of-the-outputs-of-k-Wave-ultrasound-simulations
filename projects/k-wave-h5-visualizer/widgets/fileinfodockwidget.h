/**
 * @file        fileinfodockwidget.h
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        9  October   2018 (created) <br>
 *              9  October   2018 (updated)
 *
 * @brief       The header file with FileInfoDockWidget class declaration.
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

#ifndef FILEINFODOCKWIDGET_H
#define FILEINFODOCKWIDGET_H

#include <QDockWidget>
#include <QScrollBar>

#include <ui_fileinfodockwidget.h>

#include <abstractwidget.h>
#include <h5objecttovisualize.h>

namespace Ui {
class FileInfoDockWidget;
}

class FileInfoDockWidget : public QDockWidget, public AbstractWidget
{
    Q_OBJECT

public:
    explicit FileInfoDockWidget(QWidget *parent = nullptr);
    ~FileInfoDockWidget();

public slots:
    void setFile(H5OpenedFile *file);
    void clear();

private:
    Q_DISABLE_COPY(FileInfoDockWidget)

    Ui::FileInfoDockWidget *ui;
};

#endif // FILEINFODOCKWIDGET_H
