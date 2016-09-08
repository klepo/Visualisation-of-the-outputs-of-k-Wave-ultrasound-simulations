/**
 * @file        main.cpp
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.0
 * @date        30 July      2014 (created)
 *              6  December  2015 (updated)
 *              8  September 2015 (updated)
 *
 * @brief       The main file with creation of graphics application (MainWindow window)
 *

 * @license     This file is partof k-Wave visualiser application
 * for visualizing HDF5 data created by the k-Wave toolbox - http://www.k-wave.org.
 *
 * @copyright   Copyright © 2016, Petr Kleparnik, VUT FIT Brno. All Rights Reserved.
 *
 * k-Wave visualiser is free software.
 */

#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    app.setApplicationName("k-Wave H5 Visualizer");
    app.setApplicationVersion("0.0");
    app.setOrganizationName("VUT FIT Brno");

    MainWindow window;
    window.show();

    return app.exec();
}
