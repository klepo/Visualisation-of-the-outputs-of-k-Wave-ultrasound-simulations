/*
 * @file        main.cpp
 * @author      Petr Kleparnik, VUT FIT Brno, xklepa01@stud.fit.vutbr.cz
 * @version     0.0
 * @date        30 July      2014 (created)
 *              6  December  2015 (updated)
 *
 * @brief       The implementation file containing k-Wave HDF5 processing application.
 *
 * @section     Licence
 * This application is for preprocessing HDF5 data created by the k-Wave toolbox - http://www.k-wave.org.
 * Copyright © 2014, Petr Kleparnik, VUT FIT Brno.
 * k-Wave h5 processing is free software.
 */

#include <iostream>
#include <string>
#include <time.h>

#include <hdf5helper.h>

#include <processing.h>

/**
 * @brief main Main function
 * @param argc
 * @param argv
 * @return EXIT_SUCCESS
 */
int main(int argc, char **argv)
{
    double t0 = HDF5Helper::getTime();

    Settings *settings = new Settings();
    settings->loadParams(argc, argv);

    FilesContext *filesContext = new FilesContext(settings);
    DtsForPcs *dtsForPcs = new DtsForPcs(filesContext, settings);
    Processing *processing = new Processing(filesContext->getHDF5OutputFile(), dtsForPcs, settings);

    // Processing of sensor mask
    if (settings->getFlagReshape()) {
        Helper::printDebugTitle("Reshaping");
        processing->reshape();
    }

    // Downsampling
    if (settings->getFlagDwnsmpl()) {
        Helper::printDebugTitle("Downsampling");
        processing->donwsampling();
    }

    // Copy 3D datasets a set new chunking
    if (settings->getFlagChangeChunks()) {
        Helper::printDebugTitle("Change chunks");
        processing->changeChunks();
    }

    Helper::printDebugTitle("Closing files");

    // Close files
    delete filesContext;

    double t2 = HDF5Helper::getTime();

    std::cout << std::endl << std::endl << "Time of the entire process: " << (t2-t0) << " ms; \t" << std::endl << std::endl << std::endl;

    std::exit(EXIT_SUCCESS);
}
