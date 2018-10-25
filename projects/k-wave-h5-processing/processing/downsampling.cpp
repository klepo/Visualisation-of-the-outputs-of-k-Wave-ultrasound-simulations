/**
 * @file        downsampling.cpp
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        8  September 2016 (created) <br>
 *              25 October   2018 (updated)
 *
 * @brief       The implementation file containing Downsampling class definition.
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

#include "downsampling.h"

/**
 * @brief Creates Downsampling object
 * @param[in] outputFile Output file
 * @param[in] dtsForPcs Datasets for processing
 * @param[in] settings Processing settings
 */
Downsampling::Downsampling(H5Helper::File *outputFile, DtsForPcs *dtsForPcs, const Settings *settings)
    : Processing(outputFile, dtsForPcs, settings)
{
}

/**
 * @brief Executes processing
 */
void Downsampling::execute()
{
    std::vector<H5Helper::DatasetType> types = {
        H5Helper::DatasetType::BASIC_3D,
        H5Helper::DatasetType::RESHAPED_3D,
        H5Helper::DatasetType::CUBOID,
        H5Helper::DatasetType::CUBOID_C,
        H5Helper::DatasetType::CUBOID_D,
        H5Helper::DatasetType::CUBOID_S,
        H5Helper::DatasetType::CUBOID_ATTR,
        H5Helper::DatasetType::CUBOID_ATTR_C,
        H5Helper::DatasetType::CUBOID_ATTR_D,
        H5Helper::DatasetType::CUBOID_ATTR_S,
    };

    try {
        H5Helper::MapOfDatasets map = getDtsForPcs()->getDatasets();

        int count = 0;
        for (H5Helper::MapOfDatasetsIt it = map.begin(); it != map.end(); ++it) {
            H5Helper::Dataset *dataset = it->second;
            H5Helper::DatasetType datasetType = dataset->getType();

            if (checkDatasetType(datasetType, types)) {
                Helper::printDebugMsg("Downsampling of dataset " + dataset->getName());
                resampleDataset(dataset, getSettings()->getFlagLog());
                count++;
                Helper::printDebugMsg("Downsampling of dataset " + dataset->getName() + " done");
            }
        }
        if (count == 0) {
            Helper::printErrorMsg("No datasets for downsampling in simulation output file");
        }
    } catch(std::exception &e) {
        Helper::printErrorMsg(e.what());
        std::exit(EXIT_FAILURE);
    }
}

/**
 * @brief Resamples dataset
 * @param[in] srcDataset Source dataset
 * @param[in] log Logging flag (optional)
 */
