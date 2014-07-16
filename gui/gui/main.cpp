/*
 * @file        main.cpp
 * @author      Petr Kleparnik, VUT FIT Brno, xklepa01@stud.fit.vutbr.cz
 * @version     0.0
 * @date        30 July 2014
 *
 * @brief       The main file with creation of graphics application (MainWindow window)
 *
 * @section     Licence
 * This file is part of k-Wave visualiser application
 * for visualizing HDF5 data created by the k-Wave toolbox - http://www.k-wave.org.
 * Copyright © 2014, Petr Kleparnik, VUT FIT Brno.
 * k-Wave visualiser is free software.
 */

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QMainWindow>
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    app.setApplicationName("k-Wave HDF5 visualizer");
    app.setApplicationVersion("0.0");
    app.setOrganizationName("VUT FIT Brno");

    MainWindow window;
    window.show();

    return app.exec();
}
