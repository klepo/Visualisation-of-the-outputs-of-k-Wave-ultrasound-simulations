/**
 * @file        changechunks.cpp
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        8  September 2016 (created) \n
 *              28 August    2017 (updated)
 *
 * @brief       The implementation file containing ChangeChunks class definition.
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

#include "changechunks.h"

ChangeChunks::ChangeChunks(HDF5Helper::File *outputFile, DtsForPcs *dtsForPcs, Settings *settings)
    : Processing(outputFile, dtsForPcs, settings)
{

}

void ChangeChunks::execute()
{
    try {
        HDF5Helper::MapOfDatasets map = getDtsForPcs()->getDatasets();
        int count = 0;
        for (HDF5Helper::MapOfDatasetsIt it = map.begin(); it != map.end(); ++it) {
            HDF5Helper::Dataset *dataset = it->second;
            HDF5Helper::DatasetType datasetType = dataset->getType();
            if (datasetType == HDF5Helper::DatasetType::BASIC_3D
                    || datasetType == HDF5Helper::DatasetType::DWNSMPL_3D
                    || datasetType == HDF5Helper::DatasetType::CUBOID
                    || datasetType == HDF5Helper::DatasetType::CUBOID_ATTR
                    ) {
                std::cout << "Change chunks of dataset " << dataset->getName() << std::endl;
                changeChunksOfDataset(dataset);
                count++;
                std::cout << "Change chunks of dataset " << dataset->getName() << " done" << std::endl << std::endl;
            }
        }
        if (count == 0) {
            std::cout << "No datasets for changing chunks in simulation output file" << std::endl;
        }
    } catch(std::exception &e) {
        std::cerr << e.what() << std::endl;
        std::exit(EXIT_FAILURE);
    }
}

void ChangeChunks::changeChunksOfDataset(HDF5Helper::Dataset *srcDataset)
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

    // Create dst dataset
    getOutputFile()->createDatasetF(srcDataset->getName(), dims, chunkDims, true);
    HDF5Helper::Dataset *dstDataset = getOutputFile()->openDataset(srcDataset->getName());

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
        srcDataset->readBlock(i, offset, count, data, minV, maxV, minVIndex, maxVIndex);
        dstDataset->writeDataset(offset, count, data, true);
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
    dstDataset->setAttribute(HDF5Helper::MIN_ATTR, minVG);
    dstDataset->setAttribute(HDF5Helper::MAX_ATTR, maxVG);
    dstDataset->setAttribute(HDF5Helper::MIN_INDEX_ATTR, minVGIndex);
    dstDataset->setAttribute(HDF5Helper::MAX_INDEX_ATTR, maxVGIndex);

    double t1 = HDF5Helper::getTime();
    std::cout << "Time of changing chunks of the whole dataset: " << (t1 - t0) << " ms; \t" << std::endl;

    getOutputFile()->closeDataset(dstDataset);
}
