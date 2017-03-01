/**
 * @file        processing.cpp
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        8  September 2016 (created) \n
 *              27 February  2017 (updated)
 *
 * @brief       The implementation file containing processing functions.
 *
 * @license     This application is for preprocessing the HDF5 data created by the k-Wave toolbox - http://www.k-wave.org.
 *              k-Wave h5 processing is free software.
 *
 * @copyright   Copyright © 2017, Petr Kleparnik, VUT FIT Brno. All Rights Reserved.
 *
 */

#include "processing.h"

Processing::Processing(HDF5Helper::File *hDF5OutputFile, DtsForPcs *dtsForPcs, Settings *settings)
{
    this->hDF5OutputFile = hDF5OutputFile;
    this->dtsForPcs = dtsForPcs;
    this->settings = settings;
}

void Processing::reshape()
{
    try {
        // For mask type datasets
        if (!dtsForPcs->getDatasets(HDF5Helper::HDF5DatasetType::BASIC_MASK).empty() && dtsForPcs->getSensorMaskIndexDataset()) {
            // Prepare some things
            HDF5Helper::HDF5Dataset *sensorMaskIndexDataset = dtsForPcs->getSensorMaskIndexDataset();
            HDF5Helper::HDF5Vector3D nDims = dtsForPcs->getNDims();

            // Find min and max position from sensor mask
            HDF5Helper::HDF5Vector3D min = nDims;
            HDF5Helper::HDF5Vector3D max = HDF5Helper::HDF5Vector3D(0, 0, 0);
            findMinAndMaxPositionFromSensorMask(min, max);

            // Compute chunk size according to min/max position
            HDF5Helper::HDF5Vector4D chunkDims;
            chunkDims.w(1);
            chunkDims.z(std::min(settings->getMaxChunkSize(), max.z() - min.z() + 1));
            chunkDims.y(std::min(settings->getMaxChunkSize(), max.y() - min.y() + 1));
            chunkDims.x(std::min(settings->getMaxChunkSize(), max.x() - min.x() + 1));
            std::cout << "   new chunk size:\t" << chunkDims.w() << " x " << chunkDims.z() << " x " << chunkDims.y() << " x " << chunkDims.x() << std::endl;

            HDF5Helper::MapOfDatasets map = dtsForPcs->getDatasets(HDF5Helper::HDF5DatasetType::BASIC_MASK);
            for (HDF5Helper::MapOfDatasetsIt it = map.begin(); it != map.end(); ++it) {
                HDF5Helper::HDF5Dataset *dataset = it->second;

                hsize_t steps = dataset->getDims()[1];

                // Compute dataset size
                HDF5Helper::HDF5Vector4D datasetDims(steps, max.z() - min.z() + 1, max.y() - min.y() + 1, max.x() - min.x() + 1);
                HDF5Helper::HDF5Vector4D stepSize(1, max.z() - min.z() + 1, max.y() - min.y() + 1, max.x() - min.x() + 1);
                std::cout << "   new dataset size:\t" << datasetDims.w() << " x " << datasetDims.z() << " x " << datasetDims.y() << " x " << datasetDims.x() << std::endl;

                // Helper variables
                HDF5Helper::HDF5Dataset *dstDataset = 0;
                hsize_t *sensorMaskData = 0;
                float *datasetData = 0;
                hsize_t frame = 0;
                hsize_t index = 0;
                HDF5Helper::HDF5Vector3D dstPos;
                HDF5Helper::HDF5Vector3D offset; // Offset
                HDF5Helper::HDF5Vector3D count; // Count

                // Min/max values vars
                float minVFG = std::numeric_limits<float>::infinity();
                float maxVFG = -std::numeric_limits<float>::infinity();

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

                // Create new group for dataset
                hDF5OutputFile->createGroup(dataset->getName(), true);
                HDF5Helper::HDF5Group *group = hDF5OutputFile->openGroup(dataset->getName());

                // Create dataset in group
                group->createDatasetF("0", datasetDims, chunkDims, true);
                dstDataset = group->openDataset("0");

                dstDataset->setAttribute(HDF5Helper::POSITION_Z_ATTR, min.z());
                dstDataset->setAttribute(HDF5Helper::POSITION_Y_ATTR, min.y());
                dstDataset->setAttribute(HDF5Helper::POSITION_X_ATTR, min.x());

                // Block reading
                for (hsize_t i = 0; i < dataset->getNumberOfBlocks(); i++) {
                    // Load data
                    if (!useTmpFlag)
                        sensorMaskIndexDataset->readBlock(i % sensorMaskIndexDataset->getNumberOfBlocks(), offset, count, sensorMaskData);

                    float minVF;
                    float maxVF;
                    dataset->readBlock(i, offset, count, datasetData, minVF, maxVF);
                    frame = offset.y();

                    if (minVFG > minVF) minVFG = minVF;
                    if (maxVFG < maxVF) maxVFG = maxVF;

                    double t0 = HDF5Helper::getTime();
                    // For the entire block write "voxels"
                    for (hsize_t z = 0; z < count.z(); z++) {
                        for (hsize_t y = 0; y < count.y(); y++) {
                            for (hsize_t x = 0; x < count.x(); x++) {
                                hsize_t srcIndex = x + y * count.x() + z * count.x() * count.y();
                                index = sensorMaskData[srcIndex] - 1;
                                HDF5Helper::convertlinearToMultiDim(index, dstPos, nDims);
                                float data = datasetData[srcIndex];
                                if (useTmpFlag)
                                    tmpData[(dstPos.z() - min.z()) * (datasetDims.y() * datasetDims.x()) + (dstPos.y() - min.y()) * datasetDims.x() + (dstPos.x() - min.x())] = data;
                                else
                                    dstDataset->writeDataset(HDF5Helper::HDF5Vector4D(frame, dstPos.z() - min.z(), dstPos.y() - min.y(), dstPos.x() - min.x()), HDF5Helper::HDF5Vector4D(1, 1, 1, 1), &data, false);
                            }
                        }
                    }

                    double t1 = HDF5Helper::getTime();

                    if (!useTmpFlag)
                        std::cout << dstDataset->getName() << " write time:  \t" << (t1 - t0) << " ms; \t" << std::endl;

                    if (!useTmpFlag)
                        delete[] sensorMaskData;

                    delete[] datasetData;

                    // Next time step for group of datasets (step)?
                    if (i % sensorMaskIndexDataset->getNumberOfBlocks() + 1 == sensorMaskIndexDataset->getNumberOfBlocks()) {
                        if (useTmpFlag)
                            dstDataset->writeDataset(HDF5Helper::HDF5Vector4D(frame, 0, 0, 0), stepSize, tmpData, true);

                        if (MAX_NUMBER_OF_FRAMES > 0) // TODO
                            if (frame + 1 == MAX_NUMBER_OF_FRAMES)
                                break;
                    }
                }

                dstDataset->setAttribute(HDF5Helper::MIN_ATTR, minVFG);
                dstDataset->setAttribute(HDF5Helper::MAX_ATTR, maxVFG);

                if (useTmpFlag) {
                    delete[] tmpData;
                    delete[] sensorMaskData;
                }
            }

        }
        // For cuboid type datasets
        else if (!dtsForPcs->getDatasets(HDF5Helper::HDF5DatasetType::CUBOID).empty() && dtsForPcs->getSensorMaskCornersDataset()) {
            // Prepare something
            HDF5Helper::HDF5Dataset *sensorMaskCornersDataset = dtsForPcs->getSensorMaskCornersDataset();
            hsize_t *data;
            sensorMaskCornersDataset->readDataset(data);

            HDF5Helper::MapOfDatasets map = dtsForPcs->getDatasets(HDF5Helper::HDF5DatasetType::CUBOID);
            for (HDF5Helper::MapOfDatasetsIt it = map.begin(); it != map.end(); ++it) {
                HDF5Helper::HDF5Dataset *dataset = it->second;

                // Copy to new file
                if (hDF5OutputFile != dataset->getFile()) {
                    changeChunksOfDataset(dataset);
                    dataset = hDF5OutputFile->openDataset(dataset->getName());
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

void Processing::changeChunks()
{
    try {
        //HDF5Helper::MapOfDatasetsIt it;
        HDF5Helper::MapOfDatasets map = dtsForPcs->getDatasets();
        int count = 0;
        for (HDF5Helper::MapOfDatasetsIt it = map.begin(); it != map.end(); ++it) {
            HDF5Helper::HDF5Dataset *dataset = it->second;
            HDF5Helper::HDF5DatasetType datasetType = dataset->getType();

            if (datasetType == HDF5Helper::HDF5DatasetType::BASIC_3D
                    || datasetType == HDF5Helper::HDF5DatasetType::DWNSMPL_3D
                    || datasetType == HDF5Helper::HDF5DatasetType::CUBOID
                    || datasetType == HDF5Helper::HDF5DatasetType::CUBOID_ATTR
                    ) {
                changeChunksOfDataset(dataset);
                count++;
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

void Processing::donwsampling()
{
    try {
        // TODO - downsampling of FI an K datasets
        // For 3D type datasets
        if (!dtsForPcs->getDatasets(HDF5Helper::HDF5DatasetType::BASIC_3D).empty()) {

            HDF5Helper::MapOfDatasets map = dtsForPcs->getDatasets(HDF5Helper::HDF5DatasetType::BASIC_3D);
            for (HDF5Helper::MapOfDatasetsIt it = map.begin(); it != map.end(); ++it) {
                HDF5Helper::HDF5Dataset *dataset = it->second;
                resampleDataset(dataset);
            }
        } else if (!dtsForPcs->getDatasets(HDF5Helper::HDF5DatasetType::CUBOID).empty()) {
            // Cuboid datasets
            HDF5Helper::MapOfDatasets map = dtsForPcs->getDatasets(HDF5Helper::HDF5DatasetType::CUBOID);
            for (HDF5Helper::MapOfDatasetsIt it = map.begin(); it != map.end(); ++it) {
                HDF5Helper::HDF5Dataset *dataset = it->second;
                resampleDataset(dataset);
            }
        } else if (!dtsForPcs->getDatasets(HDF5Helper::HDF5DatasetType::CUBOID_ATTR).empty()) {
            // Cuboid datasets with attributes
            HDF5Helper::MapOfDatasets map = dtsForPcs->getDatasets(HDF5Helper::HDF5DatasetType::CUBOID_ATTR);
            for (HDF5Helper::MapOfDatasetsIt it = map.begin(); it != map.end(); ++it) {
                HDF5Helper::HDF5Dataset *dataset = it->second;
                resampleDataset(dataset);
            }
        } else {
            std::cout << "No datasets for downsampling in simulation output file" << std::endl;
        }
    } catch(std::exception &e) {
        std::cerr << e.what() << std::endl;
        std::exit(EXIT_FAILURE);
    }
}

void Processing::compress()
{
    try {
        if (!settings->getPeriod()) {
            std::cout << "No known period for compression" << std::endl;
            return;
        }
        HDF5Helper::MapOfDatasets map = dtsForPcs->getDatasets();
        hsize_t sensorMaskSize = dtsForPcs->getSensorMaskSize();
        int count = 0;
        std::cout << "Compression with period " << settings->getPeriod() << " steps of " << settings->getHarmonic() << ". harmonic frequency" << std::endl;
        for (HDF5Helper::MapOfDatasetsIt it = map.begin(); it != map.end(); ++it) {
            HDF5Helper::HDF5Dataset *dataset = it->second;
            HDF5Helper::HDF5DatasetType datasetType = dataset->getType(sensorMaskSize);

            if (datasetType == HDF5Helper::HDF5DatasetType::BASIC_MASK
                    || datasetType == HDF5Helper::HDF5DatasetType::CUBOID
                    || datasetType == HDF5Helper::HDF5DatasetType::CUBOID_ATTR
                    ) {
                std::cout << "Compression of dataset " << dataset->getName() << std::endl;
                compressDataset(dataset);
                count++;
                std::cout << "Compression of dataset " << dataset->getName() << " done" << std::endl << std::endl;
            }
        }

        if (count == 0) {
            std::cout << "No datasets for compression in simulation output file" << std::endl;
        }
    } catch(std::exception &e) {
        std::cerr << e.what() << std::endl;
        std::exit(EXIT_FAILURE);
    }
}

void Processing::decompress()
{
    try {
        HDF5Helper::MapOfDatasets map = dtsForPcs->getDatasets();
        hsize_t sensorMaskSize = dtsForPcs->getSensorMaskSize();
        int count = 0;
        std::vector<HDF5Helper::HDF5Dataset *> datasetsFi;
        std::vector<HDF5Helper::HDF5Dataset *> datasetsK;
        for (HDF5Helper::MapOfDatasetsIt it = map.begin(); it != map.end(); ++it) {
            HDF5Helper::HDF5Dataset *datasetFi = it->second;
            HDF5Helper::HDF5DatasetType datasetType = datasetFi->getType(sensorMaskSize);

            // TODO downsampled datasets
            if (datasetType == HDF5Helper::HDF5DatasetType::FI_MASK
                    || datasetType == HDF5Helper::HDF5DatasetType::CUBOID_FI
                    || datasetType == HDF5Helper::HDF5DatasetType::CUBOID_ATTR_FI
                    //|| datasetType == HDF5Helper::HDF5DatasetType::CUBOID_DWNSMPL_FI
                    //|| datasetType == HDF5Helper::HDF5DatasetType::CUBOID_ATTR_DWNSMPL_FI
                    ) {

                std::string nameFi = datasetFi->readAttributeS(HDF5Helper::SRC_DATASET_NAME_ATTR, false);
                hsize_t periodFi = datasetFi->readAttributeI(HDF5Helper::C_PERIOD_ATTR, false);
                hsize_t harmonicFi = 1;
                if (datasetFi->hasAttribute(HDF5Helper::C_HARMONIC_ATTR)) {
                    harmonicFi = datasetFi->readAttributeI(HDF5Helper::C_HARMONIC_ATTR, false);
                }

                HDF5Helper::HDF5Dataset *datasetK = 0;

                for (HDF5Helper::MapOfDatasetsIt it2 = map.begin(); it2 != map.end(); ++it2) {
                    HDF5Helper::HDF5Dataset *dataset = it2->second;
                    HDF5Helper::HDF5DatasetType datasetKType = dataset->getType(sensorMaskSize);
                    if ((datasetKType == HDF5Helper::HDF5DatasetType::K_MASK && datasetType == HDF5Helper::HDF5DatasetType::FI_MASK)
                            || (datasetKType == HDF5Helper::HDF5DatasetType::CUBOID_K && datasetType == HDF5Helper::HDF5DatasetType::CUBOID_FI)
                            || (datasetKType == HDF5Helper::HDF5DatasetType::CUBOID_ATTR_K && datasetType == HDF5Helper::HDF5DatasetType::CUBOID_ATTR_FI)
                            ) {
                        hsize_t harmonicK = 1;
                        if (dataset->hasAttribute(HDF5Helper::C_HARMONIC_ATTR)) {
                            harmonicK = dataset->readAttributeI(HDF5Helper::C_HARMONIC_ATTR, false);
                        }
                        // TODO Check C_MOS_ATTR, number of coefficients, same periods, harmonic indices
                        if (periodFi == dataset->readAttributeI(HDF5Helper::C_PERIOD_ATTR, false)
                                && nameFi == dataset->readAttributeS(HDF5Helper::SRC_DATASET_NAME_ATTR, false)
                                && harmonicK == harmonicFi
                                ) {
                            datasetK = dataset;
                            break;
                        }
                    }
                }

                if (datasetK) {
                    datasetsFi.push_back(datasetFi);
                    datasetsK.push_back(datasetK);
                    count++;
                }
            }
        }

        if (count == 0) {
            std::cout << "No datasets for decompression in simulation output file" << std::endl;
        } else {
            std::cout << "Decompression of datasets ..." << std::endl;
            decompressDatasets(datasetsFi, datasetsK);
            std::cout << "Decompression of datasets done" << std::endl << std::endl;
        }

    } catch(std::exception &e) {
        std::cerr << e.what() << std::endl;
        std::exit(EXIT_FAILURE);
    }
}

void Processing::difference()
{
    try {
        HDF5Helper::MapOfDatasets map = dtsForPcs->getDatasets();
        hsize_t sensorMaskSize = dtsForPcs->getSensorMaskSize();
        int count = 0;
        for (HDF5Helper::MapOfDatasetsIt it = map.begin(); it != map.end(); ++it) {
            HDF5Helper::HDF5Dataset *datasetOriginal = it->second;
            HDF5Helper::HDF5DatasetType datasetType = datasetOriginal->getType(sensorMaskSize);

            // TODO downsampled datasets
            if (datasetType == HDF5Helper::HDF5DatasetType::BASIC_MASK
                    || datasetType == HDF5Helper::HDF5DatasetType::CUBOID
                    || datasetType == HDF5Helper::HDF5DatasetType::CUBOID_ATTR
                    ) {

                HDF5Helper::HDF5Dataset *datasetDecoded = 0;

                for (HDF5Helper::MapOfDatasetsIt it2 = map.begin(); it2 != map.end(); ++it2) {
                    HDF5Helper::HDF5Dataset *dataset = it2->second;
                    HDF5Helper::HDF5DatasetType datasetDecodedType = dataset->getType(sensorMaskSize);
                    if ((datasetDecodedType == HDF5Helper::HDF5DatasetType::D_MASK && datasetType == HDF5Helper::HDF5DatasetType::BASIC_MASK)
                            || (datasetDecodedType == HDF5Helper::HDF5DatasetType::CUBOID_D && datasetType == HDF5Helper::HDF5DatasetType::CUBOID)
                            || (datasetDecodedType == HDF5Helper::HDF5DatasetType::CUBOID_ATTR_D && datasetType == HDF5Helper::HDF5DatasetType::CUBOID_ATTR)
                            ) {
                        if (datasetOriginal->getName() == dataset->readAttributeS(HDF5Helper::SRC_DATASET_NAME_ATTR, false)) {
                            datasetDecoded = dataset;
                            break;
                        }
                    }
                }

                if (datasetDecoded) {
                    std::cout << "Subtraction of datasets " << datasetOriginal->getName() << " and " << datasetDecoded->getName() << std::endl;
                    subtractDatasets(datasetOriginal, datasetDecoded);
                    std::cout << "Subtraction of datasets done" << std::endl << std::endl;
                    count++;
                }
            }
        }

        if (count == 0) {
            std::cout << "No datasets for making difference in simulation output file" << std::endl;
        }
    } catch(std::exception &e) {
        std::cerr << e.what() << std::endl;
        std::exit(EXIT_FAILURE);
    }
}

void Processing::findMinAndMaxPositionFromSensorMask(HDF5Helper::HDF5Vector3D &min, HDF5Helper::HDF5Vector3D &max)
{
    // Find min and max position from linear saved values
    hsize_t index = 0;
    hsize_t *data;
    HDF5Helper::HDF5Vector3D offset;
    HDF5Helper::HDF5Vector3D count;
    HDF5Helper::HDF5Vector3D dstPos;

    for (hsize_t i = 0; i < dtsForPcs->getSensorMaskIndexDataset()->getNumberOfBlocks(); i++) {
        dtsForPcs->getSensorMaskIndexDataset()->readBlock(i, offset, count, data);
        for (hsize_t z = 0; z < count.z(); z++)
            for (hsize_t y = 0; y < count.y(); y++)
                for (hsize_t x = 0; x < count.x(); x++) {
                    // [x + y * count.x() + z * count.x() * count.y()] is from 0, but value of index is from 1
                    index = data[x + y * count.x() + z * count.x() * count.y()] - 1;
                    HDF5Helper::convertlinearToMultiDim(index, dstPos, HDF5Helper::HDF5Vector3D(dtsForPcs->getNDims()));

                    if (dstPos.x() < min.x()) min.x(dstPos.x());
                    if (dstPos.y() < min.y()) min.y(dstPos.y());
                    if (dstPos.z() < min.z()) min.z(dstPos.z());
                    if (dstPos.x() > max.x()) max.x(dstPos.x());
                    if (dstPos.y() > max.y()) max.y(dstPos.y());
                    if (dstPos.z() > max.z()) max.z(dstPos.z());
                }
        delete[] data; // !!
    }

    std::cout << "   min point:\t" << min << std::endl;
    std::cout << "   max point:\t" << max << std::endl;
}

void Processing::computeDstDims(HDF5Helper::HDF5Vector3D dimsSrc, float ratio, HDF5Helper::HDF5Vector3D &dimsDst, HDF5Helper::HDF5Vector3D &chunkDims, Settings *settings)
{
    dimsDst.z(Helper::round(dimsSrc.z() * ratio));
    dimsDst.y(Helper::round(dimsSrc.y() * ratio));
    dimsDst.x(Helper::round(dimsSrc.x() * ratio));
    // Check < 1
    if (dimsDst.z() < 1) dimsDst.z(1);
    if (dimsDst.y() < 1) dimsDst.y(1);
    if (dimsDst.x() < 1) dimsDst.x(1);
    // Chunk size
    chunkDims.z(settings->getMaxChunkSize());
    chunkDims.y(settings->getMaxChunkSize());
    chunkDims.x(settings->getMaxChunkSize());
    if (chunkDims.z() > dimsDst.z()) chunkDims.z(dimsDst.z());
    if (chunkDims.y() > dimsDst.y()) chunkDims.y(dimsDst.y());
    if (chunkDims.x() > dimsDst.x()) chunkDims.x(dimsDst.x());
    std::cout << "   new size:\t" << dimsDst << std::endl;
}

void Processing::changeChunksOfDataset(HDF5Helper::HDF5Dataset *srcDataset)
{
    HDF5Helper::HDF5Vector dims = srcDataset->getDims();

    // Chunk dims
    HDF5Helper::HDF5Vector chunkDims(dims.getLength(), 1);

    #pragma omp parallel for
    for (hssize_t i = 0; i < hssize_t(dims.getLength()); i++) {
        chunkDims[i] = settings->getMaxChunkSize();
        if (chunkDims[i] > dims[i]) chunkDims[i] = dims[i];
    }

    // Create dst dataset
    hDF5OutputFile->createDatasetF(srcDataset->getName(), dims, chunkDims, true);
    HDF5Helper::HDF5Dataset *dstDataset = hDF5OutputFile->openDataset(srcDataset->getName());

    double t0 = HDF5Helper::getTime();

    float *data = 0;
    float minV, maxV;
    float minVG = 0, maxVG = 0;
    bool first = true;
    HDF5Helper::HDF5Vector offset;
    HDF5Helper::HDF5Vector count;

    // Change chunks
    for (hsize_t i = 0; i < srcDataset->getNumberOfBlocks(); i++) {
        srcDataset->readBlock(i, offset, count, data, minV, maxV);
        dstDataset->writeDataset(offset, count, data, true);
        delete[] data;
        if (first) {
            minVG = minV;
            maxVG = maxV;
            first = false;
        }
        if (minVG > minV) minVG = minV;
        if (maxVG < maxV) maxVG = maxV;
    }

    // Copy attributes
    copyAttributes(srcDataset, dstDataset);

    // Set min/max values
    dstDataset->setAttribute(HDF5Helper::MIN_ATTR, minVG);
    dstDataset->setAttribute(HDF5Helper::MAX_ATTR, maxVG);

    double t1 = HDF5Helper::getTime();
    std::cout << "Time of changing chunks of the whole dataset: " << (t1 - t0) << " ms; \t" << std::endl;

    hDF5OutputFile->closeDataset(dstDataset);
}

void Processing::resampleDataset(HDF5Helper::HDF5Dataset *srcDataset)
{
    // Dims
    HDF5Helper::HDF5Vector dimsSrc = srcDataset->getDims();
    HDF5Helper::HDF5Vector3D dimsSrc3D = dimsSrc;
    HDF5Helper::HDF5Vector dimsDst;
    HDF5Helper::HDF5Vector3D dimsDst3D;
    HDF5Helper::HDF5Vector chunkDimsDst;
    HDF5Helper::HDF5Vector3D chunkDimsDst3D;

    // Compute ratio
    float ratio = float(settings->getMaxSize()) / std::max(std::max(dimsSrc3D.z(), dimsSrc3D.y()), dimsSrc3D.x());
    // Check downsampling size
    if (ratio >= 1) {
        std::cout << "Bad destination size for downsampling" << std::endl;
        return;
    }

    // Compute destination dims
    computeDstDims(dimsSrc3D, ratio, dimsDst3D, chunkDimsDst3D, settings);

    // For 4D datasets
    if (dimsSrc.getLength() == 4) {
        chunkDimsDst = HDF5Helper::HDF5Vector4D(settings->getMaxChunkSize(), chunkDimsDst3D); // Maybe better is (1, chunkSizeDst3D)
        dimsDst = HDF5Helper::HDF5Vector4D(HDF5Helper::HDF5Vector4D(dimsSrc).w(), dimsDst3D);
    } else { // 3D datasets
        chunkDimsDst = chunkDimsDst3D;
        dimsDst = dimsDst3D;
    }

    // Create dst dataset
    hDF5OutputFile->createDatasetF(srcDataset->getName() + "_" + std::to_string(settings->getMaxSize()), dimsDst, chunkDimsDst, true);
    HDF5Helper::HDF5Dataset *dstDataset = hDF5OutputFile->openDataset(srcDataset->getName() + "_" + std::to_string(settings->getMaxSize()));

    double t0 = HDF5Helper::getTime();

    // Tmp has original Z dimension (hDF5SimOutputFile->getNZ())
    HDF5Helper::HDF5Vector3D newTmpDatasetDims(dimsSrc3D.z(), dimsDst3D.y(), dimsDst3D.x());
    HDF5Helper::HDF5Vector3D newTmpDatasetChunkDims(dimsSrc3D.z(), 1, dimsDst3D.x());

    // Create temp file and dataset
    HDF5Helper::File *tmpFile = new HDF5Helper::File("tmp.h5", HDF5Helper::File::CREATE);
    tmpFile->createDatasetF("tmp", newTmpDatasetDims, newTmpDatasetChunkDims);
    HDF5Helper::HDF5Dataset *tmpDataset = tmpFile->openDataset("tmp");

    float *srcData = 0;
    float *dstData = 0;

    hsize_t steps = 1;
    if (dimsSrc.getLength() == 4)
        steps = HDF5Helper::HDF5Vector4D(dimsSrc).w();

    for (unsigned int t = 0; t < steps; t++) {
        // If we have enough memory, resample full 3D dataset
        if (srcDataset->getNumberOfElmsToLoad() >= srcDataset->getSize()) {
            if (dimsSrc.getLength() == 4)
                srcDataset->readDataset(HDF5Helper::HDF5Vector4D(t, 0, 0, 0), HDF5Helper::HDF5Vector4D(1, dimsSrc3D.z(), dimsSrc3D.y(), dimsSrc3D.x()), srcData);
            else
                srcDataset->readDataset(srcData);
            dstData = new float[dimsDst3D.x() * dimsDst3D.y() * dimsDst3D.z()]();
            resize3D(srcData, dstData, dimsSrc3D.x(), dimsSrc3D.y(), dimsSrc3D.z(), dimsDst3D.x(), dimsDst3D.y(), dimsDst3D.z());
            if (dimsSrc.getLength() == 4)
                dstDataset->writeDataset(HDF5Helper::HDF5Vector4D(t, 0, 0, 0), HDF5Helper::HDF5Vector4D(1, dimsDst3D.z(), dimsSrc3D.y(), dimsDst3D.x()), dstData, true);
            else
                dstDataset->writeDataset(dstData, true);
            delete[] srcData;
            delete[] dstData;
        } else {
            // First 2D slices in XY plane
            for (unsigned int z = 0; z < dimsSrc3D.z(); z++) {
                if (dimsSrc.getLength() == 4)
                    srcDataset->readDataset(HDF5Helper::HDF5Vector4D(t, z, 0, 0), HDF5Helper::HDF5Vector4D(1, 1, dimsSrc3D.y(), dimsSrc3D.x()), srcData);
                else
                    srcDataset->readDataset(HDF5Helper::HDF5Vector3D(z, 0, 0), HDF5Helper::HDF5Vector3D(1, dimsSrc3D.y(), dimsSrc3D.x()), srcData);
                dstData = new float[dimsDst3D.x() * dimsDst3D.y()]();
                resize2D(srcData, dstData, dimsSrc3D.x(), dimsSrc3D.y(), dimsDst3D.x(), dimsDst3D.y());
                tmpDataset->writeDataset(HDF5Helper::HDF5Vector3D(z, 0, 0), HDF5Helper::HDF5Vector3D(1, dimsDst3D.y(), dimsDst3D.x()), dstData, true);
                delete[] srcData;
                delete[] dstData;
            }

            // and after 2D slices XZ plane
            for (unsigned int y = 0; y < dimsDst3D.y(); y++) {
                tmpDataset->readDataset(HDF5Helper::HDF5Vector3D(0, y, 0), HDF5Helper::HDF5Vector3D(dimsSrc3D.z(), 1, dimsDst3D.x()), srcData);
                dstData = new float[dimsDst3D.x() * dimsDst3D.z()]();
                resize2D(srcData, dstData, dimsDst3D.x(), dimsSrc3D.z(), dimsDst3D.x(), dimsDst3D.z());
                if (dimsSrc.getLength() == 4)
                    dstDataset->writeDataset(HDF5Helper::HDF5Vector4D(t, 0, y, 0), HDF5Helper::HDF5Vector4D(1, dimsDst3D.z(), 1, dimsDst3D.x()), dstData, true);
                else
                    dstDataset->writeDataset(HDF5Helper::HDF5Vector3D(0, y, 0), HDF5Helper::HDF5Vector3D(dimsDst3D.z(), 1, dimsDst3D.x()), dstData, true);
                delete[] srcData;
                delete[] dstData;
            }
        }
    }

    delete tmpFile;
    remove("tmp.h5");

    dstDataset->findAndSetGlobalMinAndMaxValue();

    // Save attributes
    dstDataset->setAttribute(HDF5Helper::SRC_SIZE_X_ATTR, dimsSrc3D.x());
    dstDataset->setAttribute(HDF5Helper::SRC_SIZE_Y_ATTR, dimsSrc3D.y());
    dstDataset->setAttribute(HDF5Helper::SRC_SIZE_Z_ATTR, dimsSrc3D.z());
    dstDataset->setAttribute(HDF5Helper::SRC_DATASET_NAME_ATTR, srcDataset->getName());

    if (srcDataset->hasAttribute(HDF5Helper::POSITION_Z_ATTR)
            && srcDataset->hasAttribute(HDF5Helper::POSITION_Z_ATTR)
            && srcDataset->hasAttribute(HDF5Helper::POSITION_Z_ATTR)
            && srcDataset->hasAttribute(HDF5Helper::POSITION_Z_ATTR)
            ) {
        hsize_t z = Helper::round(srcDataset->readAttributeI(HDF5Helper::POSITION_Z_ATTR, false) * ratio);
        hsize_t y = Helper::round(srcDataset->readAttributeI(HDF5Helper::POSITION_Y_ATTR, false) * ratio);
        hsize_t x = Helper::round(srcDataset->readAttributeI(HDF5Helper::POSITION_X_ATTR, false) * ratio);

        dstDataset->setAttribute(HDF5Helper::POSITION_Z_ATTR, z);
        dstDataset->setAttribute(HDF5Helper::POSITION_Y_ATTR, y);
        dstDataset->setAttribute(HDF5Helper::POSITION_X_ATTR, x);
    }
    double t1 = HDF5Helper::getTime();
    std::cout << "Time of resampling of the whole dataset: " << (t1 - t0) << " ms; \t" << std::endl;
}

void Processing::compressDataset(HDF5Helper::HDF5Dataset *srcDataset)
{
    // Only one coding parameter - multiple of overlap size
    // Overlap size
    hsize_t oSize = settings->getPeriod() * settings->getMOS();
    // Base size
    hsize_t bSize = oSize * 2 + 1;

    // Get dims
    HDF5Helper::HDF5Vector dims = srcDataset->getDims();
    HDF5Helper::HDF5Vector outputDims = dims;

    // Compute steps, step size and output dims
    hsize_t steps = 0;
    hsize_t outputSteps = 0;
    hsize_t stepSize = 0;
    if (dims.getLength() == 4) { // 4D dataset
        steps = HDF5Helper::HDF5Vector4D(dims).w();
        outputSteps = hsize_t(floor(float(steps) / oSize));
        outputDims[0] = outputSteps;
        stepSize = outputDims[1] * outputDims[2] * outputDims[3];
    } else if (dims.getLength() == 3) { // 3D dataset (defined by sensor mask)
        steps = HDF5Helper::HDF5Vector3D(dims).y();
        outputSteps = hsize_t(floor(float(steps) / oSize));
        outputDims[1] = outputSteps;
        stepSize = outputDims[2];
    } else { // Something wrong.
        std::cout << "Something wrong with dataset dims" << std::endl;
        return;
    }

    // Memory for helper functions data
    float *b = new float[bSize]();
    Helper::floatC *e = new Helper::floatC[bSize]();
    Helper::floatC *bE = new Helper::floatC[bSize]();
    Helper::floatC *bE_1 = new Helper::floatC[bSize]();

    // Generate basis function (window)
    Helper::triangular(oSize, b);  // Triangular window
    //Helper::hann(oSize, b);        // Hann window

    // Generate complex exponential functions
    Helper::floatC i(0, -1);
    #pragma omp parallel for
    for (hssize_t x = 0; x < hssize_t(bSize); x++) {
        e[x] = std::exp(i * (2.0f * float(M_PI) / (float(settings->getPeriod()) / settings->getHarmonic())) * float(x));
    }
    #pragma omp parallel for
    for (hssize_t x = 0; x < hssize_t(bSize); x++) {
        bE[x] = b[x] * e[x] * (2.0f / float(oSize));
        bE_1[x] = b[(hsize_t(x) + oSize) % (bSize - 1)] * e[(hsize_t(x) + oSize) % (bSize - 1)] * (2.0f / float(oSize));
    }

    // Compute new chunk size
    HDF5Helper::HDF5Vector chunkDims(outputDims.getLength(), 1);
    #pragma omp parallel for
    for (hssize_t i = 1; i < hssize_t(outputDims.getLength()); i++) {
        chunkDims[i] = settings->getMaxChunkSize();
        if (chunkDims[i] > outputDims[i]) chunkDims[i] = outputDims[i];
    }
    if (dims.getLength() == 3) { // Original chunks layout
        chunkDims = outputDims;
        chunkDims[1] = 1;
    }

    std::cout << "steps:       " << steps << std::endl;
    std::cout << "outputSteps: " << outputSteps << std::endl;
    std::cout << "dims:        " << dims << std::endl;
    std::cout << "outputDims:  " << outputDims << std::endl;
    std::cout << "stepSize:    " << stepSize << std::endl;

    // Create dst datasets
    hDF5OutputFile->createDatasetF(srcDataset->getName() + "_k_" + std::to_string(settings->getHarmonic()), outputDims, chunkDims, true);
    hDF5OutputFile->createDatasetF(srcDataset->getName() + "_fi_" + std::to_string(settings->getHarmonic()), outputDims, chunkDims, true);
    HDF5Helper::HDF5Dataset *dstDatasetK = hDF5OutputFile->openDataset(srcDataset->getName() + "_k_" + std::to_string(settings->getHarmonic()));
    HDF5Helper::HDF5Dataset *dstDatasetFi = hDF5OutputFile->openDataset(srcDataset->getName() + "_fi_" + std::to_string(settings->getHarmonic()));

    double t0 = HDF5Helper::getTime();

    // Variables for block reading
    float *data = 0;
    HDF5Helper::HDF5Vector offset;
    HDF5Helper::HDF5Vector count;
    float maxVK = 0, maxVFi = 0;
    float minVK = 0, minVFi = 0;
    bool first = true;

    // If we have enough memory - minimal for one full step in 3D space
    if (srcDataset->getNumberOfElmsToLoad() >= stepSize) {

        // Complex buffers for accumulation
        Helper::floatC *sCTmp1 = new Helper::floatC[stepSize]();
        Helper::floatC *sCTmp2 = new Helper::floatC[stepSize]();

        //hsize_t step = 0;
        hsize_t frame = 1;

        float *dataK = new float[stepSize];
        float *dataFi = new float[stepSize];

        // Reading and compression
        for (hsize_t i = 0; i < srcDataset->getNumberOfBlocks(); i++) {
            srcDataset->readBlock(i, offset, count, data);

            hsize_t stepsCount;
            hsize_t stepsOffset;

            if (dims.getLength() == 4) { // 4D dataset
                stepsCount = count[0];
                stepsOffset = offset[0];
            } else { // 3D dataset
                stepsCount = count[1];
                stepsOffset = offset[1];
            }

            // For every step
            for (hsize_t step = 0; step < stepsCount; step++) {
                //std::cout << "Encoding step " << stepsOffset + step << std::endl;

                // Compute local index
                hsize_t stepLocal = (stepsOffset + step) % (bSize - 1);

                // For every point
                #pragma omp parallel for
                for (hssize_t p = 0; p < hssize_t(stepSize); p++) {
                    // Correlation step
                    sCTmp1[p] += bE[stepLocal] * data[step * stepSize + hsize_t(p)];
                    sCTmp2[p] += bE_1[stepLocal] * data[step * stepSize + hsize_t(p)];

                    //std::cout << "step: " << step << std::endl;
                    //std::cout << "step * stepSize: " << step << std::endl;

                    // Check if we are at saving point
                    //if (stepLocal + 1 == oSize || stepLocal + 1 == oSize * 2) {
                    if ((stepLocal + 1) % oSize == 0) {

                        Helper::floatC sC;

                        // Select accumulated value
                        if ((frame % 2) == 0) {
                            sC = sCTmp1[p];
                            sCTmp1[p] = 0;
                        } else {
                            sC = sCTmp2[p];
                            sCTmp2[p] = 0;
                        }

                        // Normalization
                        //sC = sC * 2.0f / float(oSize);

                        // Computing amplitude
                        float k = abs(sC);

                        // Computing phase
                        float fi = arg(sC);

                        // Save complex coefficients
                        // Drop first "half" frame
                        if (frame > 1) {
                            dataK[p] = k;
                            dataFi[p] = fi;

                            // Min/max values
                            HDF5Helper::checkOrSetMinMaxValue(first, minVK, maxVK, k);
                            HDF5Helper::checkOrSetMinMaxValue(first, minVFi, maxVFi, fi);
                        }

                    }
                }

                // Increment frame
                //if (stepLocal == oSize - 1 || stepLocal == oSize * 2 - 1) {
                if ((stepLocal + 1) % oSize == 0) {
                    if (frame > 1) {
                        std::cout << "Saving frame " << frame - 1 << " ... ";
                        if (dims.getLength() == 4) { // 4D dataset
                            dstDatasetK->writeDataset(HDF5Helper::HDF5Vector4D(frame - 2, 0, 0, 0), HDF5Helper::HDF5Vector4D(1, dims[1], dims[2], dims[3]), dataK);
                            dstDatasetFi->writeDataset(HDF5Helper::HDF5Vector4D(frame - 2, 0, 0, 0), HDF5Helper::HDF5Vector4D(1, dims[1], dims[2], dims[3]), dataFi);
                        } else if (dims.getLength() == 3) {
                            dstDatasetK->writeDataset(HDF5Helper::HDF5Vector3D(0, frame - 2, 0), HDF5Helper::HDF5Vector3D(1, 1, dims[2]), dataK);
                            dstDatasetFi->writeDataset(HDF5Helper::HDF5Vector3D(0, frame - 2, 0), HDF5Helper::HDF5Vector3D(1, 1, dims[2]), dataFi);
                        }
                        std::cout << "saved" << std::endl;
                    }
                    frame++;
                }
                //std::cout << "encoded" << std::endl;
            }

            // Last frame (copy last)
            if (frame - 1 == outputSteps) {
                std::cout << "Saving frame " << frame - 1 << " ... ";
                if (dims.getLength() == 4) { // 4D dataset
                    dstDatasetK->writeDataset(HDF5Helper::HDF5Vector4D(frame - 2, 0, 0, 0), HDF5Helper::HDF5Vector4D(1, dims[1], dims[2], dims[3]), dataK);
                    dstDatasetFi->writeDataset(HDF5Helper::HDF5Vector4D(frame - 2, 0, 0, 0), HDF5Helper::HDF5Vector4D(1, dims[1], dims[2], dims[3]), dataFi);
                } else if (dims.getLength() == 3) {
                    dstDatasetK->writeDataset(HDF5Helper::HDF5Vector3D(0, frame - 2, 0), HDF5Helper::HDF5Vector3D(1, 1, dims[2]), dataK);
                    dstDatasetFi->writeDataset(HDF5Helper::HDF5Vector3D(0, frame - 2, 0), HDF5Helper::HDF5Vector3D(1, 1, dims[2]), dataFi);
                }
                std::cout << "saved" << std::endl;
            }

            delete[] data;
        }

        delete[] dataK;
        delete[] dataFi;

        // Delete complex buffers
        delete[] sCTmp1;
        delete[] sCTmp2;

    } else {
        // Not implemented yet
        std::cout << "Not implemented for such big datasets yet" << std::endl;
        delete[] b;
        delete[] e;
        delete[] bE;
        delete[] bE_1;
        hDF5OutputFile->closeDataset(dstDatasetK);
        hDF5OutputFile->closeDataset(dstDatasetFi);
        return;
    }

    // Delete some memory
    delete[] b;
    delete[] e;
    delete[] bE;
    delete[] bE_1;

    // Copy attributes
    copyAttributes(srcDataset, dstDatasetK);
    copyAttributes(srcDataset, dstDatasetFi);

    // Set min/max values
    dstDatasetK->setAttribute(HDF5Helper::MIN_ATTR, minVK);
    dstDatasetK->setAttribute(HDF5Helper::MAX_ATTR, maxVK);
    dstDatasetK->setAttribute(HDF5Helper::SRC_DATASET_NAME_ATTR, srcDataset->getName());
    dstDatasetK->setAttribute(HDF5Helper::C_TYPE_ATTR, "k");
    dstDatasetK->setAttribute(HDF5Helper::C_PERIOD_ATTR, settings->getPeriod());
    dstDatasetK->setAttribute(HDF5Helper::C_HARMONIC_ATTR, settings->getHarmonic());
    dstDatasetK->setAttribute(HDF5Helper::C_MOS_ATTR, settings->getMOS());
    dstDatasetFi->setAttribute(HDF5Helper::MIN_ATTR, minVFi);
    dstDatasetFi->setAttribute(HDF5Helper::MAX_ATTR, maxVFi);
    dstDatasetFi->setAttribute(HDF5Helper::SRC_DATASET_NAME_ATTR, srcDataset->getName());
    dstDatasetFi->setAttribute(HDF5Helper::C_TYPE_ATTR, "fi");
    dstDatasetFi->setAttribute(HDF5Helper::C_PERIOD_ATTR, settings->getPeriod());
    dstDatasetFi->setAttribute(HDF5Helper::C_HARMONIC_ATTR, settings->getHarmonic());
    dstDatasetFi->setAttribute(HDF5Helper::C_MOS_ATTR, settings->getMOS());

    double t1 = HDF5Helper::getTime();
    std::cout << "Time of the whole dataset compression: " << (t1 - t0) << " ms; \t" << std::endl;

    hDF5OutputFile->closeDataset(dstDatasetK);
    hDF5OutputFile->closeDataset(dstDatasetFi);
}

void Processing::decompressDatasets(std::vector<HDF5Helper::HDF5Dataset *> srcDatasetsFi, std::vector<HDF5Helper::HDF5Dataset *> srcDatasetsK)
{
    // First decoding parameter - multiple of overlap size
    hsize_t s = 1;
    //if (srcDatasetFi->hasAttribute(HDF5Helper::C_MOS_ATTR))
    //    s = srcDatasetFi->readAttributeI(HDF5Helper::C_MOS_ATTR);
    // Second decoding parametr - period
    hsize_t period = srcDatasetsFi.at(0)->readAttributeI(HDF5Helper::C_PERIOD_ATTR);
    // Harmonic frequency
    hsize_t harmonics = srcDatasetsFi.size();
    //if (srcDatasetFi->hasAttribute(HDF5Helper::C_HARMONIC_ATTR)) {
    //    harmonic = srcDatasetFi->readAttributeI(HDF5Helper::C_HARMONIC_ATTR);
    //}

    // Overlap size
    hsize_t oSize = period * s;
    // Base size
    hsize_t bSize = oSize * 2 + 1;

    // Get dims
    HDF5Helper::HDF5Vector dims = srcDatasetsFi.at(0)->getDims();
    HDF5Helper::HDF5Vector outputDims = dims;

    // TODO - check same dims of srcDatasetFi and srcDatasetK

    // Compute steps, step size and output dims
    hsize_t steps = 0;
    hsize_t outputSteps = 0;
    hsize_t stepSize = 0;
    if (dims.getLength() == 4) { // 4D dataset
        steps = HDF5Helper::HDF5Vector4D(dims).w();
        outputSteps = (steps + 1) * oSize ;
        outputDims[0] = outputSteps;
        stepSize = outputDims[1] * outputDims[2] * outputDims[3];
    } else if (dims.getLength() == 3) { // 3D dataset (defined by sensor mask)
        steps = HDF5Helper::HDF5Vector3D(dims).y();
        outputSteps = (steps + 1) * oSize;
        outputDims[1] = outputSteps;
        stepSize = outputDims[2];
    } else { // Something wrong.
        std::cout << "Something wrong with dataset dims" << std::endl;
        return;
    }


    // Memory for helper functions data, 2d arrays for harmonics
    float *b = new float[bSize]();
    Helper::floatC *e = new Helper::floatC[bSize * harmonics]();
    Helper::floatC *bE = new Helper::floatC[bSize* harmonics]();
    Helper::floatC *bE_1 = new Helper::floatC[bSize* harmonics]();

    // Generate basis function (window)
    Helper::triangular(oSize, b);  // Triangular window
    //Helper::hann(oSize, b);        // Hann window

    // Generate complex exponential functions
    Helper::floatC i(0, -1);
    #pragma omp parallel for
    for (hssize_t h = 0; h < hssize_t(harmonics); h++) {
        hsize_t ih = 1;
        if (srcDatasetsFi.at(hsize_t(h))->hasAttribute(HDF5Helper::C_HARMONIC_ATTR)) {
            ih = srcDatasetsFi.at(hsize_t(h))->readAttributeI(HDF5Helper::C_HARMONIC_ATTR);
        }
        #pragma omp parallel for
        for (hssize_t x = 0; x < hssize_t(bSize); x++) {
            hssize_t hx = h * hssize_t(bSize) + x;
            e[hx] = std::exp(i * (2.0f * float(M_PI) / (float(period) / (ih))) * float(x));
        }
    }
    #pragma omp parallel for
    for (hssize_t h = 0; h < hssize_t(harmonics); h++) {
        #pragma omp parallel for
        for (hssize_t x = 0; x < hssize_t(bSize); x++) {
            hssize_t hx = h * hssize_t(bSize) + x;
            bE[hx] = b[x] * e[hx];
            bE_1[hx] = b[(hsize_t(x) + oSize) % (bSize - 1)] * e[hsize_t(h) * bSize + ((hsize_t(x) + oSize) % (bSize - 1))];
        }
    }

    // Compute new chunk dims (it is 1 for time dimension)
    HDF5Helper::HDF5Vector chunkDims(outputDims.getLength(), 1);
    #pragma omp parallel for
    for (hssize_t i = 1; i < hssize_t(outputDims.getLength()); i++) {
        chunkDims[i] = settings->getMaxChunkSize();
        if (chunkDims[i] > outputDims[i]) chunkDims[i] = outputDims[i];
    }
    if (dims.getLength() == 3) { // Original chunks layout
        chunkDims = outputDims;
        chunkDims[1] = 1;
    }

    std::cout << "steps:        " << steps << std::endl;
    std::cout << "output steps: " << outputSteps << std::endl;
    std::cout << "dims:         " << dims << std::endl;
    std::cout << "output dims:  " << outputDims << std::endl;
    std::cout << "step size:    " << stepSize << std::endl;

    // Create dst dataset
    std::string srcName = srcDatasetsFi.at(0)->readAttributeS(HDF5Helper::SRC_DATASET_NAME_ATTR, false);
    hDF5OutputFile->createDatasetF(srcName + "_d", outputDims, chunkDims, true);
    HDF5Helper::HDF5Dataset *dstDataset = hDF5OutputFile->openDataset(srcName + "_d");

    double t0 = HDF5Helper::getTime();

    // Variables for block reading
    float **dataFi = new float*[harmonics];
    float **dataK = new float*[harmonics];
    float *dataFiL = 0;
    float *dataKL = 0;
    HDF5Helper::HDF5Vector offset;
    HDF5Helper::HDF5Vector count;
    float maxV = 0, minV = 0;
    bool first = true;

    // If we have enough memory - minimal for one full step * (1 (data) + 4 buffers (k, fi, lastK, lastFi) * number of harmonics) in 3D space
    if (srcDatasetsFi.at(0)->getFile()->getNumberOfElmsToLoad() >= stepSize * (1 + 4 * harmonics)) {
        // Buffers for last K and Fi
        float *k = new float[stepSize * harmonics]();
        float *fi = new float[stepSize * harmonics]();
        float *lastK = new float[stepSize * harmonics]();
        float *lastFi = new float[stepSize * harmonics]();
        float *data = new float[stepSize]();

        hsize_t frameDst = 0;

        // Reading and decompression
        for (hsize_t i = 0; i < srcDatasetsFi.at(0)->getNumberOfBlocks(); i++) {
            for (hsize_t h = 0; h < harmonics; h++) {
                srcDatasetsFi.at(h)->readBlock(i, offset, count, dataFiL);
                srcDatasetsK.at(h)->readBlock(i, offset, count, dataKL);
                dataFi[h] = dataFiL;
                dataK[h] = dataKL;
            }

            hsize_t framesCount;
            hsize_t framesOffset;

            if (dims.getLength() == 4) { // 4D dataset
                framesCount = count[0];
                framesOffset = offset[0];
            } else { // 3D dataset
                framesCount = count[1];
                framesOffset = offset[1];
            }

            // Because of last coefficient duplication
            bool lastFlag = false;
            if (framesOffset + framesCount == steps) {
                lastFlag = true;
                framesCount++;
            }

            // For every frame
            for (hsize_t frame = 0; frame < framesCount; frame++) {
                std::cout << "Decoding frame " << frameDst << std::endl;
                // Decode steps
                for (hsize_t p = 0; p < oSize; p++) {
                    // For every coefficient in step

                    double t0 = HDF5Helper::getTime();

                    #pragma omp parallel for
                    for (hssize_t cKFi = 0; cKFi < hssize_t(stepSize); cKFi++) {
                        if (p == 0) {
                            // Save last coefficients
                            for (hsize_t h = 0; h < harmonics; h++) {
                                hsize_t hcKFi = stepSize * h + hsize_t(cKFi);
                                lastK[hcKFi] = k[hcKFi];
                                lastFi[hcKFi] = fi[hcKFi];

                                // Copy first coefficients
                                if (frame == 0) {
                                    lastK[hcKFi] = dataK[h][cKFi];
                                    lastFi[hcKFi] = dataFi[h][cKFi];
                                }

                                if (frame == framesCount - 1 && lastFlag) {
                                    // Duplicate last coefficient
                                    k[hcKFi] = dataK[h][(frame - 1) * stepSize + hsize_t(cKFi)];
                                    fi[hcKFi] = dataFi[h][(frame - 1) * stepSize + hsize_t(cKFi)];
                                } else {
                                    // Read coefficient
                                    k[hcKFi] = dataK[h][frame * stepSize + hsize_t(cKFi)];
                                    fi[hcKFi] = dataFi[h][frame * stepSize + hsize_t(cKFi)];
                                }
                            }
                        }

                        // Compute new point value
                        data[cKFi] = 0;
                        Helper::floatC i(0, -1);
                        for (hsize_t h = 0; h < harmonics; h++) {
                            hsize_t hcKFi = stepSize * h + hsize_t(cKFi);
                            data[cKFi] += real(k[hcKFi] * std::exp(i * fi[hcKFi]) * bE[h * bSize + p]) + real(lastK[hcKFi] * std::exp(i * lastFi[hcKFi]) * bE_1[h * bSize + p]);
                        }
                        // Min/max values
                        HDF5Helper::checkOrSetMinMaxValue(first, minV, maxV, data[cKFi]);
                    }

                    double t1 = HDF5Helper::getTime();
                    std::cout << "Time of the one step decoding: " << (t1 - t0) << " ms; \t" << std::endl;

                    std::cout << "Saving step " << frameDst * oSize + p << " ... ";
                    if (dims.getLength() == 4) { // 4D dataset
                        dstDataset->writeDataset(HDF5Helper::HDF5Vector4D(frameDst * oSize + p, 0, 0, 0), HDF5Helper::HDF5Vector4D(1, dims[1], dims[2], dims[3]), data, true);
                    } else if (dims.getLength() == 3) {
                        dstDataset->writeDataset(HDF5Helper::HDF5Vector3D(0, frameDst * oSize + p, 0), HDF5Helper::HDF5Vector3D(1, 1, dims[2]), data, true);
                    }
                    std::cout << "saved" << std::endl;
                }
                frameDst++;
                std::cout << "encoded" << std::endl;
            }

            #pragma omp parallel for
            for (hssize_t h = 0; h < hssize_t(harmonics); h++) {
                delete[] dataK[h];
                delete[] dataFi[h];
            }
            delete[] dataK;
            delete[] dataFi;
        }

        // Delete buffers
        delete[] data;
        delete[] k;
        delete[] fi;

    } else {
        // Not implemented yet
        std::cout << "Not implemented yet" << std::endl;
        delete[] b;
        delete[] e;
        delete[] bE;
        delete[] bE_1;
        hDF5OutputFile->closeDataset(dstDataset);
        return;
    }

    // Delete some memory
    delete[] b;
    delete[] e;
    delete[] bE;
    delete[] bE_1;

    // Copy attributes
    copyAttributes(srcDatasetsFi.at(0), dstDataset);

    dstDataset->removeAttribute(HDF5Helper::C_HARMONIC_ATTR, false);
    dstDataset->setAttribute(HDF5Helper::C_HARMONICS_ATTR, harmonics);

    // Set min/max values
    dstDataset->setAttribute(HDF5Helper::MIN_ATTR, minV);
    dstDataset->setAttribute(HDF5Helper::MAX_ATTR, maxV);
    dstDataset->setAttribute(HDF5Helper::C_TYPE_ATTR, "d");

    double t1 = HDF5Helper::getTime();
    std::cout << "Time of the whole dataset compression: " << (t1 - t0) << " ms; \t" << std::endl;

    hDF5OutputFile->closeDataset(dstDataset);
}

void Processing::subtractDatasets(HDF5Helper::HDF5Dataset *datasetOriginal, HDF5Helper::HDF5Dataset *datasetDecoded)
{
    HDF5Helper::HDF5Vector outputDims = datasetOriginal->getDims();
    HDF5Helper::HDF5Vector outputChunkDims = datasetOriginal->getChunkDims();

    std::string srcName = datasetDecoded->readAttributeS(HDF5Helper::SRC_DATASET_NAME_ATTR, false);
    hDF5OutputFile->createDatasetF(srcName + "_s", outputDims, outputChunkDims, true);
    HDF5Helper::HDF5Dataset *dstDataset = hDF5OutputFile->openDataset(srcName + "_s");

    // Variables for block reading
    float *dataO = 0;
    float *dataD = 0;
    HDF5Helper::HDF5Vector offset;
    HDF5Helper::HDF5Vector count;
    float maxV = 0, minV = 0;
    bool first = true;
    float sum = 0.0f;
    float sum2 = 0.0f;

    datasetDecoded->setNumberOfElmsToLoad(datasetOriginal->getNumberOfElmsToLoad());
    // Reading and substraction
    for (hsize_t i = 0; i < datasetOriginal->getNumberOfBlocks(); i++) {
        // First read decoded -> it is larger
        datasetDecoded->readBlock(i, offset, count, dataD);
        datasetOriginal->readBlock(i, offset, count, dataO);
        // The count is from original dataset and is smaller for last step than is in the decoded dataset

        #pragma omp parallel for reduction(+ : sum) reduction(+ : sum2)
        for (hssize_t i = 0; i < hssize_t(count.getSize()); i++) {
            dataD[i] = dataO[i] - dataD[i];
            sum += abs(dataD[i]);
            sum2 += (dataD[i] * dataD[i]);
            // Min/max values
            HDF5Helper::checkOrSetMinMaxValue(first, minV, maxV, dataD[i]);
        }

        dstDataset->writeDataset(offset, count, dataD);
        delete[] dataD;
        delete[] dataO;
    }

    // Copy attributes
    copyAttributes(datasetDecoded, dstDataset);

    float minVG;
    float maxVG;
    datasetOriginal->getGlobalMinValue(minVG);
    datasetOriginal->getGlobalMaxValue(maxVG);
    float maxError = std::max(abs(minV), abs(maxV));
    float meanError = sum / float(outputDims.getSize());
    float maxValue = std::max(abs(minVG), abs(maxVG));

    // Set min/max values
    dstDataset->setAttribute(HDF5Helper::MIN_ATTR, minV);
    dstDataset->setAttribute(HDF5Helper::MAX_ATTR, maxV);
    dstDataset->setAttribute(HDF5Helper::C_TYPE_ATTR, "s");
    dstDataset->setAttribute("mean_error", (meanError / maxValue) * 100);
    dstDataset->setAttribute("mean_error_value", meanError);
    dstDataset->setAttribute("max_error", (maxError / maxValue) * 100);
    dstDataset->setAttribute("max_error_value", maxError);
    dstDataset->setAttribute("mean_squared_error", sum2 / float(outputDims.getSize()));

    std::cout << std::endl;
    std::cout << "Mean error: " << (meanError / maxValue) * 100 << " %" << std::endl;
    std::cout << "Max error:  " << (maxError / maxValue) * 100  << " %" << std::endl;
    std::cout << "MSE:        " << sum2 / float(outputDims.getSize()) << std::endl;
    std::cout << std::endl;


    hDF5OutputFile->closeDataset(dstDataset);
}

void Processing::copyAttributes(HDF5Helper::HDF5Dataset *srcDataset, HDF5Helper::HDF5Dataset *dstDataset)
{
    for (hsize_t i = 0; i < srcDataset->getNumAttrs(); i++) {
        HDF5Helper::HDF5Attribute *attr = srcDataset->getAttribute(i);
        dstDataset->setAttribute(attr, false);
        delete attr;
    }
}

void Processing::resize2D(float *dataSrc, float *dataDst, unsigned int srcWidth, unsigned int srcHeight, unsigned int dstWidth, unsigned int dstHeight)
{
    resize2D(dataSrc, dataDst, static_cast<hsize_t>(srcWidth), static_cast<hsize_t>(srcHeight), static_cast<hsize_t>(dstWidth), static_cast<hsize_t>(dstHeight));
}

void Processing::resize2D(float *dataSrc, float *dataDst, hsize_t srcWidth, hsize_t srcHeight, hsize_t dstWidth, hsize_t dstHeight)
{
    float scaleWidth = static_cast<float>(dstWidth) / srcWidth;
    float scaleHeight = static_cast<float>(dstHeight) / srcHeight;

    #pragma omp parallel for
    for (hssize_t y = 0; y < hssize_t(dstHeight); y++) {
        #pragma omp parallel for
        for (hssize_t x = 0; x < hssize_t(dstWidth); x++) {

            //float newX = static_cast<float>(x) / scaleWidth;
            //float newY = static_cast<float>(y) / scaleHeight;
            //hsize_t left = static_cast<hsize_t>(floor(newX));
            //hsize_t right = static_cast<hsize_t>(ceil(newX));
            //hsize_t top = static_cast<hsize_t>(floor(newY));
            //hsize_t bottom = static_cast<hsize_t>(ceil(newY));
            //float wX = ceil(newX) - newX;
            //float wY = ceil(newY) - newY;

            //dataSrc[x, y] = dataSrc[x + y * w];

            //float topL = dataSrc[left + top * w] * wX + dataSrc[right + top * w] * (1 - wX);
            //float bottomL = dataSrc[left + bottom * w] * wX + dataSrc[right + bottom * w] * (1 - wX);

            //dataDst[ceil(x / s), ceil(y / s)] = dataSrc[x, y];
            //dataDst[x + y * wDst] = topL * wY + bottomL  * (1 - wY);

            // Nearest-neighbour interpolation
            float newX = static_cast<float>(x) / scaleWidth;
            float newY = static_cast<float>(y) / scaleHeight;
            hsize_t xSrc = Helper::round(newX);
            hsize_t ySrc = Helper::round(newY);
            dataDst[hsize_t(x) + hsize_t(y) * dstWidth] = dataSrc[xSrc + ySrc * srcWidth];
        }
    }
}

void Processing::resize3D(float *dataSrc, float *dataDst, unsigned int srcWidth, unsigned int srcHeight, unsigned int srcDepth, unsigned int dstWidth, unsigned int dstHeight, unsigned int dstDepth)
{
    resize3D(dataSrc, dataDst, static_cast<hsize_t>(srcWidth), static_cast<hsize_t>(srcHeight), static_cast<hsize_t>(srcDepth), static_cast<hsize_t>(dstWidth), static_cast<hsize_t>(dstHeight), static_cast<hsize_t>(dstDepth));
}

void Processing::resize3D(float *dataSrc, float *dataDst, hsize_t srcWidth, hsize_t srcHeight, hsize_t srcDepth, hsize_t dstWidth, hsize_t dstHeight, hsize_t dstDepth)
{
    float scaleWidth = static_cast<float>(dstWidth) / srcWidth;
    float scaleHeight = static_cast<float>(dstHeight) / srcHeight;
    float scaleDepth = static_cast<float>(dstDepth) / srcDepth;

    #pragma omp parallel for
    for (hssize_t z = 0; z < hssize_t(dstDepth); z++) {
        #pragma omp parallel for
        for (hssize_t y = 0; y < hssize_t(dstHeight); y++) {
            #pragma omp parallel for
            for (hssize_t x = 0; x < hssize_t(dstWidth); x++) {
                // Nearest-neighbour interpolation
                float newX = static_cast<float>(x) / scaleWidth;
                float newY = static_cast<float>(y) / scaleHeight;
                float newZ = static_cast<float>(z) / scaleDepth;
                hsize_t xSrc = Helper::round(newX);
                hsize_t ySrc = Helper::round(newY);
                hsize_t zSrc = Helper::round(newZ);
                dataDst[hsize_t(x) + hsize_t(y) * dstWidth + hsize_t(z) * dstWidth * dstHeight] = dataSrc[xSrc + ySrc * srcWidth + zSrc * srcWidth * srcHeight];
            }
        }
    }
}
