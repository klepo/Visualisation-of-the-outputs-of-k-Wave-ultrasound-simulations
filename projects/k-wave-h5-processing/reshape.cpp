/**
 * @file        reshape.cpp
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        8  September 2016 (created) \n
 *              11 September 2017 (updated)
 *
 * @brief       The implementation file containing Reshape class definition.
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

#include "reshape.h"

/**
 * @brief Creates Reshape object
 * @param[in] outputFile Output file
 * @param[in] dtsForPcs Datasets for processing
 * @param[in] settings Processing settings
 */
Reshape::Reshape(HDF5Helper::File *outputFile, DtsForPcs *dtsForPcs, Settings *settings)
    : ChangeChunks(outputFile, dtsForPcs, settings)
{

}

/**
 * @brief Executes processing
 */
void Reshape::execute()
{
    std::vector<HDF5Helper::DatasetType> types = {
        HDF5Helper::DatasetType::TIME_STEPS_MASK,
        HDF5Helper::DatasetType::BASIC_MASK
    };

    try {
        if ((!getDtsForPcs()->getDatasets(HDF5Helper::DatasetType::TIME_STEPS_MASK).empty() || !getDtsForPcs()->getDatasets(HDF5Helper::DatasetType::BASIC_MASK).empty()) && getDtsForPcs()->getSensorMaskIndexDataset()) {
            // For mask type datasets
            // Prepare something
            // Find min and max position from sensor mask
            HDF5Helper::Vector3D min;
            HDF5Helper::Vector3D max;
            findMinAndMaxPositionFromSensorMask(getDtsForPcs()->getSensorMaskIndexDataset(), min, max, getSettings()->getFlagLog());

            HDF5Helper::Vector3D dims(max.z() - min.z() + 1, max.y() - min.y() + 1, max.x() - min.x() + 1);

            // Compute chunk size according to min/max position
            HDF5Helper::Vector4D chunkDims;
            chunkDims.w(1);
            chunkDims.z(std::min(getSettings()->getMaxChunkSize(), dims.z()));
            chunkDims.y(std::min(getSettings()->getMaxChunkSize(), dims.y()));
            chunkDims.x(std::min(getSettings()->getMaxChunkSize(), dims.x()));
            //Helper::printDebugTwoColumns2S("new chunk size", chunkDims);

            HDF5Helper::MapOfDatasets map = getDtsForPcs()->getDatasets();
            for (HDF5Helper::MapOfDatasetsIt it = map.begin(); it != map.end(); ++it) {
                HDF5Helper::Dataset *dataset = it->second;
                HDF5Helper::DatasetType datasetType = dataset->getType(getDtsForPcs()->getSensorMaskSize());
                if (checkDatasetType(datasetType, types)) {
                    Helper::printDebugMsg("Reshaping of dataset " + dataset->getName());
                    reshapeMaskTypeDataset(dataset, min, dims, chunkDims, getSettings()->getFlagLog());
                    Helper::printDebugMsg("Reshaping of dataset " + dataset->getName() + " done");
                }
            }
        } else if (!getDtsForPcs()->getDatasets(HDF5Helper::DatasetType::CUBOID).empty() && getDtsForPcs()->getSensorMaskCornersDataset()) {
            // For cuboid type datasets
            // Prepare something
            hsize_t *sensorMaskCornersData;
            getDtsForPcs()->getSensorMaskCornersDataset()->readDataset(sensorMaskCornersData);

            HDF5Helper::MapOfDatasets map = getDtsForPcs()->getDatasets(HDF5Helper::DatasetType::CUBOID);
            for (HDF5Helper::MapOfDatasetsIt it = map.begin(); it != map.end(); ++it) {
                HDF5Helper::Dataset *dataset = it->second;
                Helper::printDebugMsg("Reshaping of dataset " + dataset->getName());
                reshapeCuboid(dataset, sensorMaskCornersData, getSettings()->getFlagLog());
                Helper::printDebugMsg("Reshaping of dataset " + dataset->getName() + " done");
            }
            delete[] sensorMaskCornersData;
        } else {
            Helper::printErrorMsg("No datasets for reshaping in simulation output file");
        }
    } catch(std::exception &e) {
        Helper::printErrorMsg(e.what());
        std::exit(EXIT_FAILURE);
    }
}

void Reshape::reshapeCuboid(HDF5Helper::Dataset *dataset, hsize_t *sensorMaskCornersData, bool log)
{
    // Copy to new file
    if (getOutputFile() != dataset->getFile()) {
        changeChunksOfDataset(dataset, log);
        dataset = getOutputFile()->openDataset(dataset->getName(), log);
    }

    // Name of the dataset to index
    hsize_t i = hsize_t(std::stoi(dataset->getName()));

    // Set position attributes
    dataset->findAndSetGlobalMinAndMaxValue();
    dataset->setAttribute(HDF5Helper::POSITION_Z_ATTR, sensorMaskCornersData[i * 6 + 0] - 1, log);
    dataset->setAttribute(HDF5Helper::POSITION_Y_ATTR, sensorMaskCornersData[i * 6 + 1] - 1, log);
    dataset->setAttribute(HDF5Helper::POSITION_X_ATTR, sensorMaskCornersData[i * 6 + 2] - 1, log);
}

