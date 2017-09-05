/**
 * @file        k-wave-h5-processing/main.cpp
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        30 July      2014 (created) \n
 *              28 August    2017 (updated)
 *
 * @brief       The main implementation file containing k-Wave HDF5 processing application.
 *
 * @license     This file is part of the k-Wave-h5-processing tool for processing the HDF5 data
 *              created by the k-Wave toolbox - http://www.k-wave.org. This file may be used,
 *              distributed and modified under the terms of the LGPL version 3 open source
 *              license. A copy of the LGPL license should have been recieved with this file.
 *              Otherwise, it can be found at: http://www.gnu.org/copyleft/lesser.html.
 *
 * @copyright   Copyright © 2017, Petr Kleparnik, VUT FIT Brno. All Rights Reserved.
 *
 */

#include <hdf5helper.h>

#include <reshape.h>
#include <downsampling.h>
#include <changechunks.h>
#include <compress.h>
#include <decompress.h>
#include <difference.h>

/**
 * @brief Main k-Wave processing function
 * @param argc Number of arguments
 * @param argv Array of argumnets
 * @return EXIT_SUCCESS
 */
int main(int argc, char **argv)
{
    std::cout << "omp_get_max_threads: " << omp_get_max_threads() << std::endl;

    double t0 = HDF5Helper::getTime(); // Save the start time

    Settings *settings = new Settings();
    settings->loadParams(argc, argv);

    FilesContext *filesContext = new FilesContext(settings);
    DtsForPcs *dtsForPcs = new DtsForPcs(filesContext, settings);

    // TODO:
    // - odzkoušet downsampling cuboidů atd.
    // OK - udělat rozdíl datasetů - výpočet chyby
    // - šlo by odhadnout mos?
    // - potřebuji znát periodu
    // - funkce pro přidávání lomítka
    // - sjednotit určité části komprese a dekomprese
    // - ošetřit maximální počet prvků podle RAM
    // - funkce pro error hlášky
    // - vyřešit případy, kdy je vstup i výstup stejný
    // OK - harmonické frekvence
    // - kolize souborů

    //std::exit(EXIT_SUCCESS);

    if (settings->getFlagReshape()
            || settings->getFlagDwnsmpl()
            || settings->getFlagChangeChunks()
            || settings->getFlagCompress()
            || settings->getFlagDecompress()
            || settings->getFlagDifference()
            ) {
        // Processing of sensor mask
        if (settings->getFlagReshape()) {
            Helper::printDebugTitle("Reshaping");
            Reshape *reshape = new Reshape(filesContext->getHDF5PcsOutputFile(), dtsForPcs, settings);
            reshape->execute();
            delete reshape;
        }

        // Downsampling
        if (settings->getFlagDwnsmpl()) {
            Helper::printDebugTitle("Downsampling");
            Downsampling *downsampling = new Downsampling(filesContext->getHDF5PcsOutputFile(), dtsForPcs, settings);
            downsampling->execute();
            delete downsampling;
        }

        // Copy 3D datasets a set new chunking
        if (settings->getFlagChangeChunks()) {
            Helper::printDebugTitle("Change chunks");
            ChangeChunks *changeChunks = new ChangeChunks(filesContext->getHDF5PcsOutputFile(), dtsForPcs, settings);
            changeChunks->execute();
            delete changeChunks;
        }

        // Compression of time series data
        if (settings->getFlagCompress()) {
            Helper::printDebugTitle("Compression");
            Compress *compress = new Compress(filesContext->getHDF5PcsOutputFile(), dtsForPcs, settings);
            compress->execute();
            delete compress;
        }

        // Decompression of time series data
        if (settings->getFlagDecompress()) {
            Helper::printDebugTitle("Decompression");
            Decompress *decompress = new Decompress(filesContext->getHDF5PcsOutputFile(), dtsForPcs, settings);
            decompress->execute();
            delete decompress;
        }

        // Substraction of time series datasets
        if (settings->getFlagDifference()) {
            Helper::printDebugTitle("Difference");
            Difference *difference = new Difference(filesContext->getHDF5PcsOutputFile(), dtsForPcs, settings);
            difference->execute();
            delete difference;
        }
    }

    delete dtsForPcs;
    Helper::printDebugTitle("Closing files");
    // Close files
    delete filesContext;

    double t1 = HDF5Helper::getTime(); // Save the final time

    std::cout << std::endl << std::endl << "Time of the entire process: " << (t1 - t0) << " ms; \t" << std::endl << std::endl << std::endl;

    std::exit(EXIT_SUCCESS);
}
