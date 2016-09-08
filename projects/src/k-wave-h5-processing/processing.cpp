/**
 * @file        processing.cpp
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.0
 * @date        8  September 2016 (created)
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
        if (dtsForPcs->getDtsMaskType().empty() && dtsForPcs->getGroupsCuboidType().empty()) {
            std::cout << "No datasets or groups for reshape in simuation output file" << std::endl;
        } else {
            // For every mask type dataset
            if (!dtsForPcs->getDtsMaskType().empty() && dtsForPcs->getSensorMaskIndexDataset()) {

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

                HDF5Helper::MapOfDatasets::iterator it;
                HDF5Helper::MapOfDatasets map;

                map = dtsForPcs->getDtsMaskType();
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

                    dstDataset->setAttribute("positionZ", min.z());
                    dstDataset->setAttribute("positionY", min.y());
                    dstDataset->setAttribute("positionX", min.x());

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

                    dstDataset->setAttribute("min", minVFG);
                    dstDataset->setAttribute("max", maxVFG);

                    if (useTmpFlag) {
                        delete[] tmpData;
                        delete[] sensorMaskData;
                    }
                }
            }

            // For cuboid type dataset
            if (!dtsForPcs->getGroupsCuboidType().empty() && dtsForPcs->getSensorMaskCornersDataset()) {

                HDF5Helper::MapOfGroups::iterator it;
                HDF5Helper::MapOfGroups map;

                HDF5Helper::HDF5Dataset *sensorMaskCornersDataset = dtsForPcs->getSensorMaskCornersDataset();
                hsize_t *data;
                sensorMaskCornersDataset->readFullDataset(data);

                // For every mask type dataset
                map = dtsForPcs->getGroupsCuboidType();
                for (it = map.begin(); it != map.end(); ++it) {
                    HDF5Helper::HDF5Group *group = it->second;
                    hDF5OutputFile->createGroup(group->getName(), true);

                    hsize_t count = group->getNumObjs();
                    // For every cuboid
                    for (hsize_t i = 0; i < count; i++) {
                        HDF5Helper::HDF5Dataset *dataset = group->openDataset(i);

                        // Copy to new file
                        if (hDF5OutputFile != dataset->getFile()) {
                            changeChunksOfDataset(dataset);
                            dataset = hDF5OutputFile->openDataset(dataset->getName());
                        }

                        dataset->findAndSetGlobalMinAndMaxValue();
                        dataset->setAttribute("positionZ", data[i * 6 + 0] - 1);
                        dataset->setAttribute("positionY", data[i * 6 + 1] - 1);
                        dataset->setAttribute("positionX", data[i * 6 + 2] - 1);
                    }
                }
                delete[] data;
            }
        }
    } catch(std::exception &e) {
        std::cerr << e.what() << std::endl;
        std::exit(EXIT_FAILURE);
    }
}

void Processing::changeChunks()
{
    try {
        if (dtsForPcs->getDts3DType().empty() && dtsForPcs->getDts3DTypeDsp().empty()
                && dtsForPcs->getGroupsCuboidTypeAttr().empty() && dtsForPcs->getGroupsCuboidType().empty()) {
            std::cout << "No dataset for change chunks in simulation output file" << std::endl;
        } else {
            HDF5Helper::MapOfDatasets::iterator it;
            HDF5Helper::MapOfDatasets map;
            // For every 3D type dataset
            map = dtsForPcs->getDts3DType();
            for (it = map.begin(); it != map.end(); ++it) {
                HDF5Helper::HDF5Dataset *srcDataset = it->second;
                changeChunksOfDataset(srcDataset);
            }

            // For every 3D type downsampled dataset
            map = dtsForPcs->getDts3DTypeDsp();
            for (it = map.begin(); it != map.end(); ++it) {
                HDF5Helper::HDF5Dataset *srcDataset = it->second;
                changeChunksOfDataset(srcDataset);
            }

            HDF5Helper::MapOfGroups::iterator itG;
            HDF5Helper::MapOfGroups mapG;
            // For every group cuboid attr type
            mapG = dtsForPcs->getGroupsCuboidTypeAttr();
            for (itG = mapG.begin(); itG != mapG.end(); ++itG) {
                HDF5Helper::HDF5Group *srcGroup = itG->second;

                hDF5OutputFile->createGroup(srcGroup->getName(), true);

                for (hsize_t i = 0; i < srcGroup->getNumObjs(); i++) {
                    HDF5Helper::HDF5Dataset *srcDataset = srcGroup->openDataset(i);
                    changeChunksOfDataset(srcDataset);
                }
            }

            // For every group cuboid type
            mapG = dtsForPcs->getGroupsCuboidType();
            for (itG = mapG.begin(); itG != mapG.end(); ++itG) {
                HDF5Helper::HDF5Group *srcGroup = itG->second;

                hDF5OutputFile->createGroup(srcGroup->getName(), true);

                for (hsize_t i = 0; i < srcGroup->getNumObjs(); i++) {
                    HDF5Helper::HDF5Dataset *srcDataset = srcGroup->openDataset(i);
                    changeChunksOfDataset(srcDataset);
                }
            }
        }
    } catch(std::exception &e) {
        std::cerr << e.what() << std::endl;
        std::exit(EXIT_FAILURE);
    }
}

void Processing::donwsampling()
{
    try {
        HDF5Helper::HDF5Vector3D dims = dtsForPcs->getNDims();
        // Check number of datasets for downsamling
        if (dtsForPcs->getDts3DType().empty() && dtsForPcs->getGroupsCuboidTypeAttr().empty()) {
            std::cout << "No dataset for downsampling in simulation output file" << std::endl;
        // Check current size -> is greater?
        } else if (std::max(std::max(dims.z(), dims.y()), dims.x()) <= settings->getMaxSize()) {
            std::cout << "No dataset for downsampling - max(nZ, nY, nX) == " + std::to_string(std::max(std::max(dims.z(), dims.y()), dims.x())) + " <= " + std::to_string(settings->getMaxSize()) << std::endl;
        } else {
            HDF5Helper::HDF5Vector3D chunkSize;
            HDF5Helper::HDF5Vector3D dimsDst;
            double ratio;

            // Get max dimension
            hsize_t nMax = std::max(std::max(dims.z(), dims.y()), dims.x());
            // Compute ratio
            ratio = static_cast<double>(settings->getMaxSize()) / nMax;

            // Compute new size
            computeDstDims(dims, ratio, dimsDst, chunkSize, settings);

            // For every 3D type dataset
            HDF5Helper::MapOfDatasets map = dtsForPcs->getDts3DType();
            HDF5Helper::MapOfDatasets::iterator it;
            for (it = map.begin(); it != map.end(); ++it) {
                HDF5Helper::HDF5Dataset *srcDataset = it->second;

                hDF5OutputFile->createDatasetF(srcDataset->getName() + "_" + std::to_string(settings->getMaxSize()), dimsDst, chunkSize, true);
                HDF5Helper::HDF5Dataset *dstDatasetFinal = hDF5OutputFile->openDataset(srcDataset->getName() + "_" + std::to_string(settings->getMaxSize()));

                // Downsampling
                resamplingOfDataset3D(srcDataset, dims, dimsDst, dstDatasetFinal);

                dstDatasetFinal->findAndSetGlobalMinAndMaxValue();

                // Save attributes
                dstDatasetFinal->setAttribute("dwnsmpl", settings->getMaxSize());
                dstDatasetFinal->setAttribute("src_dataset_name", srcDataset->getName());
                dstDatasetFinal->setAttribute("src_dataset_id", srcDataset->getId());

                // Filter or add to map
                std::list<std::string> names = settings->getNames();
                if (!settings->getFlagNames() || std::find(names.begin(), names.end(), dstDatasetFinal->getName()) != names.end())
                    dtsForPcs->getDts3DTypeDsp().insert(HDF5Helper::PairOfDatasets(dstDatasetFinal->getName(), dstDatasetFinal));
            }

            HDF5Helper::MapOfGroups mapG = dtsForPcs->getGroupsCuboidTypeAttr();
            HDF5Helper::MapOfGroups::iterator itG;
            for (itG = mapG.begin(); itG != mapG.end(); ++itG) {
                HDF5Helper::HDF5Group *srcGroup = itG->second;

                hDF5OutputFile->createGroup(srcGroup->getName() + "_" + std::to_string(settings->getMaxSize()), true);
                HDF5Helper::HDF5Group *group = hDF5OutputFile->openGroup(srcGroup->getName() + "_" + std::to_string(settings->getMaxSize()));

                for (hsize_t i = 0; i < srcGroup->getNumObjs(); i++) {
                    HDF5Helper::HDF5Dataset *srcDataset = srcGroup->openDataset(i);

                    HDF5Helper::HDF5Vector3D dims = srcDataset->getDims();
                    HDF5Helper::HDF5Vector4D dims4D = srcDataset->getDims();
                    HDF5Helper::HDF5Vector3D chunkSize;
                    HDF5Helper::HDF5Vector4D chunkSize4D;
                    HDF5Helper::HDF5Vector3D dimsDst;
                    HDF5Helper::HDF5Vector4D dimsDst4D;

                    computeDstDims(dims, ratio, dimsDst, chunkSize, settings);

                    chunkSize4D = HDF5Helper::HDF5Vector4D(settings->getMaxChunkSize(), chunkSize);
                    dimsDst4D = HDF5Helper::HDF5Vector4D(dims4D.w(), dimsDst);

                    std::cout << "chunks: " << chunkSize4D << " new size: " << dimsDst4D << std::endl;

                    group->createDatasetF(srcDataset->getOnlyName(), dimsDst4D, chunkSize4D, true);
                    HDF5Helper::HDF5Dataset *dstDatasetFinal = group->openDataset(srcDataset->getOnlyName());

                    resamplingOfDataset4D(srcDataset, dims4D, dimsDst4D, dstDatasetFinal);

                    dstDatasetFinal->findAndSetGlobalMinAndMaxValue();

                    // Save attributes
                    group->setAttribute("dwnsmpl", settings->getMaxSize());
                    group->setAttribute("src_group_name", srcGroup->getName());
                    group->setAttribute("src_group_id", srcGroup->getId());

                    hsize_t z = Helper::round(srcDataset->readAttributeI("positionZ") * ratio);
                    hsize_t y = Helper::round(srcDataset->readAttributeI("positionY") * ratio);
                    hsize_t x = Helper::round(srcDataset->readAttributeI("positionX") * ratio);

                    dstDatasetFinal->setAttribute("positionZ", z);
                    dstDatasetFinal->setAttribute("positionY", y);
                    dstDatasetFinal->setAttribute("positionX", x);

                    // Filter or add to map
                    std::list<std::string> names = settings->getNames();
                    if (!settings->getFlagNames() || std::find(names.begin(), names.end(), dstDatasetFinal->getName()) != names.end())
                        dtsForPcs->getGroupsCuboidTypeAttrDsp().insert(HDF5Helper::PairOfGroups(group->getName(), group));
                }
            }
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

void Processing::computeDstDims(HDF5Helper::HDF5Vector3D dimsSrc, double ratio, HDF5Helper::HDF5Vector3D &dimsDst, HDF5Helper::HDF5Vector3D &chunkSize, Settings *settings)
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
    dstDataset->setAttribute("min", minVG);
    dstDataset->setAttribute("max", maxVG);

    double t1 = HDF5Helper::getTime();
    std::cout << "Time of changing chunks of the whole dataset: " << (t1 - t0) << " ms; \t" << std::endl;

    hDF5OutputFile->closeDataset(dstDataset->getName());
}

void Processing::resamplingOfDataset3D(HDF5Helper::HDF5Dataset *srcDataset, HDF5Helper::HDF5Vector3D dimsSrc, HDF5Helper::HDF5Vector3D dimsDst, HDF5Helper::HDF5Dataset *dstDatasetFinal)
{
    double t0 = HDF5Helper::getTime();

    // Tmp has original Z dimension (hDF5SimOutputFile->getNZ())
    HDF5Helper::HDF5Vector3D newTmpDatasetSize(dimsSrc.z(), dimsDst.y(), dimsDst.x());

    // Create temp file and dataset
    HDF5Helper::File *tmpFile = new HDF5Helper::File("tmp.h5", HDF5Helper::File::CREATE);
    tmpFile->createDatasetF("tmp", newTmpDatasetSize, HDF5Helper::HDF5Vector3D(dimsSrc.z(), 1, dimsDst.x()));
    HDF5Helper::HDF5Dataset *tmpDataset = tmpFile->openDataset("tmp");

    float *srcData = 0;
    float *dstData = 0;

    // First 2D slices in XY plane
    for (unsigned int z = 0; z < dimsSrc.z(); z++) {
        srcDataset->readDataset(HDF5Helper::HDF5Vector3D(z, 0, 0), HDF5Helper::HDF5Vector3D(1, dimsSrc.y(), dimsSrc.x()), srcData);
        dstData = new float[dimsDst.x() * dimsDst.y()];
        resize(srcData, dstData, dimsSrc.x(), dimsSrc.y(), dimsDst.x(), dimsDst.y());
        tmpDataset->writeDataset(HDF5Helper::HDF5Vector3D(z, 0, 0), HDF5Helper::HDF5Vector3D(1, dimsDst.y(), dimsDst.x()), dstData, true);
        delete[] srcData;
        delete[] dstData;
    }

    // and after 2d slices XZ plane
    for (unsigned int y = 0; y < dimsDst.y(); y++) {
        tmpDataset->readDataset(HDF5Helper::HDF5Vector3D(0, y, 0), HDF5Helper::HDF5Vector3D(dimsSrc.z(), 1, dimsDst.x()), srcData);
        dstData = new float[dimsDst.x() * dimsDst.z()];
        resize(srcData, dstData, dimsDst.x(), dimsSrc.z(), dimsDst.x(), dimsDst.z());
        dstDatasetFinal->writeDataset(HDF5Helper::HDF5Vector3D(0, y, 0), HDF5Helper::HDF5Vector3D(dimsDst.z(), 1, dimsDst.x()), dstData, true);
        delete[] srcData;
        delete[] dstData;
    }

    delete tmpFile;
    remove("tmp.h5");

    double t1 = HDF5Helper::getTime();
    std::cout << "Time of resampling of the whole 3D dataset: " << (t1 - t0) << " ms; \t" << std::endl;
}

void Processing::resamplingOfDataset4D(HDF5Helper::HDF5Dataset *srcDataset, HDF5Helper::HDF5Vector4D dimsSrc, HDF5Helper::HDF5Vector4D dimsDst, HDF5Helper::HDF5Dataset *dstDatasetFinal)
{
    double t0 = HDF5Helper::getTime();

    // Tmp has original Z dimension (hDF5SimOutputFile->getNZ())
    HDF5Helper::HDF5Vector3D newTmpDatasetSize(dimsSrc.z(), dimsDst.y(), dimsDst.x());

    // Create temp file and dataset
    HDF5Helper::File *tmpFile = new HDF5Helper::File("tmp.h5", HDF5Helper::File::CREATE);
    tmpFile->createDatasetF("tmp", newTmpDatasetSize, HDF5Helper::HDF5Vector3D(dimsSrc.z(), 1, dimsDst.x()));
    HDF5Helper::HDF5Dataset *tmpDataset = tmpFile->openDataset("tmp");

    float *srcData = 0;
    float *dstData = 0;

    for (unsigned int t = 0; t < dimsSrc.w(); t++) {
        // First 2D slices in XY plane
        for (unsigned int z = 0; z < dimsSrc.z(); z++) {
            srcDataset->readDataset(HDF5Helper::HDF5Vector4D(t, z, 0, 0), HDF5Helper::HDF5Vector4D(1, 1, dimsSrc.y(), dimsSrc.x()), srcData);
            dstData = new float[dimsDst.x() * dimsDst.y()];
            resize(srcData, dstData, dimsSrc.x(), dimsSrc.y(), dimsDst.x(), dimsDst.y());
            tmpDataset->writeDataset(HDF5Helper::HDF5Vector3D(z, 0, 0), HDF5Helper::HDF5Vector3D(1, dimsDst.y(), dimsDst.x()), dstData, true);
            delete[] srcData;
            delete[] dstData;
        }

        // and after 2d slices XZ plane
        for (unsigned int y = 0; y < dimsDst.y(); y++) {
            tmpDataset->readDataset(HDF5Helper::HDF5Vector3D(0, y, 0), HDF5Helper::HDF5Vector3D(dimsSrc.z(), 1, dimsDst.x()), srcData);
            dstData = new float[dimsDst.x() * dimsDst.z()];
            resize(srcData, dstData, dimsDst.x(), dimsSrc.z(), dimsDst.x(), dimsDst.z());
            dstDatasetFinal->writeDataset(HDF5Helper::HDF5Vector4D(t, 0, y, 0), HDF5Helper::HDF5Vector4D(1, dimsDst.z(), 1, dimsDst.x()), dstData, true);
            delete[] srcData;
            delete[] dstData;
        }
    }

    delete tmpFile;
    remove("tmp.h5");

    double t1 = HDF5Helper::getTime();
    std::cout << "Time of resampling of the whole 4D dataset: " << (t1 - t0) << " ms; \t" << std::endl;
}

void Processing::copyAttributes(HDF5Helper::HDF5Dataset *srcDataset, HDF5Helper::HDF5Dataset *dstDataset)
{
    for (unsigned int i = 0; i < srcDataset->getNumAttrs(); i++) {
        HDF5Helper::HDF5Attribute *attr = srcDataset->getAttribute(i);
        dstDataset->setAttribute(attr);
        delete attr;
    }
}

void Processing::resize(float *dataSrc, float *dataDst, unsigned int wSrc, unsigned int hSrc, unsigned int wDst, unsigned int hDst)
{
    resize(dataSrc, dataDst, static_cast<hsize_t>(wSrc), static_cast<hsize_t>(hSrc), static_cast<hsize_t>(wDst), static_cast<hsize_t>(hDst));
}

void Processing::resize(float *dataSrc, float *dataDst, hsize_t wSrc, hsize_t hSrc, hsize_t wDst, hsize_t hDst)
{
    float scaleWidth = static_cast<float>(wDst) / wSrc;
    float scaleHeight = static_cast<float>(hDst) / hSrc;

    for (unsigned int y = 0; y < hDst; y++) {
        for (unsigned int x = 0; x < wDst; x++) {

            float newX = static_cast<float>(x) / scaleWidth;
            float newY = static_cast<float>(y) / scaleHeight;
            //hsize_t left = static_cast<hsize_t>(floor(newX));
            //hsize_t right = static_cast<hsize_t>(ceil(newX));
            //hsize_t top = static_cast<hsize_t>(floor(newY));
            //hsize_t bottom = static_cast<hsize_t>(ceil(newY));
            //float wX = ceil(newX) - newX;
            //float wY = ceil(newY) - newY;

            //dataSrc[x, y] = dataSrc[x + y * w];

            //float topL = dataSrc[left + top * w] * wX + dataSrc[right + top * w] * (1 - wX);
            //float bottomL = dataSrc[left + bottom * w] * wX + dataSrc[right + bottom * w] * (1 - wX);

            hsize_t xSrc = Helper::round(newX);
            hsize_t ySrc = Helper::round(newY);

            //dataDst[ceil(x / s), ceil(y / s)] = dataSrc[x, y];
            //dataDst[x + y * wDst] = topL * wY + bottomL  * (1 - wY);
            dataDst[x + y * wDst] = dataSrc[xSrc + ySrc * wSrc];
        }
    }
}
