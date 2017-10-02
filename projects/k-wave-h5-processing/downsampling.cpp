/**
 * @file        downsampling.cpp
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        8  September 2016 (created) \n
 *              11 September 2017 (updated)
 *
 * @brief       The implementation file containing Downsampling class definition.
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

#include "downsampling.h"

/**
 * @brief Creates Downsampling object
 * @param[in] outputFile Output file
 * @param[in] dtsForPcs Datasets for processing
 * @param[in] settings Processing settings
 */
Downsampling::Downsampling(HDF5Helper::File *outputFile, DtsForPcs *dtsForPcs, Settings *settings)
    : Processing(outputFile, dtsForPcs, settings)
{

}

/**
 * @brief Executes processing
 */
void Downsampling::execute()
{
    std::vector<HDF5Helper::DatasetType> types = {
        HDF5Helper::DatasetType::BASIC_3D,
        HDF5Helper::DatasetType::CUBOID,
        HDF5Helper::DatasetType::CUBOID_ATTR
    };
    // TODO - downsampling of MASK_3D PHI, K, D, S datasets

    try {
        HDF5Helper::MapOfDatasets map = getDtsForPcs()->getDatasets();

        int count = 0;
        for (HDF5Helper::MapOfDatasetsIt it = map.begin(); it != map.end(); ++it) {
            HDF5Helper::Dataset *dataset = it->second;
            HDF5Helper::DatasetType datasetType = dataset->getType();

            if (checkDatasetType(datasetType, types)) {
                Helper::printDebugMsg("Downsampling of dataset " + dataset->getName());
                resampleDataset(dataset);
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
 */
void Downsampling::resampleDataset(HDF5Helper::Dataset *srcDataset)
{
    // Dims
    HDF5Helper::Vector dimsSrc = srcDataset->getDims();
    HDF5Helper::Vector3D dimsSrc3D = dimsSrc;
    HDF5Helper::Vector dimsDst;
    HDF5Helper::Vector3D dimsDst3D;
    HDF5Helper::Vector chunkDimsDst;
    HDF5Helper::Vector3D chunkDimsDst3D;

    // Compute ratio
    float ratio = float(getSettings()->getMaxSize()) / std::max(std::max(dimsSrc3D.z(), dimsSrc3D.y()), dimsSrc3D.x());
    // Check downsampling size
    if (ratio >= 1) {
        Helper::printErrorMsg("Bad destination size for downsampling");
        return;
    }

    // Compute destination dims
    computeDstDims(dimsSrc3D, ratio, dimsDst3D, chunkDimsDst3D, getSettings()->getMaxChunkSize());

    // For 4D datasets
    if (dimsSrc.getLength() == 4) {
        chunkDimsDst = HDF5Helper::Vector4D(getSettings()->getMaxChunkSize(), chunkDimsDst3D); // Maybe better is (1, chunkSizeDst3D)
        dimsDst = HDF5Helper::Vector4D(HDF5Helper::Vector4D(dimsSrc).w(), dimsDst3D);
    } else { // 3D datasets
        chunkDimsDst = chunkDimsDst3D;
        dimsDst = dimsDst3D;
    }

    // Create destination dataset
    getOutputFile()->createDatasetF(srcDataset->getName() + "_" + std::to_string(getSettings()->getMaxSize()), dimsDst, chunkDimsDst, true);
    HDF5Helper::Dataset *dstDataset = getOutputFile()->openDataset(srcDataset->getName() + "_" + std::to_string(getSettings()->getMaxSize()));

    double t0 = HDF5Helper::getTime();

    // Tmp has original Z dimension (simOutputFile->getNZ())
    HDF5Helper::Vector3D newTmpDatasetDims(dimsSrc3D.z(), dimsDst3D.y(), dimsDst3D.x());
    HDF5Helper::Vector3D newTmpDatasetChunkDims(dimsSrc3D.z(), 1, dimsDst3D.x());

    // Create temp file and dataset
    HDF5Helper::File *tmpFile = new HDF5Helper::File("tmp.h5", HDF5Helper::File::CREATE);
    tmpFile->createDatasetF("tmp", newTmpDatasetDims, newTmpDatasetChunkDims);
    HDF5Helper::Dataset *tmpDataset = tmpFile->openDataset("tmp");

    float *srcData = 0;
    float *dstData = 0;

    hsize_t steps = 1;
    if (dimsSrc.getLength() == 4)
        steps = HDF5Helper::Vector4D(dimsSrc).w();

    for (unsigned int t = 0; t < steps; t++) {
        // If we have enough memory, resample full 3D dataset
        if (srcDataset->getNumberOfElmsToLoad() >= srcDataset->getSize()) {
            if (dimsSrc.getLength() == 4)
                srcDataset->readDataset(HDF5Helper::Vector4D(t, 0, 0, 0), HDF5Helper::Vector4D(1, dimsSrc3D.z(), dimsSrc3D.y(), dimsSrc3D.x()), srcData);
            else
                srcDataset->readDataset(srcData);
            dstData = new float[dimsDst3D.x() * dimsDst3D.y() * dimsDst3D.z()]();
            resize3D(srcData, dstData, dimsSrc3D.x(), dimsSrc3D.y(), dimsSrc3D.z(), dimsDst3D.x(), dimsDst3D.y(), dimsDst3D.z());
            if (dimsSrc.getLength() == 4)
                dstDataset->writeDataset(HDF5Helper::Vector4D(t, 0, 0, 0), HDF5Helper::Vector4D(1, dimsDst3D.z(), dimsSrc3D.y(), dimsDst3D.x()), dstData, true);
            else
                dstDataset->writeDataset(dstData, true);
            delete[] srcData;
            delete[] dstData;
        } else {
            // First 2D slices in XY plane
            for (unsigned int z = 0; z < dimsSrc3D.z(); z++) {
                if (dimsSrc.getLength() == 4)
                    srcDataset->readDataset(HDF5Helper::Vector4D(t, z, 0, 0), HDF5Helper::Vector4D(1, 1, dimsSrc3D.y(), dimsSrc3D.x()), srcData);
                else
                    srcDataset->readDataset(HDF5Helper::Vector3D(z, 0, 0), HDF5Helper::Vector3D(1, dimsSrc3D.y(), dimsSrc3D.x()), srcData);
                dstData = new float[dimsDst3D.x() * dimsDst3D.y()]();
                resize2D(srcData, dstData, dimsSrc3D.x(), dimsSrc3D.y(), dimsDst3D.x(), dimsDst3D.y());
                tmpDataset->writeDataset(HDF5Helper::Vector3D(z, 0, 0), HDF5Helper::Vector3D(1, dimsDst3D.y(), dimsDst3D.x()), dstData, true);
                delete[] srcData;
                delete[] dstData;
            }

            // and after 2D slices XZ plane
            for (unsigned int y = 0; y < dimsDst3D.y(); y++) {
                tmpDataset->readDataset(HDF5Helper::Vector3D(0, y, 0), HDF5Helper::Vector3D(dimsSrc3D.z(), 1, dimsDst3D.x()), srcData);
                dstData = new float[dimsDst3D.x() * dimsDst3D.z()]();
                resize2D(srcData, dstData, dimsDst3D.x(), dimsSrc3D.z(), dimsDst3D.x(), dimsDst3D.z());
                if (dimsSrc.getLength() == 4)
                    dstDataset->writeDataset(HDF5Helper::Vector4D(t, 0, y, 0), HDF5Helper::Vector4D(1, dimsDst3D.z(), 1, dimsDst3D.x()), dstData, true);
                else
                    dstDataset->writeDataset(HDF5Helper::Vector3D(0, y, 0), HDF5Helper::Vector3D(dimsDst3D.z(), 1, dimsDst3D.x()), dstData, true);
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
    Helper::printDebugTime("dataset resampling", t0, t1);
}

/**
 * @brief Computes destination size
 * @param[in] dimsSrc Source dims
 * @param[in] ratio Dims ratio
 * @param[out] dimsDst Destination Dims
 * @param[out] chunkDims Chunk dims
 * @param[in] maxChunkSize Maximal chunk size
 */
void Downsampling::computeDstDims(HDF5Helper::Vector3D dimsSrc, float ratio, HDF5Helper::Vector3D &dimsDst, HDF5Helper::Vector3D &chunkDims, hsize_t maxChunkSize, bool log)
{
    dimsDst.z(Helper::round(dimsSrc.z() * ratio));
    dimsDst.y(Helper::round(dimsSrc.y() * ratio));
    dimsDst.x(Helper::round(dimsSrc.x() * ratio));
    // Check < 1
    if (dimsDst.z() < 1) dimsDst.z(1);
    if (dimsDst.y() < 1) dimsDst.y(1);
    if (dimsDst.x() < 1) dimsDst.x(1);
    // Chunk size
    chunkDims.z(maxChunkSize);
    chunkDims.y(maxChunkSize);
    chunkDims.x(maxChunkSize);
    if (chunkDims.z() > dimsDst.z()) chunkDims.z(dimsDst.z());
    if (chunkDims.y() > dimsDst.y()) chunkDims.y(dimsDst.y());
    if (chunkDims.x() > dimsDst.x()) chunkDims.x(dimsDst.x());
    if (log) {
        Helper::printDebugTwoColumns2S("new size", dimsDst);
    }
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
void Downsampling::resize2D(float *dataSrc, float *dataDst, unsigned int srcWidth, unsigned int srcHeight, unsigned int dstWidth, unsigned int dstHeight)
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
void Downsampling::resize2D(float *dataSrc, float *dataDst, hsize_t srcWidth, hsize_t srcHeight, hsize_t dstWidth, hsize_t dstHeight)
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
void Downsampling::resize3D(float *dataSrc, float *dataDst, unsigned int srcWidth, unsigned int srcHeight, unsigned int srcDepth, unsigned int dstWidth, unsigned int dstHeight, unsigned int dstDepth)
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
void Downsampling::resize3D(float *dataSrc, float *dataDst, hsize_t srcWidth, hsize_t srcHeight, hsize_t srcDepth, hsize_t dstWidth, hsize_t dstHeight, hsize_t dstDepth)
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