void Downsampling::resampleDataset(H5Helper::Dataset *srcDataset, bool log)
{
    // Dims
    H5Helper::Vector dimsSrc = srcDataset->getDims();
    H5Helper::Vector3D dimsSrc3D = dimsSrc;
    H5Helper::Vector dimsDst;
    H5Helper::Vector3D dimsDst3D;
    H5Helper::Vector chunkDimsDst;
    H5Helper::Vector3D chunkDimsDst3D;

    Helper::printDebugTwoColumns2S("Size", dimsSrc);

    // Compute ratio
    hsize_t maxDim = std::max(std::max(dimsSrc3D.z(), dimsSrc3D.y()), dimsSrc3D.x());
    float ratio = float(getSettings()->getMaxSize()) / maxDim;
    // Check downsampling size
    if (ratio >= 1) {
        Helper::printErrorMsg("Bad destination size for downsampling (" + std::to_string(getSettings()->getMaxSize()) + " >= " + std::to_string(maxDim) + ")");
        return;
    }

    // Compute destination dims
    H5Helper::Vector3D maxChunkDims(1);
    maxChunkDims.x(getSettings()->getMaxChunkSizeX());
    maxChunkDims.y(getSettings()->getMaxChunkSizeY());
    maxChunkDims.z(getSettings()->getMaxChunkSizeZ());
    computeDstDims(dimsSrc3D, maxChunkDims, ratio, dimsDst3D, chunkDimsDst3D);

    // For 4D datasets
    if (dimsSrc.getLength() == 4) {
        chunkDimsDst = H5Helper::Vector4D(std::min(getSettings()->getMaxChunkSizeW(), H5Helper::Vector4D(dimsSrc).w()), chunkDimsDst3D); // or (getSettings()->getMaxChunkSize(), chunkSizeDst3D)?
        dimsDst = H5Helper::Vector4D(H5Helper::Vector4D(dimsSrc).w(), dimsDst3D);
    } else { // 3D datasets
        chunkDimsDst = chunkDimsDst3D;
        dimsDst = dimsDst3D;
    }

    Helper::printDebugTwoColumns2S("New size", dimsDst);

    // Create destination dataset
    getOutputFile()->createDatasetF(srcDataset->getName() + "_" + std::to_string(getSettings()->getMaxSize()), dimsDst, chunkDimsDst, true, log);
    H5Helper::Dataset *dstDataset = getOutputFile()->openDataset(srcDataset->getName() + "_" + std::to_string(getSettings()->getMaxSize()), log);

    double t0 = H5Helper::getTime();

    // Tmp has original Z dimension (simOutputFile->getNZ())
    H5Helper::Vector3D newTmpDatasetDims(dimsSrc3D.z(), dimsDst3D.y(), dimsDst3D.x());
    H5Helper::Vector3D newTmpDatasetChunkDims(dimsSrc3D.z(), 1, dimsDst3D.x());

    // Create temp file and dataset
    H5Helper::File *tmpFile = new H5Helper::File("tmp.h5", H5Helper::File::CREATE, log);
    tmpFile->createDatasetF("tmp", newTmpDatasetDims, newTmpDatasetChunkDims, true, log);
    H5Helper::Dataset *tmpDataset = tmpFile->openDataset("tmp", log);

    float *srcData = nullptr;
    float *dstData = nullptr;

    hsize_t steps = 1;
    if (dimsSrc.getLength() == 4)
        steps = H5Helper::Vector4D(dimsSrc).w();

    // If we have enough memory, resample full 3D dataset
    if (srcDataset->getNumberOfElmsToLoad() >= srcDataset->getSize()) {
        srcData = new float[dimsSrc3D.getSize()]();
        dstData = new float[dimsDst3D.getSize()]();
        for (unsigned int t = 0; t < steps; t++) {
            if (dimsSrc.getLength() == 4)
                srcDataset->readDataset(H5Helper::Vector4D(t, 0, 0, 0), H5Helper::Vector4D(1, dimsSrc3D.z(), dimsSrc3D.y(), dimsSrc3D.x()), srcData, log);
            else
                srcDataset->readDataset(srcData, log);
            resize3D(srcData, dstData, dimsSrc3D.x(), dimsSrc3D.y(), dimsSrc3D.z(), dimsDst3D.x(), dimsDst3D.y(), dimsDst3D.z());
            if (dimsSrc.getLength() == 4)
                dstDataset->writeDataset(H5Helper::Vector4D(t, 0, 0, 0), H5Helper::Vector4D(1, dimsDst3D.z(), dimsDst3D.y(), dimsDst3D.x()), dstData, log);
            else
                dstDataset->writeDataset(dstData, log);
        }
        delete[] srcData;
        srcData = nullptr;
        delete[] dstData;
        dstData = nullptr;
    } else {
        hsize_t maxSize = std::max(dimsSrc3D.x(), std::max(dimsSrc3D.y(), dimsSrc3D.z())) * std::max(dimsSrc3D.x(), std::max(dimsSrc3D.y(), dimsSrc3D.z()));
        srcData = new float[maxSize]();
        dstData = new float[maxSize]();
        for (unsigned int t = 0; t < steps; t++) {
            // First 2D slices in XY plane
            for (unsigned int z = 0; z < dimsSrc3D.z(); z++) {
                if (dimsSrc.getLength() == 4)
                    srcDataset->readDataset(H5Helper::Vector4D(t, z, 0, 0), H5Helper::Vector4D(1, 1, dimsSrc3D.y(), dimsSrc3D.x()), srcData, log);
                else
                    srcDataset->readDataset(H5Helper::Vector3D(z, 0, 0), H5Helper::Vector3D(1, dimsSrc3D.y(), dimsSrc3D.x()), srcData, log);
                //dstData = new float[dimsDst3D.x() * dimsDst3D.y()]();
                resize2D(srcData, dstData, dimsSrc3D.x(), dimsSrc3D.y(), dimsDst3D.x(), dimsDst3D.y());
                tmpDataset->writeDataset(H5Helper::Vector3D(z, 0, 0), H5Helper::Vector3D(1, dimsDst3D.y(), dimsDst3D.x()), dstData, log);
                //delete[] srcData;
                //delete[] dstData;
            }

            // and after 2D slices XZ plane
            for (unsigned int y = 0; y < dimsDst3D.y(); y++) {
                tmpDataset->readDataset(H5Helper::Vector3D(0, y, 0), H5Helper::Vector3D(dimsSrc3D.z(), 1, dimsDst3D.x()), srcData, log);
                //dstData = new float[dimsDst3D.x() * dimsDst3D.z()]();
                resize2D(srcData, dstData, dimsDst3D.x(), dimsSrc3D.z(), dimsDst3D.x(), dimsDst3D.z());
                if (dimsSrc.getLength() == 4)
                    dstDataset->writeDataset(H5Helper::Vector4D(t, 0, y, 0), H5Helper::Vector4D(1, dimsDst3D.z(), 1, dimsDst3D.x()), dstData, log);
                else
                    dstDataset->writeDataset(H5Helper::Vector3D(0, y, 0), H5Helper::Vector3D(dimsDst3D.z(), 1, dimsDst3D.x()), dstData, log);
                //delete[] srcData;
                //delete[] dstData;
            }
        }
        delete[] srcData;
        srcData = nullptr;
        delete[] dstData;
        dstData = nullptr;
    }


    delete tmpFile;
    tmpFile = nullptr;
    remove("tmp.h5");

    dstDataset->findAndSetGlobalMinAndMaxValue(true, log);

    // Save attributes
    dstDataset->setAttribute(H5Helper::SRC_SIZE_X_ATTR, dimsSrc3D.x(), log);
    dstDataset->setAttribute(H5Helper::SRC_SIZE_Y_ATTR, dimsSrc3D.y(), log);
    dstDataset->setAttribute(H5Helper::SRC_SIZE_Z_ATTR, dimsSrc3D.z(), log);
    dstDataset->setAttribute(H5Helper::SRC_DATASET_NAME_ATTR, srcDataset->getName(), log);

    if (srcDataset->hasAttribute(H5Helper::POSITION_Z_ATTR)
            && srcDataset->hasAttribute(H5Helper::POSITION_Y_ATTR)
            && srcDataset->hasAttribute(H5Helper::POSITION_X_ATTR)) {
        hsize_t z = srcDataset->readAttributeI(H5Helper::POSITION_Z_ATTR, log);
        hsize_t y = srcDataset->readAttributeI(H5Helper::POSITION_Y_ATTR, log);
        hsize_t x = srcDataset->readAttributeI(H5Helper::POSITION_X_ATTR, log);
        dstDataset->setAttribute(H5Helper::POSITION_Z_ATTR, Helper::round(z * ratio), log);
        dstDataset->setAttribute(H5Helper::POSITION_Y_ATTR, Helper::round(y * ratio), log);
        dstDataset->setAttribute(H5Helper::POSITION_X_ATTR, Helper::round(x * ratio), log);
        dstDataset->setAttribute(H5Helper::SRC_POSITION_Z_ATTR, z, log);
        dstDataset->setAttribute(H5Helper::SRC_POSITION_Y_ATTR, y, log);
        dstDataset->setAttribute(H5Helper::SRC_POSITION_X_ATTR, x, log);
    }
    double t1 = H5Helper::getTime();
    Helper::printDebugTime("dataset resampling", t0, t1);
}

