/**
 * @file        processing.cpp
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        8  September 2016 (created)
 *              3  November  2016 (updated)
 *
 * @brief       The implementation file containing processing functions.
 *
 * @license     This application is for preprocessing the HDF5 data created by the k-Wave toolbox - http://www.k-wave.org.
 *              k-Wave h5 processing is free software.
 *
 * @copyright   Copyright © 2016, Petr Kleparnik, VUT FIT Brno. All Rights Reserved.
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
            HDF5Helper::HDF5Vector4D chunkSize;
            chunkSize.w(1);
            chunkSize.z(std::min(settings->getMaxChunkSize(), max.z() - min.z() + 1));
            chunkSize.y(std::min(settings->getMaxChunkSize(), max.y() - min.y() + 1));
            chunkSize.x(std::min(settings->getMaxChunkSize(), max.x() - min.x() + 1));
            std::cout << "   new chunk size:\t" << chunkSize.w() << " x " << chunkSize.z() << " x " << chunkSize.y() << " x " << chunkSize.x() << std::endl;

            HDF5Helper::MapOfDatasets map = dtsForPcs->getDatasets(HDF5Helper::HDF5DatasetType::BASIC_MASK);
            HDF5Helper::MapOfDatasets::iterator it;
            for (it = map.begin(); it != map.end(); ++it) {
                HDF5Helper::HDF5Dataset *dataset = it->second;

                hsize_t steps = dataset->getDims()[1];

                // Compute dataset size
                HDF5Helper::HDF5Vector4D datasetSize(steps, max.z() - min.z() + 1, max.y() - min.y() + 1, max.x() - min.x() + 1);
                HDF5Helper::HDF5Vector4D stepSize(1, max.z() - min.z() + 1, max.y() - min.y() + 1, max.x() - min.x() + 1);
                std::cout << "   new dataset size:\t" << datasetSize.w() << " x " << datasetSize.z() << " x " << datasetSize.y() << " x " << datasetSize.x() << std::endl;

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
                if (datasetSize.z() * datasetSize.y() * datasetSize.x() <= dataset->getNumberOfElmsToLoad()) {
                    useTmpFlag = true;
                    tmpData = new float[datasetSize.z() * datasetSize.y() * datasetSize.x()];
                    // Read sensorMaskIndexDataset only once
                    sensorMaskIndexDataset->readBlock(0, offset, count, sensorMaskData);
                }

                // Create new group for dataset
                hDF5OutputFile->createGroup(dataset->getName(), true);
                HDF5Helper::HDF5Group *group = hDF5OutputFile->openGroup(dataset->getName());

                // Create dataset in group
                group->createDatasetF("0", datasetSize, chunkSize, true);
                dstDataset = group->openDataset("0");

                dstDataset->setAttribute(HDF5Helper::File::POSITION_Z_ATTR, min.z());
                dstDataset->setAttribute(HDF5Helper::File::POSITION_Y_ATTR, min.y());
                dstDataset->setAttribute(HDF5Helper::File::POSITION_X_ATTR, min.x());

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
                                    tmpData[(dstPos.z() - min.z()) * (datasetSize.y() * datasetSize.x()) + (dstPos.y() - min.y()) * datasetSize.x() + (dstPos.x() - min.x())] = data;
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

                dstDataset->setAttribute(HDF5Helper::File::MIN_ATTR, minVFG);
                dstDataset->setAttribute(HDF5Helper::File::MAX_ATTR, maxVFG);

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
            HDF5Helper::MapOfDatasets::iterator it;
            for (it = map.begin(); it != map.end(); ++it) {
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
                dataset->setAttribute(HDF5Helper::File::POSITION_Z_ATTR, data[i * 6 + 0] - 1);
                dataset->setAttribute(HDF5Helper::File::POSITION_Y_ATTR, data[i * 6 + 1] - 1);
                dataset->setAttribute(HDF5Helper::File::POSITION_X_ATTR, data[i * 6 + 2] - 1);
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
        HDF5Helper::MapOfDatasets::iterator it;
        HDF5Helper::MapOfDatasets map = dtsForPcs->getDatasets();
        HDF5Helper::HDF5Vector4D nDims4D = hDF5OutputFile->getNdims();
        hsize_t sensorMaskSize = dtsForPcs->getSensorMaskSize();
        int count = 0;
        for (it = map.begin(); it != map.end(); ++it) {
            HDF5Helper::HDF5Dataset *dataset = it->second;
            HDF5Helper::HDF5DatasetType datasetType = dataset->getType(nDims4D, sensorMaskSize);

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
            HDF5Helper::MapOfDatasets::iterator it;
            for (it = map.begin(); it != map.end(); ++it) {
                HDF5Helper::HDF5Dataset *dataset = it->second;
                resampleDataset(dataset);
            }
        } else if (!dtsForPcs->getDatasets(HDF5Helper::HDF5DatasetType::CUBOID).empty()) {
            // Cuboid datasets
            HDF5Helper::MapOfDatasets map = dtsForPcs->getDatasets(HDF5Helper::HDF5DatasetType::CUBOID);
            HDF5Helper::MapOfDatasets::iterator it;
            for (it = map.begin(); it != map.end(); ++it) {
                HDF5Helper::HDF5Dataset *dataset = it->second;
                resampleDataset(dataset);
            }
        } else if (!dtsForPcs->getDatasets(HDF5Helper::HDF5DatasetType::CUBOID_ATTR).empty()) {
            // Cuboid datasets with attributes
            HDF5Helper::MapOfDatasets map = dtsForPcs->getDatasets(HDF5Helper::HDF5DatasetType::CUBOID_ATTR);
            HDF5Helper::MapOfDatasets::iterator it;
            for (it = map.begin(); it != map.end(); ++it) {
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
        HDF5Helper::MapOfDatasets::iterator it;
        HDF5Helper::MapOfDatasets map = dtsForPcs->getDatasets();
        HDF5Helper::HDF5Vector4D nDims4D = hDF5OutputFile->getNdims();
        hsize_t sensorMaskSize = dtsForPcs->getSensorMaskSize();
        int count = 0;
        std::cout << "Compression with period: " << settings->getPeriod() << " steps" << std::endl;
        for (it = map.begin(); it != map.end(); ++it) {
            HDF5Helper::HDF5Dataset *dataset = it->second;
            HDF5Helper::HDF5DatasetType datasetType = dataset->getType(nDims4D, sensorMaskSize);

            if (datasetType == HDF5Helper::HDF5DatasetType::BASIC_MASK
                    || datasetType == HDF5Helper::HDF5DatasetType::CUBOID
                    || datasetType == HDF5Helper::HDF5DatasetType::CUBOID_ATTR
                    ) {
                compressDataset(dataset);
                count++;
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
        HDF5Helper::MapOfDatasets::iterator it;
        HDF5Helper::MapOfDatasets map = dtsForPcs->getDatasets();
        HDF5Helper::HDF5Vector4D nDims4D = hDF5OutputFile->getNdims();
        hsize_t sensorMaskSize = dtsForPcs->getSensorMaskSize();
        int count = 0;
        //std::cout << "Compression with period: " << settings->getPeriod() << " steps" << std::endl;
        for (it = map.begin(); it != map.end(); ++it) {
            HDF5Helper::HDF5Dataset *datasetFi = it->second;
            HDF5Helper::HDF5DatasetType datasetType = datasetFi->getType(nDims4D, sensorMaskSize);

            if (datasetType == HDF5Helper::HDF5DatasetType::FI_MASK
                    || datasetType == HDF5Helper::HDF5DatasetType::CUBOID_FI
                    || datasetType == HDF5Helper::HDF5DatasetType::CUBOID_ATTR_FI
                    || datasetType == HDF5Helper::HDF5DatasetType::CUBOID_DWNSMPL_FI
                    || datasetType == HDF5Helper::HDF5DatasetType::CUBOID_ATTR_DWNSMPL_FI
                    ) {

                std::string srcName = datasetFi->readAttributeS(HDF5Helper::File::SRC_DATASET_NAME_ATTR);
                std::string kName = srcName + "_k";
                if (map.find(kName) == map.end())
                    continue;

                HDF5Helper::HDF5Dataset *datasetK = map.at(kName);

                decompressDatasets(datasetFi, datasetK);
                count++;
            }
        }

        if (count == 0) {
            std::cout << "No datasets for decompression in simulation output file" << std::endl;
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

void Processing::computeDstDims(HDF5Helper::HDF5Vector3D dimsSrc, float ratio, HDF5Helper::HDF5Vector3D &dimsDst, HDF5Helper::HDF5Vector3D &chunkSize, Settings *settings)
{
    dimsDst.z(Helper::round(dimsSrc.z() * ratio));
    dimsDst.y(Helper::round(dimsSrc.y() * ratio));
    dimsDst.x(Helper::round(dimsSrc.x() * ratio));
    // Check < 1
    if (dimsDst.z() < 1) dimsDst.z(1);
    if (dimsDst.y() < 1) dimsDst.y(1);
    if (dimsDst.x() < 1) dimsDst.x(1);
    // Chunk size
    chunkSize.z(settings->getMaxChunkSize());
    chunkSize.y(settings->getMaxChunkSize());
    chunkSize.x(settings->getMaxChunkSize());
    if (chunkSize.z() > dimsDst.z()) chunkSize.z(dimsDst.z());
    if (chunkSize.y() > dimsDst.y()) chunkSize.y(dimsDst.y());
    if (chunkSize.x() > dimsDst.x()) chunkSize.x(dimsDst.x());
    std::cout << "   new size:\t" << dimsDst << std::endl;
}

void Processing::changeChunksOfDataset(HDF5Helper::HDF5Dataset *srcDataset)
{
    HDF5Helper::HDF5Vector dims = srcDataset->getDims();

    // Chunk size
    HDF5Helper::HDF5Vector chunkSize(dims.getLength(), 1);

    for (hsize_t i = 0; i < dims.getLength(); i++) {
        chunkSize[i] = settings->getMaxChunkSize();
        if (chunkSize[i] > dims[i]) chunkSize[i] = dims[i];
    }

    // Create dst dataset
    hDF5OutputFile->createDatasetF(srcDataset->getName(), dims, chunkSize, true);
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
    dstDataset->setAttribute(HDF5Helper::File::MIN_ATTR, minVG);
    dstDataset->setAttribute(HDF5Helper::File::MAX_ATTR, maxVG);

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
    HDF5Helper::HDF5Vector chunkSizeDst;
    HDF5Helper::HDF5Vector3D chunkSizeDst3D;

    // Compute ratio
    float ratio = float(settings->getMaxSize()) / std::max(std::max(dimsSrc3D.z(), dimsSrc3D.y()), dimsSrc3D.x());
    // Check downsampling size
    if (ratio >= 1) {
        std::cout << "Bad destination size for downsampling" << std::endl;
        return;
    }

    // Compute destination dims
    computeDstDims(dimsSrc3D, ratio, dimsDst3D, chunkSizeDst3D, settings);

    // For 4D datasets
    if (dimsSrc.getLength() == 4) {
        chunkSizeDst = HDF5Helper::HDF5Vector4D(settings->getMaxChunkSize(), chunkSizeDst3D); // Maybe better is (1, chunkSizeDst3D)
        dimsDst = HDF5Helper::HDF5Vector4D(HDF5Helper::HDF5Vector4D(dimsSrc).w(), dimsDst3D);
    } else { // 3D datasets
        chunkSizeDst = chunkSizeDst3D;
        dimsDst = dimsDst3D;
    }

    // Create dst dataset
    hDF5OutputFile->createDatasetF(srcDataset->getName() + "_" + std::to_string(settings->getMaxSize()), dimsDst, chunkSizeDst, true);
    HDF5Helper::HDF5Dataset *dstDataset = hDF5OutputFile->openDataset(srcDataset->getName() + "_" + std::to_string(settings->getMaxSize()));

    double t0 = HDF5Helper::getTime();

    // Tmp has original Z dimension (hDF5SimOutputFile->getNZ())
    HDF5Helper::HDF5Vector3D newTmpDatasetSize(dimsSrc3D.z(), dimsDst3D.y(), dimsDst3D.x());
    HDF5Helper::HDF5Vector3D newTmpDatasetChunkSize(dimsSrc3D.z(), 1, dimsDst3D.x());

    // Create temp file and dataset
    HDF5Helper::File *tmpFile = new HDF5Helper::File("tmp.h5", HDF5Helper::File::CREATE);
    tmpFile->createDatasetF("tmp", newTmpDatasetSize, newTmpDatasetChunkSize);
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
    dstDataset->setAttribute(HDF5Helper::File::SRC_SIZE_X_ATTR, dimsSrc3D.x());
    dstDataset->setAttribute(HDF5Helper::File::SRC_SIZE_Y_ATTR, dimsSrc3D.y());
    dstDataset->setAttribute(HDF5Helper::File::SRC_SIZE_Z_ATTR, dimsSrc3D.z());
    dstDataset->setAttribute(HDF5Helper::File::SRC_DATASET_NAME_ATTR, srcDataset->getName());

    if (srcDataset->hasAttribute(HDF5Helper::File::POSITION_Z_ATTR)
            && srcDataset->hasAttribute(HDF5Helper::File::POSITION_Z_ATTR)
            && srcDataset->hasAttribute(HDF5Helper::File::POSITION_Z_ATTR)
            && srcDataset->hasAttribute(HDF5Helper::File::POSITION_Z_ATTR)
            ) {
        hsize_t z = Helper::round(srcDataset->readAttributeI(HDF5Helper::File::POSITION_Z_ATTR) * ratio);
        hsize_t y = Helper::round(srcDataset->readAttributeI(HDF5Helper::File::POSITION_Y_ATTR) * ratio);
        hsize_t x = Helper::round(srcDataset->readAttributeI(HDF5Helper::File::POSITION_X_ATTR) * ratio);

        dstDataset->setAttribute(HDF5Helper::File::POSITION_Z_ATTR, z);
        dstDataset->setAttribute(HDF5Helper::File::POSITION_Y_ATTR, y);
        dstDataset->setAttribute(HDF5Helper::File::POSITION_X_ATTR, x);
    }
    double t1 = HDF5Helper::getTime();
    std::cout << "Time of resampling of the whole dataset: " << (t1 - t0) << " ms; \t" << std::endl;
}

void Processing::compressDataset(HDF5Helper::HDF5Dataset *srcDataset)
{
    // Only one coding parameter - multiple of overlap size
    hsize_t s = 1;

    // Overlap size
    hsize_t oSize = settings->getPeriod() * s;
    // Base size
    hsize_t bSize = oSize * 2 + 1;

    // Memory for helper functions data
    float *b = new float[bSize]();
    Helper::floatC *e = new Helper::floatC[bSize]();
    Helper::floatC *bE = new Helper::floatC[bSize]();
    Helper::floatC *bE_1 = new Helper::floatC[bSize]();

    // Generate basis function (window)
    Helper::triangular(oSize, b);  // Triangular window
    //Helper::hann(oSize, b);        // Hann window

    // Generate complex exponential functions
    Helper::floatC i(0,-1);
    for (hsize_t x = 0; x < bSize; x++) {
        e[x] = std::exp(i * (2.0f * float(M_PI) / settings->getPeriod()) * float(x));
    }

    for (hsize_t x = 0; x < bSize; x++) {
        bE[x] = b[x] * e[x];
        bE_1[x] = b[(x + oSize) % (bSize - 1)] * e[(x + oSize) % (bSize - 1)];
    }

    // Get dims
    HDF5Helper::HDF5Vector dims = srcDataset->getDims();
    HDF5Helper::HDF5Vector outputDims = dims;

    // Compute steps, step size and output dims
    hsize_t steps = 0;
    hsize_t outputSteps = 0;
    hsize_t stepSize = 0;
    if (dims.getLength() == 4) { // 4D dataset
        steps = HDF5Helper::HDF5Vector4D(dims).w();
        outputSteps = hsize_t(ceil(float(steps) / oSize));
        outputDims[0] = outputSteps;
        stepSize = outputDims[1] * outputDims[2] * outputDims[3];
    } else if (dims.getLength() == 3) { // 3D dataset (defined by sensor mask)
        steps = HDF5Helper::HDF5Vector3D(dims).y();
        outputSteps = hsize_t(ceil(float(steps) / oSize));
        outputDims[1] = outputSteps;
        stepSize = outputDims[2];
    } else { // Something wrong.
        std::cout << "Something wrong with dataset dims" << std::endl;
        return;
    }

    // Compute new chunk size
    HDF5Helper::HDF5Vector chunkSize(outputDims.getLength(), 1);
    for (hsize_t i = 0; i < outputDims.getLength(); i++) {
        chunkSize[i] = settings->getMaxChunkSize();
        if (chunkSize[i] > outputDims[i]) chunkSize[i] = outputDims[i];
    }

    if (dims.getLength() == 3) { // Original chunks layout
        chunkSize = outputDims;
        chunkSize[1] = 1;
    }

    std::cout << "steps:       " << steps << std::endl;
    std::cout << "outputSteps: " << outputSteps << std::endl;
    std::cout << "dims:        " << dims << std::endl;
    std::cout << "outputDims:  " << outputDims << std::endl;
    std::cout << "stepSize:    " << stepSize << std::endl;

    // Create dst dataset
    hDF5OutputFile->createDatasetF(srcDataset->getName() + "_k", outputDims, chunkSize, true);
    hDF5OutputFile->createDatasetF(srcDataset->getName() + "_fi", outputDims, chunkSize, true);
    HDF5Helper::HDF5Dataset *dstDatasetK = hDF5OutputFile->openDataset(srcDataset->getName() + "_k");
    HDF5Helper::HDF5Dataset *dstDatasetFi = hDF5OutputFile->openDataset(srcDataset->getName() + "_fi");

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

        hsize_t step = 0;
        hsize_t frame = 1;

        // Reading and compression
        for (hsize_t i = 0; i < srcDataset->getNumberOfBlocks(); i++) {
            srcDataset->readBlock(i, offset, count, data);

            float *dataK = new float[stepSize];
            float *dataFi = new float[stepSize];

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
            for (step = 0; step < stepsCount; step++) {
                //std::cout << "Step: " << step << std::endl;

                // Compute local index
                hsize_t stepLocal = step % (bSize - 1);

                // For every point
                for (hsize_t p = 0; p < stepSize; p++) {
                    // Correlation step
                    sCTmp1[p] += bE[stepLocal] * data[step * stepSize + p];
                    sCTmp2[p] += bE_1[stepLocal] * data[step * stepSize + p];

                    //std::cout << "step: " << step << std::endl;
                    //std::cout << "step * stepSize: " << step << std::endl;

                    // Check if we are at saving point
                    if (stepLocal == oSize - 1 || stepLocal == oSize * 2 - 1) {

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
                        sC = sC * 2.0f / float(oSize);

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
                            if (first) {
                                maxVK = k;
                                maxVFi = fi;
                                minVK = k;
                                minVFi = fi;
                                first = false;
                            } else {
                                if (minVK > k) minVK = k;
                                if (maxVK < k) maxVK = k;
                                if (minVFi > fi) minVFi = fi;
                                if (maxVFi < fi) maxVFi = fi;
                            }
                        }

                    }
                }

                // Increment frame
                if (stepLocal == oSize - 1 || stepLocal == oSize * 2 - 1) {
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
            }

            delete[] dataK;
            delete[] dataFi;

            delete[] data;
        }

        // Delete complex buffers
        delete[] sCTmp1;
        delete[] sCTmp2;

    } else {
        // Not implemented yet
        std::cout << "Not implemented yet" << std::endl;
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
    dstDatasetK->setAttribute(HDF5Helper::File::MIN_ATTR, minVK);
    dstDatasetK->setAttribute(HDF5Helper::File::MAX_ATTR, maxVK);
    dstDatasetK->setAttribute(HDF5Helper::File::SRC_DATASET_NAME_ATTR, srcDataset->getName());
    dstDatasetK->setAttribute(HDF5Helper::File::C_TYPE_ATTR, "k");
    dstDatasetK->setAttribute(HDF5Helper::File::C_PERIOD_ATTR, settings->getPeriod());
    dstDatasetK->setAttribute(HDF5Helper::File::C_S_ATTR, s);
    dstDatasetFi->setAttribute(HDF5Helper::File::MIN_ATTR, minVFi);
    dstDatasetFi->setAttribute(HDF5Helper::File::MAX_ATTR, maxVFi);
    dstDatasetFi->setAttribute(HDF5Helper::File::SRC_DATASET_NAME_ATTR, srcDataset->getName());
    dstDatasetFi->setAttribute(HDF5Helper::File::C_TYPE_ATTR, "fi");
    dstDatasetFi->setAttribute(HDF5Helper::File::C_PERIOD_ATTR, settings->getPeriod());
    dstDatasetFi->setAttribute(HDF5Helper::File::C_S_ATTR, s);

    double t1 = HDF5Helper::getTime();
    std::cout << "Time of the whole dataset compression: " << (t1 - t0) << " ms; \t" << std::endl;

    hDF5OutputFile->closeDataset(dstDatasetK);
    hDF5OutputFile->closeDataset(dstDatasetFi);
}

void Processing::decompressDatasets(HDF5Helper::HDF5Dataset *srcDatasetFi, HDF5Helper::HDF5Dataset *srcDatasetK)
{
    // Only one decoding parameter - multiple of overlap size
    hsize_t s = 1;
    if (srcDatasetFi->hasAttribute(HDF5Helper::File::C_S_ATTR))
        s = srcDatasetFi->readAttributeI(HDF5Helper::File::C_S_ATTR);
}

void Processing::copyAttributes(HDF5Helper::HDF5Dataset *srcDataset, HDF5Helper::HDF5Dataset *dstDataset)
{
    for (hsize_t i = 0; i < srcDataset->getNumAttrs(); i++) {
        HDF5Helper::HDF5Attribute *attr = srcDataset->getAttribute(i);
        dstDataset->setAttribute(attr);
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

    for (unsigned int y = 0; y < dstHeight; y++) {
        for (unsigned int x = 0; x < dstWidth; x++) {

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
            dataDst[x + y * dstWidth] = dataSrc[xSrc + ySrc * srcWidth];
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

    for (unsigned int z = 0; z < dstDepth; z++) {
        for (unsigned int y = 0; y < dstHeight; y++) {
            for (unsigned int x = 0; x < dstWidth; x++) {
                // Nearest-neighbour interpolation
                float newX = static_cast<float>(x) / scaleWidth;
                float newY = static_cast<float>(y) / scaleHeight;
                float newZ = static_cast<float>(z) / scaleDepth;
                hsize_t xSrc = Helper::round(newX);
                hsize_t ySrc = Helper::round(newY);
                hsize_t zSrc = Helper::round(newZ);
                dataDst[x + y * dstWidth + z * dstWidth * dstHeight] = dataSrc[xSrc + ySrc * srcWidth + zSrc * srcWidth * srcHeight];
            }
        }
    }
}
