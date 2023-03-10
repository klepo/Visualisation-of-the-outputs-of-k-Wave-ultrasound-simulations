/**
 * @file        k-wave-h5-processing/main.cpp
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        30 July      2014 (created) <br>
 *              10 February  2023 (updated)
 *
 * @brief       The main implementation file containing k-Wave HDF5 processing application.
 *
 * @license     This file is part of the k-Wave-h5-processing tool for processing the HDF5 data
 *              created by the k-Wave toolbox - http://www.k-wave.org. This file may be used,
 *              distributed and modified under the terms of the LGPL version 3 open source
 *              license. A copy of the LGPL license should have been received with this file.
 *              Otherwise, it can be found at: http://www.gnu.org/copyleft/lesser.html.
 *
 * @copyright   Copyright © 2019, Petr Kleparnik, VUT FIT Brno. All Rights Reserved.
 *
 */

#include <k-wave-h5-helper.h>

#include "reshape.h"
#include "downsampling.h"
#include "changechunks.h"
#include "compress.h"
#include "decompress.h"
#include "difference.h"

/**
 * @brief Main k-Wave processing function
 * @param[in] argc Number of arguments
 * @param[in] argv Array of arguments
 * @return EXIT_SUCCESS
 */
int main(int argc, const char **argv)
{
    double t0 = H5Helper::getTime(); // Save the start time

    // Test 40bit coding
    /*//const int kMaxExpP = 138;
    const int kMaxExpP = 114;
    //H5Helper::floatC number(pow(2, 26-16) * 0x1FFFF, 134216704);
    //H5Helper::floatC number(pow(2, 26-16-15) * 0x1, 0.0312500000);
    H5Helper::floatC number(pow(2, 2-16) * 0x1FFFF, 7.999938964843750000000000000000);
    //H5Helper::floatC number(pow(2, 2-16-15) * 0x1, 0.000000001862645149230957031250);
    std::cout << std::fixed << std::setprecision(50) << number << std::endl;
    uint8_t in[5];
    H5Helper::CompressHelper::convertFloatCTo40b(number, in, kMaxExpP);
    H5Helper::CompressHelper::convert40bToFloatC(in, number, kMaxExpP);
    std::cout << float(number.imag()) << std::endl;
    return 0;*/
    // std::exit(EXIT_SUCCESS);

    Settings *settings = new Settings(argc, argv);

    FilesContext *filesContext = new FilesContext(settings);
    DtsForPcs *dtsForPcs       = new DtsForPcs(filesContext, settings);

    // Helper::printDebugTwoColumns2S("omp_get_max_threads", omp_get_max_threads());

    // TODO:
    // - kolize souborů?

    if (settings->getFlagReshape() || settings->getFlagDwnsmpl() || settings->getFlagChangeChunks()
        || settings->getFlagCompress() || settings->getFlagDecompress() || settings->getFlagDifference()) {
        // Processing of sensor mask
        if (settings->getFlagReshape()) {
            Helper::printDebugTitle("Reshaping");
            Reshape *reshape = new Reshape(filesContext->getPcsOutputFile(), dtsForPcs, settings);
            reshape->execute();
            delete reshape;
            reshape = nullptr;
        }

        // Downsampling
        if (settings->getFlagDwnsmpl()) {
            Helper::printDebugTitle("Downsampling");
            Downsampling *downsampling = new Downsampling(filesContext->getPcsOutputFile(), dtsForPcs, settings);
            downsampling->execute();
            delete downsampling;
            downsampling = nullptr;
        }

        // Copy 3D datasets a set new chunking
        if (settings->getFlagChangeChunks()) {
            Helper::printDebugTitle("Change chunks");
            ChangeChunks *changeChunks = new ChangeChunks(filesContext->getPcsOutputFile(), dtsForPcs, settings);
            changeChunks->execute();
            delete changeChunks;
            changeChunks = nullptr;
        }

        // Compression of time series data
        if (settings->getFlagCompress()) {
            Helper::printDebugTitle("Compression");
            Compress *compress = new Compress(filesContext->getPcsOutputFile(), dtsForPcs, settings);
            compress->execute();
            delete compress;
            compress = nullptr;
        }

        // Decompression of time series data
        if (settings->getFlagDecompress()) {
            Helper::printDebugTitle("Decompression");
            Decompress *decompress = new Decompress(filesContext->getPcsOutputFile(), dtsForPcs, settings);
            decompress->execute();
            delete decompress;
            decompress = nullptr;
        }

        // Subtraction of time series datasets
        if (settings->getFlagDifference()) {
            Helper::printDebugTitle("Difference");
            Difference *difference = new Difference(filesContext->getPcsOutputFile(), dtsForPcs, settings);
            difference->execute();
            delete difference;
            difference = nullptr;
        }
    }

    delete dtsForPcs;
    dtsForPcs = nullptr;
    Helper::printDebugTitle("Closing files");
    // Close files
    delete filesContext;
    filesContext = nullptr;

    double t1 = H5Helper::getTime(); // Save the final time

    Helper::printDebugLine();
    Helper::printDebugTime("the entire process", t0, t1);
    Helper::printDebugLine();

    std::exit(EXIT_SUCCESS);
}