void Reshape::reshapeMaskTypeDataset(HDF5Helper::Dataset *dataset, HDF5Helper::Vector3D globalPos, HDF5Helper::Vector3D dims, HDF5Helper::Vector4D chunkDims, bool log)
{
    HDF5Helper::Dataset *sensorMaskIndexDataset = getDtsForPcs()->getSensorMaskIndexDataset();
    HDF5Helper::DatasetType datasetType = dataset->getType(getDtsForPcs()->getSensorMaskSize());

    hsize_t steps = dataset->getDims()[1];

    // Compute dataset size
    HDF5Helper::Vector4D datasetDims(steps, dims);
    HDF5Helper::Vector4D stepSize(1, dims);
    if (log) {
        Helper::printDebugTwoColumns2S("new dataset size", datasetDims);
    }

    // Helper variables
    HDF5Helper::Dataset *dstDataset = 0;
    hsize_t *sensorMaskData = 0;
    float *datasetData = 0;
    hsize_t frame = 0;
    HDF5Helper::Vector3D offset; // Offset
    HDF5Helper::Vector3D count;  // Count

    // Min/max values vars
    float minVFG = std::numeric_limits<float>::infinity();
    float maxVFG = -std::numeric_limits<float>::infinity();
    hsize_t minVFGIndex = 0;
    hsize_t maxVFGIndex = 0;

    // Set same block size as sensorMaskIndexDataset
    dataset->setNumberOfElmsToLoad(sensorMaskIndexDataset->getRealNumberOfElmsToLoad());

    // Flag and temp data for faster reading
    bool useTmpFlag = false;
    float *tmpData = 0;

    // Check whether is possible to write whole one step (3D dataset)
    if (datasetDims.z() * datasetDims.y() * datasetDims.x() <= dataset->getNumberOfElmsToLoad()) {
        useTmpFlag = true;
        tmpData = new float[datasetDims.z() * datasetDims.y() * datasetDims.x()];
        // Read sensorMaskIndexDataset only once
        sensorMaskIndexDataset->readBlock(0, offset, count, sensorMaskData, log);
    }

    // TODO check getOutputFile() != dataset->getFile()

    if (datasetType == HDF5Helper::DatasetType::TIME_STEPS_MASK) {
        // Create new group for dataset
        getOutputFile()->createGroup(dataset->getName(), false, log);
        HDF5Helper::Group *group = getOutputFile()->openGroup(dataset->getName(), log);
        // Create dataset in group
        group->createDatasetF("0", datasetDims, chunkDims, true, log);
        dstDataset = group->openDataset("0", log);
    } else if (datasetType == HDF5Helper::DatasetType::BASIC_MASK) {
        // 3D dataset
        getOutputFile()->createDatasetF(dataset->getName(), HDF5Helper::Vector3D(datasetDims), HDF5Helper::Vector3D(chunkDims), true, log);
        dstDataset = getOutputFile()->openDataset(dataset->getName(), log);
    } else { // Something wrong.
        Helper::printErrorMsg("Something wrong with dataset type");
        return;
    }

    HDF5Helper::Vector3D nDims = getDtsForPcs()->getNDims();

    // Block reading
    for (hsize_t i = 0; i < dataset->getNumberOfBlocks(); i++) {
        // Load data
        if (!useTmpFlag) {
            sensorMaskIndexDataset->readBlock(i % sensorMaskIndexDataset->getNumberOfBlocks(), offset, count, sensorMaskData, log);
        }

        float minVF;
        float maxVF;
        hsize_t minVFIndex;
        hsize_t maxVFIndex;
        dataset->readBlock(i, offset, count, datasetData, minVF, maxVF, minVFIndex, maxVFIndex, log);
        frame = offset.y();

        hsize_t linearOffset;
        convertMultiDimToLinear(offset, linearOffset, dataset->getDims());

        if (minVFG > minVF) {
            minVFG = minVF;
            minVFGIndex = linearOffset + minVFIndex;
        }
        if (maxVFG < maxVF) {
            maxVFG = maxVF;
            maxVFGIndex = linearOffset + maxVFIndex;
        }

        double t0 = HDF5Helper::getTime();

        // For the entire block write "voxels"
        if (!useTmpFlag) {
            for (hssize_t i = 0; i < hssize_t(count.x()); i++) {
                    HDF5Helper::Vector3D dstPos;
                    HDF5Helper::convertlinearToMultiDim(sensorMaskData[i] - 1, dstPos, nDims);
                    if (datasetType == HDF5Helper::DatasetType::TIME_STEPS_MASK) {
                        dstDataset->writeDataset(HDF5Helper::Vector4D(frame, dstPos.z() - globalPos.z(), dstPos.y() - globalPos.y(), dstPos.x() - globalPos.x()), HDF5Helper::Vector4D(1, 1, 1, 1), &datasetData[i], false);
                    } else if (datasetType == HDF5Helper::DatasetType::BASIC_MASK) {
                        dstDataset->writeDataset(HDF5Helper::Vector3D(dstPos.z() - globalPos.z(), dstPos.y() - globalPos.y(), dstPos.x() - globalPos.x()), HDF5Helper::Vector3D(1, 1, 1), &datasetData[i], false);
                    } else { // Something wrong.
                        Helper::printErrorMsg("Something wrong with dataset type");
                        return;
                    }
            }
        } else {
            #pragma omp parallel for
            for (hssize_t i = 0; i < hssize_t(count.x()); i++) {
                HDF5Helper::Vector3D dstPos;
                HDF5Helper::convertlinearToMultiDim(sensorMaskData[i] - 1, dstPos, nDims);
                tmpData[(dstPos.z() - globalPos.z()) * (datasetDims.y() * datasetDims.x()) + (dstPos.y() - globalPos.y()) * datasetDims.x() + (dstPos.x() - globalPos.x())] = datasetData[i];
            }
        }

        double t1 = HDF5Helper::getTime();

        if (!useTmpFlag) {
            Helper::printDebugTime("point by point data writing", t0, t1);
            delete[] sensorMaskData;
        }
        delete[] datasetData;

        // Next time step?
        if (i % sensorMaskIndexDataset->getNumberOfBlocks() + 1 == sensorMaskIndexDataset->getNumberOfBlocks()) {
            if (useTmpFlag) {
                if (datasetType == HDF5Helper::DatasetType::TIME_STEPS_MASK) {
                    dstDataset->writeDataset(HDF5Helper::Vector4D(frame, 0, 0, 0), stepSize, tmpData, log);
                } else {
                    dstDataset->writeDataset(HDF5Helper::Vector3D(0, 0, 0), HDF5Helper::Vector3D(stepSize), tmpData, log);
                }
            }
            if (MAX_NUMBER_OF_FRAMES > 0) // TODO
                if (frame + 1 == MAX_NUMBER_OF_FRAMES)
                    break;
        }
    }

    dstDataset->setAttribute(HDF5Helper::POSITION_Z_ATTR, globalPos.z(), log);
    dstDataset->setAttribute(HDF5Helper::POSITION_Y_ATTR, globalPos.y(), log);
    dstDataset->setAttribute(HDF5Helper::POSITION_X_ATTR, globalPos.x(), log);
    dstDataset->setAttribute(HDF5Helper::MIN_ATTR, minVFG, log);
    dstDataset->setAttribute(HDF5Helper::MAX_ATTR, maxVFG, log);
    dstDataset->setAttribute(HDF5Helper::MIN_INDEX_ATTR, minVFGIndex, log);
    dstDataset->setAttribute(HDF5Helper::MAX_INDEX_ATTR, maxVFGIndex, log);

    if (useTmpFlag) {
        delete[] tmpData;
        delete[] sensorMaskData;
    }
}

