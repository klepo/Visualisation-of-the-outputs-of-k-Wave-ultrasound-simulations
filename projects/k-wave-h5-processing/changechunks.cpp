/**
 * @file        changechunks.cpp
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        8  September 2016 (created) \n
 *              11 September 2017 (updated)
 *
 * @brief       The implementation file containing ChangeChunks class definition.
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

#include "changechunks.h"

/**
 * @brief Creates ChangeChunks object
 * @param[in] outputFile Output file
 * @param[in] dtsForPcs Datasets for processing
 * @param[in] settings Processing settings
 */
ChangeChunks::ChangeChunks(HDF5Helper::File *outputFile, DtsForPcs *dtsForPcs, Settings *settings)
    : Processing(outputFile, dtsForPcs, settings)
{

}

/**
 * @brief Executes processing
 */
void ChangeChunks::execute()
{
    std::vector<HDF5Helper::DatasetType> types = {
        HDF5Helper::DatasetType::BASIC_3D,
        HDF5Helper::DatasetType::DWNSMPL_3D,
        HDF5Helper::DatasetType::CUBOID,
        HDF5Helper::DatasetType::CUBOID_ATTR
    };

    try {
        HDF5Helper::MapOfDatasets map = getDtsForPcs()->getDatasets();

        int count = 0;
        for (HDF5Helper::MapOfDatasetsIt it = map.begin(); it != map.end(); ++it) {
            HDF5Helper::Dataset *dataset = it->second;
            HDF5Helper::DatasetType datasetType = dataset->getType();

            if (checkDatasetType(datasetType, types)) {
                Helper::printDebugMsg("Change chunks of dataset " + dataset->getName());
                changeChunksOfDataset(dataset);
                count++;
                Helper::printDebugMsg("Change chunks of dataset " + dataset->getName() + " done");
            }
        }
        if (count == 0) {
            Helper::printErrorMsg("No datasets for changing chunks in simulation output file");
        }
    } catch(std::exception &e) {
        Helper::printErrorMsg(e.what());
        std::exit(EXIT_FAILURE);
    }
}

/**
 * @brief Changes chunks of dataset
 * @param[in] srcDataset Source dataset
 */
void ChangeChunks::changeChunksOfDataset(HDF5Helper::Dataset *srcDataset, bool log)
{
    // Dims
    HDF5Helper::Vector dims = srcDataset->getDims();

    // Chunk dims
    HDF5Helper::Vector chunkDims(dims.getLength(), 1);

    // Set new chunk dims
    #pragma omp parallel for
    for (hssize_t i = 0; i < hssize_t(dims.getLength()); i++) {
        chunkDims[i] = getSettings()->getMaxChunkSize();
        if (chunkDims[i] > dims[i]) chunkDims[i] = dims[i];
    }

    // Create destination dataset
    getOutputFile()->createDatasetF(srcDataset->getName(), dims, chunkDims, true, log);
    HDF5Helper::Dataset *dstDataset = getOutputFile()->openDataset(srcDataset->getName(), log);

    double t0 = HDF5Helper::getTime();

    float *data = 0;
    float minV, maxV;
    hsize_t minVIndex, maxVIndex;
    float minVG, maxVG;
    hsize_t minVGIndex, maxVGIndex;
    bool first = true;
    HDF5Helper::Vector offset;
    HDF5Helper::Vector count;

    // Change chunks
    for (hsize_t i = 0; i < srcDataset->getNumberOfBlocks(); i++) {
        srcDataset->readBlock(i, offset, count, data, minV, maxV, minVIndex, maxVIndex, log);
        dstDataset->writeDataset(offset, count, data, log);
        delete[] data;
        if (first) {
            minVG = minV;
            maxVG = maxV;
            first = false;
        }

        hsize_t linearOffset;
        convertMultiDimToLinear(offset, linearOffset, srcDataset->getDims());

        if (minVG > minV) {
            minVG = minV;
            minVGIndex = linearOffset + minVIndex;
        }
        if (maxVG < maxV) {
            maxVG = maxV;
            maxVGIndex = linearOffset + maxVIndex;
        }
    }

    // Copy attributes
    copyAttributes(srcDataset, dstDataset);

    // Set min/max values
    dstDataset->setAttribute(HDF5Helper::MIN_ATTR, minVG, log);
    dstDataset->setAttribute(HDF5Helper::MAX_ATTR, maxVG, log);
    dstDataset->setAttribute(HDF5Helper::MIN_INDEX_ATTR, minVGIndex, log);
    dstDataset->setAttribute(HDF5Helper::MAX_INDEX_ATTR, maxVGIndex, log);

    double t1 = HDF5Helper::getTime();
    Helper::printDebugTime("changing chunks of the whole dataset", t0, t1);

    getOutputFile()->closeDataset(dstDataset, log);
}
