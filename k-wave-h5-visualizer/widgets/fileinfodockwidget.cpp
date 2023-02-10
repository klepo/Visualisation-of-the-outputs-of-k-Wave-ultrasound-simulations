/**
 * @file        fileinfodockwidget.cpp
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        9  October   2018 (created) <br>
 *              10 February  2023 (updated)
 *
 * @brief       The implementation file containing FileInfoDockWidget class definition.
 *
 * @license     This file is part of the k-Wave-h5-visualizer tool for processing the HDF5 data
 *              created by the k-Wave toolbox - http://www.k-wave.org. This file may be used,
 *              distributed and modified under the terms of the LGPL version 3 open source
 *              license. A copy of the LGPL license should have been received with this file.
 *              Otherwise, it can be found at: http://www.gnu.org/copyleft/lesser.html.
 *
 * @copyright   Copyright © 2019, Petr Kleparnik, VUT FIT Brno. All Rights Reserved.
 *
 */

#include "fileinfodockwidget.h"

/**
 * @brief Creates FileInfoDockWidget object
 * @param[in] parent Parent (optional)
 */
FileInfoDockWidget::FileInfoDockWidget(QWidget *parent)
    : QDockWidget(parent)
    , ui(new Ui::FileInfoDockWidget)
{
    ui->setupUi(this);
}

/**
 * @brief Destructor of FileInfoDockWidget
 *
 * Deletes ui and clears widget content.
 */
FileInfoDockWidget::~FileInfoDockWidget()
{
    clear();
    delete ui;
}

/**
 * @brief Sets file
 * @param[in] file H5OpenedFile
 */
void FileInfoDockWidget::setFile(const H5OpenedFile *file)
{
    // Set info from HDF5 file to GUI (dock panel)
    ui->textBrowserInfo->clear();
    foreach (QString key, file->getInfo().keys())
        ui->textBrowserInfo->append("<strong>" + key + "</strong><br>" + file->getInfo().value(key) + "<br>");
    QScrollBar *v = ui->textBrowserInfo->verticalScrollBar();
    // Scroll up
    v->setValue(v->minimum());
}

/**
 * @brief Clears file info
 */
void FileInfoDockWidget::clear()
{
    ui->textBrowserInfo->clear();
}