/**
 * @brief Computes destination size
 * @param[in] dimsSrc Source dims
 * @param[in] maxChunkDims Maximal chunk dimensions
 * @param[in] ratio Dims ratio
 * @param[out] dimsDst Destination Dims
 * @param[out] chunkDims Chunk dims
 */
void Downsampling::computeDstDims(H5Helper::Vector3D dimsSrc, H5Helper::Vector3D maxChunkDims, float ratio, H5Helper::Vector3D &dimsDst, H5Helper::Vector3D &chunkDims)
{
    dimsDst.z(Helper::round(dimsSrc.z() * ratio));
    dimsDst.y(Helper::round(dimsSrc.y() * ratio));
    dimsDst.x(Helper::round(dimsSrc.x() * ratio));
    // Check < 1
    if (dimsDst.z() < 1) dimsDst.z(1);
    if (dimsDst.y() < 1) dimsDst.y(1);
    if (dimsDst.x() < 1) dimsDst.x(1);
    // Chunk size
    chunkDims.z(maxChunkDims.x());
    chunkDims.y(maxChunkDims.y());
    chunkDims.x(maxChunkDims.z());
    if (chunkDims.z() > dimsDst.z()) chunkDims.z(dimsDst.z());
    if (chunkDims.y() > dimsDst.y()) chunkDims.y(dimsDst.y());
    if (chunkDims.x() > dimsDst.x()) chunkDims.x(dimsDst.x());
}

