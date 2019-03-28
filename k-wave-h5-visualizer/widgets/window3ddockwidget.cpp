/**
 * @file        window3ddockwidget.cpp
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        9  October   2018 (created) <br>
 *              27 March     2019 (updated)
 *
 * @brief       The implementation file containing Window3DDockWidget class definition.
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

#include "window3ddockwidget.h"

/**
 * @brief Creates Window3DDockWidget object
 * @param[in] parent Parent (optional)
 */
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

/**
 * @brief Destructor of Window3DDockWidget
 *
 * Deletes ui, gWindow and clears gWindow.
 */
Window3DDockWidget::~Window3DDockWidget()
{
    //clear();
    delete ui;
    delete gWindow;
    gWindow = nullptr;
}

/**
 * @brief Returns OpenGL window
 * @return OpenGL window
 */
GWindow *Window3DDockWidget::getGWindow() const
{
    return gWindow;
}

/**
 * @brief Sets H5ObjectToVisualize object
 * @param[in] object H5ObjectToVisualize
 */
void Window3DDockWidget::setObject(H5ObjectToVisualize *object)
{
    clear();
    gWindow->setObject(object);
    connect(object, SIGNAL(data3DLoadingStarted()), this, SLOT(showLabel3DLoading()));
    connect(object, SIGNAL(currentData3DLoaded()), this, SLOT(hideLabel3DLoading()));
    connect(object, SIGNAL(last3DReadingTimeNs(qint64)), this, SLOT(showReadingTime(qint64)));

}

/**
 * @brief Clears OpenGL window
 */
void Window3DDockWidget::clear()
{
    gWindow->clear();
    hideLabel3DLoading();
}

/**
 * @brief Toggles label 3D loading animation
 * @param[in] value True/False
 */
void Window3DDockWidget::toggleLabel3DLoading(bool value)
{
    if (value) {
        showLabel3DLoading();
    } else {
        hideLabel3DLoading();
    }
}

/**
 * @brief Hides label 3D loading animation
 */
void Window3DDockWidget::hideLabel3DLoading()
{
    ui->label3DLoading->clear();
}

/**
 * @brief Shows label 3D loading animation
 */
void Window3DDockWidget::showLabel3DLoading()
{
    ui->label3DLoading->setMovie(movie);
}

/**
 * @brief Shows reading time
 * @param[in] value Time in nanosecons
 */
void Window3DDockWidget::showReadingTime(qint64 value)
{
    ui->labelInfo->setText("Last 3D read time: " + QString::number(double(value) / 1000000, 'f', 3) + " ms");
}