/**
 * @brief Finds min and max position from sensor mask
 * @param[in] sensorMaskIndexDataset Sensor mask index dataset
 * @param[out] min Min position
 * @param[out] max Max position
 */
void Reshape::findMinAndMaxPositionFromSensorMask(HDF5Helper::Dataset *sensorMaskIndexDataset, HDF5Helper::Vector3D &min, HDF5Helper::Vector3D &max, bool log)
{
    // Find min and max position from linear saved values
    hsize_t *data;
    HDF5Helper::Vector3D offset;
    HDF5Helper::Vector3D count;
    HDF5Helper::Vector4D nDims = sensorMaskIndexDataset->getFile()->getNdims();

    min = nDims;
    max = HDF5Helper::Vector3D(0, 0, 0);

    for (hsize_t i = 0; i < sensorMaskIndexDataset->getNumberOfBlocks(); i++) {
        sensorMaskIndexDataset->readBlock(i, offset, count, data, log);
        for (hssize_t idx = 0; idx < hssize_t(count.x()); idx++) {
            HDF5Helper::Vector3D dstPos;
            // i is indexed from 0, but value of index is from 1
            HDF5Helper::convertlinearToMultiDim(data[idx] - 1, dstPos, HDF5Helper::Vector3D(nDims));
            if (dstPos.x() < min.x()) min.x(dstPos.x());
            if (dstPos.y() < min.y()) min.y(dstPos.y());
            if (dstPos.z() < min.z()) min.z(dstPos.z());
            if (dstPos.x() > max.x()) max.x(dstPos.x());
            if (dstPos.y() > max.y()) max.y(dstPos.y());
            if (dstPos.z() > max.z()) max.z(dstPos.z());
        }
        delete[] data; // !!
    }

    if (log) {
        Helper::printDebugTwoColumns2S("min sensor mask point", min);
        Helper::printDebugTwoColumns2S("max sensor mask point", max);
    }
}
