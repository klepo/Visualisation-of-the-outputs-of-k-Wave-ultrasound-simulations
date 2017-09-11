/**
 * @file        reshape.cpp
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        8  September 2016 (created) \n
 *              28 August    2017 (updated)
 *
 * @brief       The implementation file containing Reshape class definition.
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

#include "reshape.h"

/**
 * @brief Creates Reshape object
 * @param[in] outputFile Output file
 * @param[in] dtsForPcs Datasets for porcessing
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
            HDF5Helper::Dataset *sensorMaskIndexDataset = getDtsForPcs()->getSensorMaskIndexDataset();

            // Find min and max position from sensor mask
            HDF5Helper::Vector3D min;
            HDF5Helper::Vector3D max;
            findMinAndMaxPositionFromSensorMask(sensorMaskIndexDataset, min, max);

            HDF5Helper::Vector3D dims(max.z() - min.z() + 1, max.y() - min.y() + 1, max.x() - min.x() + 1);

            // Compute chunk size according to min/max position
            HDF5Helper::Vector4D chunkDims;
            chunkDims.w(1);
            chunkDims.z(std::min(getSettings()->getMaxChunkSize(), dims.z()));
            chunkDims.y(std::min(getSettings()->getMaxChunkSize(), dims.y()));
            chunkDims.x(std::min(getSettings()->getMaxChunkSize(), dims.x()));
            std::cout << "   new chunk size:\t" << chunkDims << std::endl;

            HDF5Helper::MapOfDatasets map = getDtsForPcs()->getDatasets();
            for (HDF5Helper::MapOfDatasetsIt it = map.begin(); it != map.end(); ++it) {
                HDF5Helper::Dataset *dataset = it->second;
                HDF5Helper::DatasetType datasetType = dataset->getType(getDtsForPcs()->getSensorMaskSize());

                if (checkDatasetType(datasetType, types)) {
                    hsize_t steps = dataset->getDims()[1];

                    // Compute dataset size
                    HDF5Helper::Vector4D datasetDims(steps, dims);
                    HDF5Helper::Vector4D stepSize(1, dims);
                    std::cout << "   new dataset size:\t" << datasetDims << std::endl;

                    // Helper variables
                    HDF5Helper::Dataset *dstDataset = 0;
                    hsize_t *sensorMaskData = 0;
                    float *datasetData = 0;
                    hsize_t frame = 0;
                    hsize_t index = 0;
                    HDF5Helper::Vector3D dstPos;
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

                    // Check whether is possible to write whole dataset
                    if (datasetDims.z() * datasetDims.y() * datasetDims.x() <= dataset->getNumberOfElmsToLoad()) {
                        useTmpFlag = true;
                        tmpData = new float[datasetDims.z() * datasetDims.y() * datasetDims.x()];
                        // Read sensorMaskIndexDataset only once
                        sensorMaskIndexDataset->readBlock(0, offset, count, sensorMaskData);
                    }

                    // TODO check getOutputFile() != dataset->getFile()
                    if (datasetType == HDF5Helper::DatasetType::TIME_STEPS_MASK) {
                        // Create new group for dataset
                        getOutputFile()->createGroup(dataset->getName(), true);
                        HDF5Helper::Group *group = getOutputFile()->openGroup(dataset->getName());
                        // Create dataset in group
                        group->createDatasetF("0", datasetDims, chunkDims, true);
                        dstDataset = group->openDataset("0");
                    } else {
                        // 3D dataset
                        getOutputFile()->createDatasetF(dataset->getName(), HDF5Helper::Vector3D(datasetDims), HDF5Helper::Vector3D(chunkDims), true);
                        dstDataset = getOutputFile()->openDataset(dataset->getName());
                    }

                    dstDataset->setAttribute(HDF5Helper::POSITION_Z_ATTR, min.z());
                    dstDataset->setAttribute(HDF5Helper::POSITION_Y_ATTR, min.y());
                    dstDataset->setAttribute(HDF5Helper::POSITION_X_ATTR, min.x());

                    HDF5Helper::Vector3D nDims = getDtsForPcs()->getNDims();

                    // Block reading
                    for (hsize_t i = 0; i < dataset->getNumberOfBlocks(); i++) {
                        // Load data
                        if (!useTmpFlag) {
                            sensorMaskIndexDataset->readBlock(i % sensorMaskIndexDataset->getNumberOfBlocks(), offset, count, sensorMaskData);
                        }

                        float minVF;
                        float maxVF;
                        hsize_t minVFIndex;
                        hsize_t maxVFIndex;
                        dataset->readBlock(i, offset, count, datasetData, minVF, maxVF, minVFIndex, maxVFIndex);
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
                        for (hsize_t z = 0; z < count.z(); z++) {
                            for (hsize_t y = 0; y < count.y(); y++) {
                                for (hsize_t x = 0; x < count.x(); x++) {
                                    hsize_t srcIndex = x + y * count.x() + z * count.x() * count.y();
                                    index = sensorMaskData[srcIndex] - 1;
                                    HDF5Helper::convertlinearToMultiDim(index, dstPos, nDims);
                                    float data = datasetData[srcIndex];
                                    if (useTmpFlag) {
                                        tmpData[(dstPos.z() - min.z()) * (datasetDims.y() * datasetDims.x()) + (dstPos.y() - min.y()) * datasetDims.x() + (dstPos.x() - min.x())] = data;
                                    } else {
                                        if (datasetType == HDF5Helper::DatasetType::TIME_STEPS_MASK) {
                                            dstDataset->writeDataset(HDF5Helper::Vector4D(frame, dstPos.z() - min.z(), dstPos.y() - min.y(), dstPos.x() - min.x()), HDF5Helper::Vector4D(1, 1, 1, 1), &data, false);
                                        } else {
                                            dstDataset->writeDataset(HDF5Helper::Vector3D(dstPos.z() - min.z(), dstPos.y() - min.y(), dstPos.x() - min.x()), HDF5Helper::Vector3D(1, 1, 1), &data, false);
                                        }
                                    }
                                }
                            }
                        }

                        double t1 = HDF5Helper::getTime();

                        if (!useTmpFlag) {
                            std::cout << dstDataset->getName() << " write time:  \t" << (t1 - t0) << " ms; \t" << std::endl;
                        }
                        if (!useTmpFlag) {
                            delete[] sensorMaskData;
                        }
                        delete[] datasetData;

                        // Next time step for group of datasets (step)?
                        if (i % sensorMaskIndexDataset->getNumberOfBlocks() + 1 == sensorMaskIndexDataset->getNumberOfBlocks()) {
                            if (useTmpFlag) {
                                if (datasetType == HDF5Helper::DatasetType::TIME_STEPS_MASK) {
                                    dstDataset->writeDataset(HDF5Helper::Vector4D(frame, 0, 0, 0), stepSize, tmpData, true);
                                } else {
                                    dstDataset->writeDataset(HDF5Helper::Vector3D(0, 0, 0), HDF5Helper::Vector3D(stepSize), tmpData, true);
                                }
                            }
                            if (MAX_NUMBER_OF_FRAMES > 0) // TODO
                                if (frame + 1 == MAX_NUMBER_OF_FRAMES)
                                    break;
                        }
                    }

                    dstDataset->setAttribute(HDF5Helper::MIN_ATTR, minVFG);
                    dstDataset->setAttribute(HDF5Helper::MAX_ATTR, maxVFG);
                    dstDataset->setAttribute(HDF5Helper::MIN_INDEX_ATTR, minVFGIndex);
                    dstDataset->setAttribute(HDF5Helper::MAX_INDEX_ATTR, maxVFGIndex);

                    if (useTmpFlag) {
                        delete[] tmpData;
                        delete[] sensorMaskData;
                    }
                }
            }
        } else if (!getDtsForPcs()->getDatasets(HDF5Helper::DatasetType::CUBOID).empty() && getDtsForPcs()->getSensorMaskCornersDataset()) {
            // For cuboid type datasets
            // Prepare something
            HDF5Helper::Dataset *sensorMaskCornersDataset = getDtsForPcs()->getSensorMaskCornersDataset();
            hsize_t *data;
            sensorMaskCornersDataset->readDataset(data);

            HDF5Helper::MapOfDatasets map = getDtsForPcs()->getDatasets(HDF5Helper::DatasetType::CUBOID);
            for (HDF5Helper::MapOfDatasetsIt it = map.begin(); it != map.end(); ++it) {
                HDF5Helper::Dataset *dataset = it->second;

                // Copy to new file
                if (getOutputFile() != dataset->getFile()) {
                    changeChunksOfDataset(dataset);
                    dataset = getOutputFile()->openDataset(dataset->getName());
                }

                // Name of the dataset to index
                hsize_t i = hsize_t(std::stoi(dataset->getName()));

                // Set position attributes
                dataset->findAndSetGlobalMinAndMaxValue();
                dataset->setAttribute(HDF5Helper::POSITION_Z_ATTR, data[i * 6 + 0] - 1);
                dataset->setAttribute(HDF5Helper::POSITION_Y_ATTR, data[i * 6 + 1] - 1);
                dataset->setAttribute(HDF5Helper::POSITION_X_ATTR, data[i * 6 + 2] - 1);
            }

            delete[] data;
        } else {
            std::cout << "No datasets for reshaping in simulation output file" << std::endl;
        }
    } catch(std::exception &e) {
        std::cerr << e.what() << std::endl;
        std::exit(EXIT_FAILURE);
    }
}

/**
 * @brief Finds min and max position from sensor mask
 * @param[in] sensorMaskIndexDataset Sensor mask index dataset
 * @param[out] min Min position
 * @param[out] max Max position
 */
