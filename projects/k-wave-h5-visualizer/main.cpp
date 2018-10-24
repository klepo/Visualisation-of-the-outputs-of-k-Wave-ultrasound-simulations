/**
 * @file        k-wave-h5-visualizer/main.cpp
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        30 July      2014 (created) <br>
 *              9  October   2018 (updated)
 *
 * @brief       The main implementation file containing k-Wave HDF5 visualizer application.
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

#include <QApplication>

#include "mainwindow.h"

/**
 * @brief Main k-Wave visualizer function
 * @param[in] argc Number of arguments
 * @param[in] argv Array of arguments
 * @return EXIT_SUCCESS
 */
int main(int argc, char *argv[])
{
    QCoreApplication::setAttribute(Qt::AA_UseDesktopOpenGL); // Forces the usage of desktop OpenGL
    QApplication app(argc, argv);

    app.setApplicationName("k-Wave H5 Visualizer");
    app.setApplicationVersion("1.1");
    app.setOrganizationName("VUT FIT Brno");

    MainWindow window;
    window.show();

    return app.exec();
}
