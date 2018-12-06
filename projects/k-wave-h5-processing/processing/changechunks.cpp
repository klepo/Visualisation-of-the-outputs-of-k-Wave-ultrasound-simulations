/**
 * @file        changechunks.cpp
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        8  September 2016 (created) <br>
 *              29 November  2018 (updated)
 *
 * @brief       The implementation file containing ChangeChunks class definition.
 *
 * @license     This file is part of the k-Wave-h5-processing tool for processing the HDF5 data
 *              created by the k-Wave toolbox - http://www.k-wave.org. This file may be used,
 *              distributed and modified under the terms of the LGPL version 3 open source
 *              license. A copy of the LGPL license should have been received with this file.
 *              Otherwise, it can be found at: http://www.gnu.org/copyleft/lesser.html.
 *
 * @copyright   Copyright © 2018, Petr Kleparnik, VUT FIT Brno. All Rights Reserved.
 *
 */

#include "changechunks.h"

/**
 * @brief Creates ChangeChunks object
 * @param[in] outputFile Output file
 * @param[in] dtsForPcs Datasets for processing
 * @param[in] settings Processing settings
 */
ChangeChunks::ChangeChunks(H5Helper::File *outputFile, DtsForPcs *dtsForPcs, const Settings *settings)
    : Processing(outputFile, dtsForPcs, settings)
{
}

/**
 * @brief Executes processing
 */
void ChangeChunks::execute()
{
    try {
        H5Helper::MapOfDatasets map = getDtsForPcs()->getDatasets();
        hsize_t sensorMaskSize = getDtsForPcs()->getSensorMaskSize();
        int count = 0;
        for (H5Helper::MapOfDatasetsIt it = map.begin(); it != map.end(); ++it) {
            H5Helper::Dataset *dataset = it->second;
            H5Helper::DatasetType datasetType = dataset->getType(sensorMaskSize);
            if (datasetType != H5Helper::DatasetType::UNKNOWN && dataset->isFloatType()) {
                Helper::printDebugMsg("Change chunks of dataset " + dataset->getName());
                changeChunksOfDataset(dataset, getSettings()->getFlagLog());
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
 * @param[in] log Logging flag (optional)
 */
void ChangeChunks::changeChunksOfDataset(H5Helper::Dataset *srcDataset, bool log)
{
    // Dims
    H5Helper::Vector dims = srcDataset->getDims();

    // Chunk dims
    H5Helper::Vector chunkDims(dims.getLength(), 1);

    // Set new chunk dims
    ParamsDefinition::VectorOfULongLongs maxChunkSizes = getSettings()->getMaxChunkSizes();
    //std::reverse(maxChunkSizes.begin(), maxChunkSizes.end());
    for (hsize_t i = 0; i < maxChunkSizes.size() && i < chunkDims.getLength(); i++) {
        chunkDims[dims.getLength() - i - 1] = maxChunkSizes.at(i);
        if (chunkDims[dims.getLength() - i - 1] > dims[dims.getLength() - i - 1])
            chunkDims[dims.getLength() - i - 1] = dims[dims.getLength() - i - 1];
    }

    Helper::printDebugTwoColumns2S("Chunk dims", srcDataset->getChunkDims());
    Helper::printDebugTwoColumns2S("New chunk dims", chunkDims);

    // Create destination dataset
    getOutputFile()->createDatasetF(srcDataset->getName(), dims, chunkDims, true, log);
    H5Helper::Dataset *dstDataset = getOutputFile()->openDataset(srcDataset->getName(), log);

    double t0 = H5Helper::getTime();

    float *data = new float[srcDataset->getGeneralBlockDims().getSize()]();
    float minV = std::numeric_limits<float>::max();
    float maxV = std::numeric_limits<float>::min();
    hsize_t minVIndex = 0;
    hsize_t maxVIndex = 0;
    float minVG = std::numeric_limits<float>::max();
    float maxVG = std::numeric_limits<float>::min();
    hsize_t minVGIndex = 0;
    hsize_t maxVGIndex = 0;
    H5Helper::Vector offset;
    H5Helper::Vector count;

    std::vector<H5Helper::DatasetType> compressTypes = {
        H5Helper::DatasetType::TIME_STEPS_C_INDEX,
        H5Helper::DatasetType::CUBOID_C,
        H5Helper::DatasetType::CUBOID_ATTR_C
    };

    bool findMinMaxFlag = true;
    if (checkDatasetType(srcDataset->getType(), compressTypes)) {
        findMinMaxFlag = false;
    }


    // Change chunks
    for (hsize_t i = 0; i < srcDataset->getNumberOfBlocks(); i++) {
        srcDataset->readBlock(i, offset, count, data, minV, maxV, minVIndex, maxVIndex, log);
        dstDataset->writeDataset(offset, count, data, log);

        hsize_t linearOffset;
        convertMultiDimToLinear(offset, linearOffset, srcDataset->getDims());
        if (findMinMaxFlag) {
            H5Helper::checkOrSetMinMaxValue(minVG, maxVG, minV, maxV, minVGIndex, maxVGIndex, linearOffset + minVIndex, linearOffset + maxVIndex);
        }
    }
    delete[] data;
    data = nullptr;

    // Copy attributes
    copyAttributes(srcDataset, dstDataset);

    // Set min/max values
    if (findMinMaxFlag) {
        dstDataset->setAttribute(H5Helper::MIN_ATTR, minVG, log);
        dstDataset->setAttribute(H5Helper::MAX_ATTR, maxVG, log);
        dstDataset->setAttribute(H5Helper::MIN_INDEX_ATTR, minVGIndex, log);
        dstDataset->setAttribute(H5Helper::MAX_INDEX_ATTR, maxVGIndex, log);
    }

    double t1 = H5Helper::getTime();
    Helper::printDebugTime("changing chunks of the whole dataset", t0, t1);

    getOutputFile()->closeDataset(dstDataset, log);
}
