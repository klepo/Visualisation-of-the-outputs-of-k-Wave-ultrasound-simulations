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
Reshape::Reshape(H5Helper::File *outputFile, DtsForPcs *dtsForPcs, Settings *settings)
    : ChangeChunks(outputFile, dtsForPcs, settings)
{

}

/**
 * @brief Executes processing
 */
void Reshape::execute()
{
    std::vector<H5Helper::DatasetType> types = {
        H5Helper::DatasetType::TIME_STEPS_INDEX,
        H5Helper::DatasetType::TIME_STEPS_C_INDEX,
        H5Helper::DatasetType::TIME_STEPS_D_INDEX,
        H5Helper::DatasetType::TIME_STEPS_S_INDEX,
        H5Helper::DatasetType::BASIC_INDEX
    };

    try {
        if ((!getDtsForPcs()->getDatasets(H5Helper::DatasetType::TIME_STEPS_INDEX).empty()
             || !getDtsForPcs()->getDatasets(H5Helper::DatasetType::TIME_STEPS_C_INDEX).empty()
             || !getDtsForPcs()->getDatasets(H5Helper::DatasetType::TIME_STEPS_D_INDEX).empty()
             || !getDtsForPcs()->getDatasets(H5Helper::DatasetType::TIME_STEPS_S_INDEX).empty()
             || !getDtsForPcs()->getDatasets(H5Helper::DatasetType::BASIC_INDEX).empty())
            && getDtsForPcs()->getSensorMaskIndexDataset()) {
            // For mask type datasets
            // Prepare something
            // Find min and max position from sensor mask
            H5Helper::Vector3D min;
            H5Helper::Vector3D max;
            findMinAndMaxPositionFromSensorMask(getDtsForPcs()->getSensorMaskIndexDataset(), min, max, getSettings()->getFlagLog());

            H5Helper::Vector3D dims(max.z() - min.z() + 1, max.y() - min.y() + 1, max.x() - min.x() + 1);

            // Compute chunk dims according to min/max position
            H5Helper::Vector4D chunkDims;
            chunkDims.w(1);
            chunkDims.z(std::min(getSettings()->getMaxChunkSizeZ(), dims.z()));
            chunkDims.y(std::min(getSettings()->getMaxChunkSizeY(), dims.y()));
            chunkDims.x(std::min(getSettings()->getMaxChunkSizeX(), dims.x()));

            H5Helper::MapOfDatasets map = getDtsForPcs()->getDatasets();
            for (H5Helper::MapOfDatasetsIt it = map.begin(); it != map.end(); ++it) {
                H5Helper::Dataset *dataset = it->second;
                H5Helper::DatasetType datasetType = dataset->getType(getDtsForPcs()->getSensorMaskSize());
                if (checkDatasetType(datasetType, types)) {
                    Helper::printDebugMsg("Reshaping of dataset " + dataset->getName());
                    reshapeMaskTypeDataset(dataset, min, dims, chunkDims, getSettings()->getFlagLog());
                    Helper::printDebugMsg("Reshaping of dataset " + dataset->getName() + " done");
                }
            }
        } else if (!getDtsForPcs()->getDatasets(H5Helper::DatasetType::CUBOID).empty() && getDtsForPcs()->getSensorMaskCornersDataset()) {
            // For cuboid type datasets
            // Prepare something
            hsize_t *sensorMaskCornersData = 0;
            getDtsForPcs()->getSensorMaskCornersDataset()->readDataset(sensorMaskCornersData);

            H5Helper::MapOfDatasets map = getDtsForPcs()->getDatasets(H5Helper::DatasetType::CUBOID);
            for (H5Helper::MapOfDatasetsIt it = map.begin(); it != map.end(); ++it) {
                H5Helper::Dataset *dataset = it->second;
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

void Reshape::reshapeCuboid(H5Helper::Dataset *dataset, hsize_t *sensorMaskCornersData, bool log)
{
    // Copy to new file
    if (getOutputFile() != dataset->getFile()) {
        changeChunksOfDataset(dataset, log);
        dataset = getOutputFile()->openDataset(dataset->getName(), log);
    }

    // Name of the dataset to index
    hsize_t i = hsize_t(std::stoi(dataset->getOnlyName()));

    // Set position attributes
    dataset->findAndSetGlobalMinAndMaxValue();
    dataset->setAttribute(H5Helper::POSITION_Z_ATTR, sensorMaskCornersData[(i - 1) * 6 + 2] - 1, log);
    dataset->setAttribute(H5Helper::POSITION_Y_ATTR, sensorMaskCornersData[(i - 1) * 6 + 1] - 1, log);
    dataset->setAttribute(H5Helper::POSITION_X_ATTR, sensorMaskCornersData[(i - 1) * 6 + 0] - 1, log);
}

void Reshape::reshapeMaskTypeDataset(H5Helper::Dataset *dataset, H5Helper::Vector3D globalPosTmp, H5Helper::Vector3D dimsTmp, H5Helper::Vector4D chunkDimsTmp, bool log)
{
    H5Helper::Dataset *sensorMaskIndexDataset = getDtsForPcs()->getSensorMaskIndexDataset();
    H5Helper::DatasetType datasetType = dataset->getType(getDtsForPcs()->getSensorMaskSize());

    H5Helper::Vector3D globalPos = globalPosTmp;
    H5Helper::Vector3D dims = dimsTmp;
    H5Helper::Vector4D chunkDims = chunkDimsTmp;
    hsize_t mos = dataset->hasAttribute(H5Helper::C_MOS_ATTR) ? dataset->readAttributeI(H5Helper::C_MOS_ATTR, getSettings()->getFlagLog()) : 1;
    hsize_t harmonics = dataset->hasAttribute(H5Helper::C_HARMONICS_ATTR) ? dataset->readAttributeI(H5Helper::C_HARMONICS_ATTR, getSettings()->getFlagLog()) : 1;
    hsize_t xStride = datasetType == H5Helper::DatasetType::TIME_STEPS_C_INDEX ? mos * harmonics * 2 : 1;
    //globalPos.x(globalPos.x() * xStride);
    dims.x(dims.x() * xStride);
    chunkDims.x(std::min(getSettings()->getMaxChunkSizeX(), chunkDims.x() * xStride));

    hsize_t steps = dataset->getDims()[1];

    chunkDims.w(std::min(getSettings()->getMaxChunkSizeW(), steps));
    Helper::printDebugTwoColumns2S("New chunk size", chunkDims);

    // Compute dataset size
    H5Helper::Vector4D datasetDims(steps, dims);
    H5Helper::Vector4D stepSize(1, dims);
    Helper::printDebugTwoColumns2S("New dataset size", datasetDims);

    // Helper variables
    H5Helper::Dataset *dstDataset = 0;
    hsize_t *sensorMaskData = 0;
    float *datasetData = 0;
    hsize_t step = 0;
    H5Helper::Vector3D offset; // Offset
    H5Helper::Vector3D count;  // Count

    // Min/max values vars
    float minVFG = std::numeric_limits<float>::max();
    float maxVFG = -std::numeric_limits<float>::min();
    hsize_t minVFGIndex = 0;
    hsize_t maxVFGIndex = 0;

    // Set same block size as sensorMaskIndexDataset
    dataset->setNumberOfElmsToLoad(sensorMaskIndexDataset->getRealNumberOfElmsToLoad() * xStride);

    // Flag and temp data for faster reading
    bool useTmpFlag = false;
    float *tmpData = 0;

    // TODO rozpoznat, zda je maska cuboid (maska má stejný počet bodů jako obálka, stoupající posloupnost všech indexů), pak se může pouze překopírovat.
    bool cuboidFlag = false;

    // Check whether is possible to write whole one step (3D dataset)
    if (datasetDims.z() * datasetDims.y() * datasetDims.x() <= dataset->getNumberOfElmsToLoad()) {
        useTmpFlag = true;
        tmpData = new float[datasetDims.z() * datasetDims.y() * datasetDims.x()];
        // Read sensorMaskIndexDataset only once
        sensorMaskIndexDataset->readBlock(0, offset, count, sensorMaskData, log);

        hsize_t last = 0;
        cuboidFlag = true;
        Helper::printDebugMsgStart("Checking of \"cuboid\" sensor mask ... ");
        if (dimsTmp.getSize() == sensorMaskIndexDataset->getSize()) {
            for (hssize_t i = 0; i < hssize_t(sensorMaskIndexDataset->getSize()); i++) {
                if (sensorMaskData[i] <= last) {
                    cuboidFlag = false;
                    Helper::printDebugMsg("The shape is not cuboid");
                    break;
                }
                last = sensorMaskData[i];
            }
        }
        if (cuboidFlag)
            Helper::printDebugMsg("OK");

    } else {
        // TODO
    }

    // TODO check getOutputFile() != dataset->getFile()

    if (datasetType == H5Helper::DatasetType::TIME_STEPS_INDEX) {
        // Create new group for dataset
        getOutputFile()->createGroup(dataset->getName(), false, log);
        H5Helper::Group *group = getOutputFile()->openGroup(dataset->getName(), log);
        // Create dataset in group
        group->createDatasetF("1", datasetDims, chunkDims, true, log);
        dstDataset = group->openDataset("1", log);
    } else if (datasetType == H5Helper::DatasetType::TIME_STEPS_C_INDEX
               || datasetType == H5Helper::DatasetType::TIME_STEPS_D_INDEX
               || datasetType == H5Helper::DatasetType::TIME_STEPS_S_INDEX) {
        // Create new group for dataset
        std::string name = dataset->readAttributeS(H5Helper::SRC_DATASET_NAME_ATTR);
        std::string type = dataset->getName().erase(0, name.length());
        getOutputFile()->createGroup(name, false, log);
        H5Helper::Group *group = getOutputFile()->openGroup(name, log);
        // Create dataset in group
        group->createDatasetF("1" + type, datasetDims, chunkDims, true, log);
        dstDataset = group->openDataset("1" + type, log);
    } else if (datasetType == H5Helper::DatasetType::BASIC_INDEX) {
        // 3D dataset
        getOutputFile()->createDatasetF(dataset->getName(), H5Helper::Vector3D(datasetDims), H5Helper::Vector3D(chunkDims), true, log);
        dstDataset = getOutputFile()->openDataset(dataset->getName(), log);
    } else { // Something wrong.
        Helper::printErrorMsg("Something wrong with dataset type");
        return;
    }

    H5Helper::Vector3D nDims = getDtsForPcs()->getNDims();

    // Block reading
    for (hsize_t i = 0; i < dataset->getNumberOfBlocks(); i++) {
        // Load data
        if (!useTmpFlag) {
            sensorMaskIndexDataset->readBlock(i % sensorMaskIndexDataset->getNumberOfBlocks(), offset, count, sensorMaskData, log);
        }

        float minVF = std::numeric_limits<float>::max();
        float maxVF = std::numeric_limits<float>::min();
        hsize_t minVFIndex = 0;
        hsize_t maxVFIndex = 0;
        dataset->readBlock(i, offset, count, datasetData, minVF, maxVF, minVFIndex, maxVFIndex, log);
        step = offset.y();

        if (datasetType != H5Helper::DatasetType::TIME_STEPS_C_INDEX) {
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
        }

        double t0 = H5Helper::getTime();

        // For the entire block write "voxels"
        if (!useTmpFlag) {
            for (hssize_t i = 0; i < hssize_t(count.x() / xStride); i++) {
                    H5Helper::Vector3D dstPos;
                    H5Helper::convertlinearToMultiDim(sensorMaskData[i] - 1, dstPos, nDims);
                    if (datasetType == H5Helper::DatasetType::TIME_STEPS_INDEX
                        || datasetType == H5Helper::DatasetType::TIME_STEPS_C_INDEX
                        || datasetType == H5Helper::DatasetType::TIME_STEPS_D_INDEX
                        || datasetType == H5Helper::DatasetType::TIME_STEPS_S_INDEX) {
                        dstDataset->writeDataset(H5Helper::Vector4D(step, dstPos.z() - globalPos.z(), dstPos.y() - globalPos.y(), (dstPos.x() - globalPos.x()) * xStride), H5Helper::Vector4D(1, 1, 1, xStride), &datasetData[i * xStride], false);
                    } else if (datasetType == H5Helper::DatasetType::BASIC_INDEX) {
                        dstDataset->writeDataset(H5Helper::Vector3D(dstPos.z() - globalPos.z(), dstPos.y() - globalPos.y(), (dstPos.x() - globalPos.x())), H5Helper::Vector3D(1, 1, 1), &datasetData[i], false);
                    } else { // Something wrong.
                        Helper::printErrorMsg("Something wrong with dataset type");
                        return;
                    }
            }
        } else {
            if (cuboidFlag) {
                tmpData = datasetData;
            } else {
                #pragma omp parallel for
                for (hssize_t i = 0; i < hssize_t(count.x() / xStride); i++) {
                    H5Helper::Vector3D dstPos;
                    H5Helper::convertlinearToMultiDim(sensorMaskData[i] - 1, dstPos, nDims);
                    for (hssize_t j = 0; j < hssize_t(xStride); j++) {
                        tmpData[(dstPos.z() - globalPos.z()) * datasetDims.y() * datasetDims.x() + (dstPos.y() - globalPos.y()) * datasetDims.x() + (dstPos.x() - globalPos.x()) * xStride + j] = datasetData[i * xStride + j];
                    }
                }
            }
        }

        double t1 = H5Helper::getTime();

        // Next time step?
        if (i % sensorMaskIndexDataset->getNumberOfBlocks() + 1 == sensorMaskIndexDataset->getNumberOfBlocks()) {
            if (useTmpFlag) {
                if (datasetType == H5Helper::DatasetType::TIME_STEPS_INDEX
                    || datasetType == H5Helper::DatasetType::TIME_STEPS_C_INDEX
                    || datasetType == H5Helper::DatasetType::TIME_STEPS_D_INDEX
                    || datasetType == H5Helper::DatasetType::TIME_STEPS_S_INDEX) {
                    dstDataset->writeDataset(H5Helper::Vector4D(step, 0, 0, 0), stepSize, tmpData, log);
                } else {
                    dstDataset->writeDataset(H5Helper::Vector3D(0, 0, 0), H5Helper::Vector3D(stepSize), tmpData, log);
                }
            }
            if (MAX_NUMBER_OF_FRAMES > 0) // TODO
                if (step + 1 == MAX_NUMBER_OF_FRAMES)
                    break;
        }

        if (!useTmpFlag) {
            Helper::printDebugTime("point by point data writing", t0, t1);
            delete[] sensorMaskData;
        }
        if (!cuboidFlag) {
            delete[] datasetData;
        }
    }

    if (datasetType != H5Helper::DatasetType::TIME_STEPS_INDEX) {
        // Copy attributes
        copyAttributes(dataset, dstDataset);
        dstDataset->setAttribute(H5Helper::POSITION_Z_ATTR, globalPos.z(), log);
        dstDataset->setAttribute(H5Helper::POSITION_Y_ATTR, globalPos.y(), log);
        dstDataset->setAttribute(H5Helper::POSITION_X_ATTR, globalPos.x(), log);
    } else {
        dstDataset->setAttribute(H5Helper::POSITION_Z_ATTR, globalPos.z(), log);
        dstDataset->setAttribute(H5Helper::POSITION_Y_ATTR, globalPos.y(), log);
        dstDataset->setAttribute(H5Helper::POSITION_X_ATTR, globalPos.x(), log);
        dstDataset->setAttribute(H5Helper::MIN_ATTR, minVFG, log);
        dstDataset->setAttribute(H5Helper::MAX_ATTR, maxVFG, log);
        dstDataset->setAttribute(H5Helper::MIN_INDEX_ATTR, minVFGIndex, log);
        dstDataset->setAttribute(H5Helper::MAX_INDEX_ATTR, maxVFGIndex, log);    }

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
void Reshape::findMinAndMaxPositionFromSensorMask(H5Helper::Dataset *sensorMaskIndexDataset, H5Helper::Vector3D &min, H5Helper::Vector3D &max, bool log)
{
    // Find min and max position from linear saved values
    hsize_t *data = new hsize_t[sensorMaskIndexDataset->getGeneralBlockDims().getSize()];
    H5Helper::Vector3D offset;
    H5Helper::Vector3D count;
    H5Helper::Vector4D nDims = sensorMaskIndexDataset->getFile()->getNdims();

    min = nDims;
    max = H5Helper::Vector3D(0, 0, 0);

    for (hsize_t i = 0; i < sensorMaskIndexDataset->getNumberOfBlocks(); i++) {
        sensorMaskIndexDataset->readBlock(i, offset, count, data, log);
        for (hssize_t idx = 0; idx < hssize_t(count.x()); idx++) {
            H5Helper::Vector3D dstPos;
            // i is indexed from 0, but value of index is from 1
            H5Helper::convertlinearToMultiDim(data[idx] - 1, dstPos, H5Helper::Vector3D(nDims));
            if (dstPos.x() < min.x()) min.x(dstPos.x());
            if (dstPos.y() < min.y()) min.y(dstPos.y());
            if (dstPos.z() < min.z()) min.z(dstPos.z());
            if (dstPos.x() > max.x()) max.x(dstPos.x());
            if (dstPos.y() > max.y()) max.y(dstPos.y());
            if (dstPos.z() > max.z()) max.z(dstPos.z());
        }
    }
    delete[] data; // !!

    Helper::printDebugTwoColumns2S("Min sensor mask point", min);
    Helper::printDebugTwoColumns2S("Max sensor mask point", max);
}