/**
 * @file        main.cpp
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        30 July      2014 (created)
 *              6  December  2015 (updated)
 *              8  September 2016 (updated)
 *              3  November  2016 (updated)
 *
 * @brief       The implementation file containing k-Wave HDF5 processing application.
 *
 * @license     This application is for preprocessing the HDF5 data created by the k-Wave toolbox - http://www.k-wave.org.
 *              k-Wave h5 processing is free software.
 *
 * @copyright   Copyright © 2016, Petr Kleparnik, VUT FIT Brno. All Rights Reserved.
 *
 */

#include <hdf5helper.h>
#include <processing.h>

/**
 * @brief Main function
 * @param argc
 * @param argv
 * @return EXIT_SUCCESS
 */
int main(int argc, char **argv)
{
    /*float u[] = {2, 4, 5};
    float v[] = {3, 2, 7};
    //float w[9] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
    const int s = 5;
    float w[s];
    int p[s];

    //Processing::conv(u, v, w, 3, 7);
    Processing::xcorr(u, v, w, 3, 3);
    //Processing::diff(v, w, 8);
    //Processing::findPeaks(v, p, s);

    for (int i = 0; i < s; i++)
        std::cout << w[i] << std::endl;

    //std::cout << Processing::mean(v, s);
    return 0;*/


    double t0 = HDF5Helper::getTime(); // Save the start time

    Settings *settings = new Settings();
    settings->loadParams(argc, argv);

    FilesContext *filesContext = new FilesContext(settings);
    DtsForPcs *dtsForPcs = new DtsForPcs(filesContext, settings);
    Processing *processing = new Processing(filesContext->getHDF5PcsOutputFile(), dtsForPcs, settings);

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

    double t1 = HDF5Helper::getTime(); // Save the final time

    std::cout << std::endl << std::endl << "Time of the entire process: " << (t1 - t0) << " ms; \t" << std::endl << std::endl << std::endl;

    std::exit(EXIT_SUCCESS);
}
