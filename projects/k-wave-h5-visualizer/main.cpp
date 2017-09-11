/**
 * @file        k-wave-h5-visualizer/main.cpp
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        30 July      2014 (created) \n
 *              11 September 2017 (updated)
 *
 * @brief       The main implementation file containing k-Wave HDF5 visualizer application.
 *
 * @license     This file is part of the k-Wave-h5-visualizer tool for processing the HDF5 data
 *              created by the k-Wave toolbox - http://www.k-wave.org. This file may be used,
 *              distributed and modified under the terms of the LGPL version 3 open source
 *              license. A copy of the LGPL license should have been recieved with this file.
 *              Otherwise, it can be found at: http://www.gnu.org/copyleft/lesser.html.
 *
 * @copyright   Copyright © 2017, Petr Kleparnik, VUT FIT Brno. All Rights Reserved.
 *
 */

#include "mainwindow.h"

#include <QApplication>

/**
 * @brief Main k-Wave visualizer function
 * @param[in] argc Number of arguments
 * @param[in] argv Array of argumnets
 * @return EXIT_SUCCESS
 */
int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    app.setApplicationName("k-Wave H5 Visualizer");
    app.setApplicationVersion("1.1");
    app.setOrganizationName("VUT FIT Brno");

    MainWindow window;
    window.show();

    return app.exec();
}
