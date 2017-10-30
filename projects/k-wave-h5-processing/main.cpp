/**
 * @file        k-wave-h5-processing/main.cpp
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        30 July      2014 (created) \n
 *              19 September 2017 (updated)
 *
 * @brief       The main implementation file containing k-Wave HDF5 processing application.
 *
 * @license     This file is part of the k-Wave-h5-processing tool for processing the HDF5 data
 *              created by the k-Wave toolbox - http://www.k-wave.org. This file may be used,
 *              distributed and modified under the terms of the LGPL version 3 open source
 *              license. A copy of the LGPL license should have been received with this file.
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
 * @param[in] argc Number of arguments
 * @param[in] argv Array of arguments
 * @return EXIT_SUCCESS
 */
int main(int argc, char **argv)
{
    double t0 = HDF5Helper::getTime(); // Save the start time

    Settings *settings = new Settings(argc, argv);

    FilesContext *filesContext = new FilesContext(settings);
    DtsForPcs *dtsForPcs = new DtsForPcs(filesContext, settings);

    //Helper::printDebugTwoColumns2S("omp_get_max_threads", omp_get_max_threads());

    // TODO:
    // - odzkoušet downsampling cuboidů atd.
    // OK - udělat rozdíl datasetů - výpočet chyby
    // - šlo by odhadnout mos?
    // OK - potřebuji znát periodu
    // OK - funkce pro přidávání lomítka
    // OK - sjednotit určité části komprese a dekomprese
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
            Reshape *reshape = new Reshape(filesContext->getPcsOutputFile(), dtsForPcs, settings);
            reshape->execute();
            delete reshape;
        }

        // Downsampling
        if (settings->getFlagDwnsmpl()) {
            Helper::printDebugTitle("Downsampling");
            Downsampling *downsampling = new Downsampling(filesContext->getPcsOutputFile(), dtsForPcs, settings);
            downsampling->execute();
            delete downsampling;
        }

        // Copy 3D datasets a set new chunking
        if (settings->getFlagChangeChunks()) {
            Helper::printDebugTitle("Change chunks");
            ChangeChunks *changeChunks = new ChangeChunks(filesContext->getPcsOutputFile(), dtsForPcs, settings);
            changeChunks->execute();
            delete changeChunks;
        }

        // Compression of time series data
        if (settings->getFlagCompress()) {
            Helper::printDebugTitle("Compression");
            Compress *compress = new Compress(filesContext->getPcsOutputFile(), dtsForPcs, settings);
            compress->execute();
            delete compress;
        }

        // Decompression of time series data
        if (settings->getFlagDecompress()) {
            Helper::printDebugTitle("Decompression");
            Decompress *decompress = new Decompress(filesContext->getPcsOutputFile(), dtsForPcs, settings);
            decompress->execute();
            delete decompress;
        }

        // Subtraction of time series datasets
        if (settings->getFlagDifference()) {
            Helper::printDebugTitle("Difference");
            Difference *difference = new Difference(filesContext->getPcsOutputFile(), dtsForPcs, settings);
            difference->execute();
            delete difference;
        }
    }

    delete dtsForPcs;
    Helper::printDebugTitle("Closing files");
    // Close files
    delete filesContext;

    double t1 = HDF5Helper::getTime(); // Save the final time

    Helper::printDebugTime("the entire process", t0, t1);
    Helper::printDebugMsg("----------------------------------------");

    std::exit(EXIT_SUCCESS);
}
