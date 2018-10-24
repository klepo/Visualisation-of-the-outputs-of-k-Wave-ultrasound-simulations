/**
 * @file        window3ddockwidget.cpp
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        9  October   2018 (created) <br>
 *              10 October   2018 (updated)
 *
 * @brief       The implementation file containing Window3DDockWidget class definition.
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

#include "window3ddockwidget.h"

Window3DDockWidget::Window3DDockWidget(QWidget *parent) :
    QDockWidget(parent),
    ui(new Ui::Window3DDockWidget)
{
    ui->setupUi(this);

    // Create OpenGL window
    gWindow = new GWindow(this);
    // Widget from QWindow
    QWidget *widget = createWindowContainer(gWindow);
    widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    QVBoxLayout *qVBoxLayout = static_cast<QVBoxLayout *>(ui->dockWidgetContents3D->layout());
    qVBoxLayout->insertWidget(0, widget);

    // Create loading animation
    //ui->label3DLoading->setVisible(false);
    movie = new QMovie(":/icons/icons/loading.gif");
    movie->setCacheMode(QMovie::CacheAll);
    movie->start();
    //ui->label3DLoading->setMovie(movie);
}

Window3DDockWidget::~Window3DDockWidget()
{
    clear();
    delete ui;
    delete gWindow;
    gWindow = nullptr;
}

GWindow *Window3DDockWidget::getGWindow() const
{
    return gWindow;
}

void Window3DDockWidget::setObject(H5ObjectToVisualize *object)
{
    clear();
    gWindow->setObject(object);
    connect(object, SIGNAL(data3DLoadingStarted()), this, SLOT(showLabel3DLoading()));
    connect(object, SIGNAL(currentData3DLoaded()), this, SLOT(hideLabel3DLoading()));
}

void Window3DDockWidget::clear()
{
    gWindow->clear();
    hideLabel3DLoading();
}

void Window3DDockWidget::toggleLabel3DLoading(bool value)
{
    if (value) {
        showLabel3DLoading();
    } else {
        hideLabel3DLoading();
    }
}

void Window3DDockWidget::hideLabel3DLoading()
{
    ui->label3DLoading->clear();
}

void Window3DDockWidget::showLabel3DLoading()
{
    ui->label3DLoading->setMovie(movie);
}