void Reshape::findMinAndMaxPositionFromSensorMask(HDF5Helper::Dataset *sensorMaskIndexDataset, HDF5Helper::Vector3D &min, HDF5Helper::Vector3D &max)
{
    // Find min and max position from linear saved values
    hsize_t index = 0;
    hsize_t *data;
    HDF5Helper::Vector3D offset;
    HDF5Helper::Vector3D count;
    HDF5Helper::Vector3D dstPos;
    HDF5Helper::Vector4D nDims = sensorMaskIndexDataset->getFile()->getNdims();

    min = nDims;
    max = HDF5Helper::Vector3D(0, 0, 0);

    for (hsize_t i = 0; i < sensorMaskIndexDataset->getNumberOfBlocks(); i++) {
        sensorMaskIndexDataset->readBlock(i, offset, count, data);
        for (hsize_t z = 0; z < count.z(); z++) {
            for (hsize_t y = 0; y < count.y(); y++) {
                for (hsize_t x = 0; x < count.x(); x++) {
                    // [x + y * count.x() + z * count.x() * count.y()] is indexed from 0, but value of index is from 1
                    index = data[x + y * count.x() + z * count.x() * count.y()] - 1;
                    HDF5Helper::convertlinearToMultiDim(index, dstPos, HDF5Helper::Vector3D(nDims));

                    if (dstPos.x() < min.x()) min.x(dstPos.x());
                    if (dstPos.y() < min.y()) min.y(dstPos.y());
                    if (dstPos.z() < min.z()) min.z(dstPos.z());
                    if (dstPos.x() > max.x()) max.x(dstPos.x());
                    if (dstPos.y() > max.y()) max.y(dstPos.y());
                    if (dstPos.z() > max.z()) max.z(dstPos.z());
                }
            }
        }
        delete[] data; // !!
    }

    std::cout << "   min sensor mask point:\t" << min << std::endl;
    std::cout << "   max sensor mask point:\t" << max << std::endl;
}
