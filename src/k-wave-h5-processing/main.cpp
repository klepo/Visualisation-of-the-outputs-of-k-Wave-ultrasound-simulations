/**
 * @file
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        30 July      2014 (created) \n
 *              27 February  2017 (updated)
 *
 * @brief       The implementation file containing k-Wave HDF5 processing application.
 *
 * @license     This application is for preprocessing the HDF5 data created by the k-Wave toolbox - http://www.k-wave.org.
 *              k-Wave h5 processing is free software.
 *
 * @copyright   Copyright © 2017, Petr Kleparnik, VUT FIT Brno. All Rights Reserved.
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
    double t0 = HDF5Helper::getTime(); // Save the start time

    Settings *settings = new Settings();
    settings->loadParams(argc, argv);

    FilesContext *filesContext = new FilesContext(settings);
    DtsForPcs *dtsForPcs = new DtsForPcs(filesContext, settings);
    Processing *processing = new Processing(filesContext->getHDF5PcsOutputFile(), dtsForPcs, settings);

    // TODO:
    // - odzkoušet downsampling cuboidů atd.
    // - udělat rozdíl datasetů - výpočet chyby
    // - šlo by odhadnout mos?
    // - potřebuji znát periodu
    // - funkce pro přidávání lomítka
    // - sjednotit určité části komprese a dekomprese
    // - ošetřit maximální počet prvků podle RAM
    // - extrapolace sinus
    // - funkce pro error hlášky
    // - vyřešit případy, kdy je vstup i výstup stejný
    // - harmonické frekvence


    //std::exit(EXIT_SUCCESS);

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

    // Compression of time series data
    if (settings->getFlagCompress()) {
        Helper::printDebugTitle("Compression");
        processing->compress();
    }

    // Decompression of time series data
    if (settings->getFlagDecompress()) {
        Helper::printDebugTitle("Decompression");
        processing->decompress();
    }

    // Substraction of time series datasets
    if (settings->getFlagDifference()) {
        Helper::printDebugTitle("Difference");
        processing->difference();
    }

    Helper::printDebugTitle("Closing files");

    delete processing;
    delete dtsForPcs;
    // Close files
    delete filesContext;

    double t1 = HDF5Helper::getTime(); // Save the final time

    std::cout << std::endl << std::endl << "Time of the entire process: " << (t1 - t0) << " ms; \t" << std::endl << std::endl << std::endl;

    std::exit(EXIT_SUCCESS);
}