/**
 * @brief Resize 2D data
 * @param[in] dataSrc Source data
 * @param[out] dataDst Destination data
 * @param[in] srcWidth Source width
 * @param[in] srcHeight Source height
 * @param[in] dstWidth Destination width
 * @param[in] dstHeight Destination height
 */
void Downsampling::resize2D(const float *dataSrc, float *dataDst, unsigned int srcWidth, unsigned int srcHeight, unsigned int dstWidth, unsigned int dstHeight)
{
    resize2D(dataSrc, dataDst, static_cast<hsize_t>(srcWidth), static_cast<hsize_t>(srcHeight), static_cast<hsize_t>(dstWidth), static_cast<hsize_t>(dstHeight));
}

/**
 * @brief Resize 2D data
 * @param[in] dataSrc Source data
 * @param[out] dataDst Destination data
 * @param[in] srcWidth Source width
 * @param[in] srcHeight Source height
 * @param[in] dstWidth Destination width
 * @param[in] dstHeight Destination height
 */
void Downsampling::resize2D(const float *dataSrc, float *dataDst, hsize_t srcWidth, hsize_t srcHeight, hsize_t dstWidth, hsize_t dstHeight)
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

            // Nearest-neighbor interpolation
            float newX = static_cast<float>(x) / scaleWidth;
            float newY = static_cast<float>(y) / scaleHeight;
            hsize_t xSrc = Helper::round(newX);
            hsize_t ySrc = Helper::round(newY);
            dataDst[hsize_t(x) + hsize_t(y) * dstWidth] = dataSrc[xSrc + ySrc * srcWidth];
        }
    }
}

/**
 * @brief Resize 3D data
 * @param[in] dataSrc Source data
 * @param[out] dataDst Destination data
 * @param[in] srcWidth Source width
 * @param[in] srcHeight Source height
 * @param[in] srcDepth Source depth
 * @param[in] dstWidth Destination width
 * @param[in] dstHeight Destination height
 * @param[in] dstDepth Destination depth
 */
void Downsampling::resize3D(const float *dataSrc, float *dataDst, unsigned int srcWidth, unsigned int srcHeight, unsigned int srcDepth, unsigned int dstWidth, unsigned int dstHeight, unsigned int dstDepth)
{
    resize3D(dataSrc, dataDst, static_cast<hsize_t>(srcWidth), static_cast<hsize_t>(srcHeight), static_cast<hsize_t>(srcDepth), static_cast<hsize_t>(dstWidth), static_cast<hsize_t>(dstHeight), static_cast<hsize_t>(dstDepth));
}

/**
 * @brief Resize 3D data
 * @param[in] dataSrc Source data
 * @param[out] dataDst Destination data
 * @param[in] srcWidth Source width
 * @param[in] srcHeight Source height
 * @param[in] srcDepth Source depth
 * @param[in] dstWidth Destination width
 * @param[in] dstHeight Destination height
 * @param[in] dstDepth Destination depth
 */
void Downsampling::resize3D(const float *dataSrc, float *dataDst, hsize_t srcWidth, hsize_t srcHeight, hsize_t srcDepth, hsize_t dstWidth, hsize_t dstHeight, hsize_t dstDepth)
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
                // Nearest-neighbor interpolation
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
