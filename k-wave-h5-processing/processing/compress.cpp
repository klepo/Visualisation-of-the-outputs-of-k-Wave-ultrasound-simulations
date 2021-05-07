/**
 * @file        compress.cpp
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        8  September 2016 (created) <br>
 *              27 March     2019 (updated)
 *
 * @brief       The implementation file containing Compress class definition.
 *
 * @license     This file is part of the k-Wave-h5-processing tool for processing the HDF5 data
 *              created by the k-Wave toolbox - http://www.k-wave.org. This file may be used,
 *              distributed and modified under the terms of the LGPL version 3 open source
 *              license. A copy of the LGPL license should have been received with this file.
 *              Otherwise, it can be found at: http://www.gnu.org/copyleft/lesser.html.
 *
 * @copyright   Copyright © 2019, Petr Kleparnik, VUT FIT Brno. All Rights Reserved.
 *
 */

#include "compress.h"

/**
 * @brief Creates Compress object
 * @param[in] outputFile Output file
 * @param[in] dtsForPcs Datasets for processing
 * @param[in] settings Processing settings
 */
Compress::Compress(H5Helper::File *outputFile, DtsForPcs *dtsForPcs, const Settings *settings)
    : Processing(outputFile, dtsForPcs, settings)
{

}

/**
 * @brief Executes processing
 */
void Compress::execute()
{
    std::vector<H5Helper::DatasetType> types = {
        H5Helper::DatasetType::TIME_STEPS_INDEX,
        H5Helper::DatasetType::CUBOID,
        H5Helper::DatasetType::CUBOID_ATTR
    };
    // TODO downsampled datasets

    try {
        H5Helper::MapOfDatasets map = getDtsForPcs()->getDatasets();
        hsize_t sensorMaskSize = getDtsForPcs()->getSensorMaskSize();
        int count = 0;
        for (H5Helper::MapOfDatasetsIt it = map.begin(); it != map.end(); ++it) {
            H5Helper::Dataset *dataset = it->second;
            H5Helper::DatasetType datasetType = dataset->getType(sensorMaskSize);
            if (checkDatasetType(datasetType, types)) {
                Helper::printDebugMsg("Compression of dataset " + dataset->getName());
                //double t00 = H5Helper::getTime();
                compressDataset(dataset, getSettings()->getFlagLog());
                //double t11 = H5Helper::getTime();
                //addTime(t11 - t00);
                //addSize(dataset->getSize() * 4);
                count++;
                Helper::printDebugMsg("Compression of dataset " + dataset->getName() + " done");
            }
        }
        if (count == 0) {
            Helper::printErrorMsg("No datasets for compression in simulation output file");
        }
    } catch(std::exception &e) {
        Helper::printErrorMsg(e.what());
        std::exit(EXIT_FAILURE);
    }
}

/**
 * @brief Compresses dataset
 * @param[in] srcDataset Source dataset
 * @param[in] log Logging flag (optional)
 */
void Compress::compressDataset(H5Helper::Dataset *srcDataset, bool log)
{
    double t0 = H5Helper::getTime();

    if (getSettings()->getPeriod() == 0.0f) {
        Helper::printErrorMsg("No known period for compression");
        return;
    }

    // First encoding parameter     - period
    // Second encoding parameter    - multiple of overlap size
    // Third encoding parameter     - number of harmonic frequencies
    // Fourth encoding parameter    - shift flag
    H5Helper::CompressHelper *compressHelper = new H5Helper::CompressHelper(getSettings()->getPeriod(), getSettings()->getMOS(), getSettings()->getHarmonics(), true, getSettings()->getFlagShift());

    float sizeMultiplier = getSettings()->getFlagC40bit() ? compressHelper->getHarmonics() * H5Helper::CompressHelper::complexSize40bit : compressHelper->getHarmonics() * 2.0f;

    int kMaxExp = H5Helper::CompressHelper::kMaxExpU;
    if (srcDataset->getName() == "/" + H5Helper::P_INDEX_DATASET || srcDataset->getName() == "/" + H5Helper::P_CUBOID_DATASET)
        kMaxExp = H5Helper::CompressHelper::kMaxExpP;

    if (log)
        Helper::printDebugMsg("Compression with period "
                              + std::to_string(size_t(compressHelper->getPeriod()))
                              + " steps (" + std::to_string(srcDataset->getFile()->getFrequency(compressHelper->getPeriod()))
                              + " Hz) and " + std::to_string(compressHelper->getHarmonics())
                              + " harmonic frequencies");

    // Overlap size and base size
    hsize_t oSize = compressHelper->getOSize();
    hsize_t bSize = compressHelper->getBSize();

    // Get dims
    H5Helper::Vector dims = srcDataset->getDims();
    H5Helper::Vector outputDims = dims;

    // Compute steps, output steps, step size, output step size and output dims
    hsize_t steps = 0;
    hsize_t outputSteps = 0;
    hsize_t stepSize = 0;
    hsize_t outputStepSize = 0;
    if (dims.getLength() == 4) { // 4D dataset
        steps = H5Helper::Vector4D(dims).w();
        outputSteps = hsize_t(floor(float(steps) / oSize));
        outputDims[0] = outputSteps;
        outputDims[3] = hsize_t(ceilf(outputDims[3] * sizeMultiplier));
        stepSize = dims[1] * dims[2] * dims[3];
        outputStepSize = outputDims[1] * outputDims[2] * outputDims[3];
    } else if (dims.getLength() == 3) { // 3D dataset (defined by sensor mask)
        steps = H5Helper::Vector3D(dims).y();
        outputSteps = hsize_t(floor(float(steps) / oSize));
        outputDims[1] = outputSteps;
        outputDims[2] = hsize_t(ceilf(outputDims[2] * sizeMultiplier));
        stepSize = dims[2];
        outputStepSize = outputDims[2];
    } else { // Something wrong.
        Helper::printErrorMsg("Something wrong with dataset dims");
        return;
    }

    // Chunk dims
    H5Helper::Vector chunkDims(srcDataset->getChunkDims());
    if (dims.getLength() == 4) { // 4D dataset
        chunkDims[3] = hsize_t(sizeMultiplier * ceilf(float(chunkDims[3]) / sizeMultiplier));
    } else if (dims.getLength() == 3) { // 3D dataset (defined by sensor mask)
        chunkDims[2] = hsize_t(sizeMultiplier * ceilf(float(chunkDims[2]) / sizeMultiplier));
    } else { // Something wrong.
        Helper::printErrorMsg("Something wrong with dataset dims");
        return;
    }

    Helper::printDebugTwoColumnsS("steps", steps);
    Helper::printDebugTwoColumnsS("outputSteps", outputSteps);
    Helper::printDebugTwoColumnsS("dims", dims);
    Helper::printDebugTwoColumnsS("outputDims", outputDims);
    Helper::printDebugTwoColumnsS("stepSize", stepSize);
    Helper::printDebugTwoColumnsS("outputStepSize", outputStepSize);
    Helper::printDebugTwoColumnsS("chunkDims", chunkDims);

    // Check no overlapping flag
    bool flagNoOverlap = getSettings()->getFlagNoOverlap();
    // Too few steps to use overlapping compression
    if (getSettings()->getPeriod() >= steps) {
        flagNoOverlap = true;
    }

    // Create destination dataset
    std::string dstName = srcDataset->getSuffixName("_c");
    getOutputFile()->createDatasetF(dstName, outputDims, chunkDims, true, log);
    H5Helper::Dataset *dstDataset = getOutputFile()->openDataset(dstName, log);

    // Variables for block reading
    float *data = (float*) _mm_malloc(srcDataset->getGeneralBlockDims().getSize() * sizeof(float), 16);
    H5Helper::Vector offset;
    H5Helper::Vector count;
    float maxV = std::numeric_limits<float>::min();
    float minV = std::numeric_limits<float>::max();
    hsize_t maxVIndex = 0;
    hsize_t minVIndex = 0;

    // If we have enough memory - minimal for one full step in 3D space
    if (srcDataset->getFile()->getNumberOfElmsToLoad() >= outputStepSize * 3) {
        // Complex buffers for accumulation
        float *sCTmp1 = (float*) _mm_malloc(outputStepSize * sizeof(float), 16);
        float *sCTmp2 = nullptr;
        if (flagNoOverlap) {
            sCTmp2 = sCTmp1;
        } else {
            sCTmp2 = (float*) _mm_malloc(outputStepSize * sizeof(float), 16);
        }
        #pragma omp parallel for
        for (ssize_t i = 0; i < ssize_t(outputStepSize); i++) {
            sCTmp1[i] = 0.0f;
            sCTmp2[i] = 0.0f;
        }
        H5Helper::floatC *sCTmp1FloatC = reinterpret_cast<H5Helper::floatC*>(sCTmp1);
        H5Helper::floatC *sCTmp2FloatC = reinterpret_cast<H5Helper::floatC*>(sCTmp2);
        uint8_t *sCTmp1Int8 = reinterpret_cast<uint8_t*>(sCTmp1);
        uint8_t *sCTmp2Int8 = reinterpret_cast<uint8_t*>(sCTmp2);

        hsize_t frame = 0;

        // Reading and compression
        for (hsize_t i = 0; i < srcDataset->getNumberOfBlocks(); i++) {
            srcDataset->readBlock(i, offset, count, data, log);

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
                if (log)
                    Helper::printDebugMsg("Encoding step " + std::to_string(stepsOffset + step + 1) + "/" + std::to_string(steps));
                // Compute local index
                const hsize_t stepLocal = (stepsOffset + step) % (bSize - 1);
                const bool savingFlag = ((stepLocal + 1) % oSize == 0) ? true : false;
                const bool oddFrameFlag = ((frame + 1) % 2 == 0) ? true : false;
                const bool mirrorFirstHalfFrameFlag = (frame == 0 && savingFlag && !flagNoOverlap) ? true : false;

                // For every point
                #pragma omp parallel for
                for (hssize_t p = 0; p < hssize_t(stepSize); p++) {
                    const hsize_t sP = step * stepSize + hsize_t(p);
                    const hsize_t pOffset = compressHelper->getHarmonics() * hsize_t(p);
                    // TODO Min/max values
                    //H5Helper::checkOrSetMinMaxValue(minV, maxV, data[sP], minVIndex, maxVIndex, stepsOffset * stepSize + sP);

                    //For every harmonics
                    for (hsize_t ih = 0; ih < hsize_t(compressHelper->getHarmonics()); ih++) {
                        hsize_t pH = pOffset + ih;
                        const size_t bIndex = ih * bSize + stepLocal;

                        if (getSettings()->getFlagC40bit()) {
                            pH = pH * 5;
                            H5Helper::floatC cc1;
                            H5Helper::floatC cc2;
                            if (flagNoOverlap) {
                                H5Helper::CompressHelper::convert40bToFloatC(&sCTmp1Int8[pH], cc1, kMaxExp);
                                cc1 += compressHelper->getBE()[bIndex] * data[sP] + compressHelper->getBE_1()[bIndex] * data[sP];
                                H5Helper::CompressHelper::convertFloatCTo40b(cc1, &sCTmp1Int8[pH], kMaxExp);
                            } else {
                                H5Helper::CompressHelper::convert40bToFloatC(&sCTmp1Int8[pH], cc1, kMaxExp);
                                H5Helper::CompressHelper::convert40bToFloatC(&sCTmp2Int8[pH], cc2, kMaxExp);
                                cc1 += compressHelper->getBE()[bIndex] * data[sP];
                                cc2 += compressHelper->getBE_1()[bIndex] * data[sP];
                                H5Helper::CompressHelper::convertFloatCTo40b(cc1, &sCTmp1Int8[pH], kMaxExp);
                                H5Helper::CompressHelper::convertFloatCTo40b(cc2, &sCTmp2Int8[pH], kMaxExp);
                                // Mirror first "half" frame
                                if (mirrorFirstHalfFrameFlag) {
                                    cc2 += cc1;
                                    H5Helper::CompressHelper::convertFloatCTo40b(cc2, &sCTmp2Int8[pH], kMaxExp);
                                }
                            }
                        } else {
                            // Correlation step
                            sCTmp1FloatC[pH] += compressHelper->getBE()[bIndex] * data[sP];
                            sCTmp2FloatC[pH] += compressHelper->getBE_1()[bIndex] * data[sP];

                            // Mirror first "half" frame
                            if (mirrorFirstHalfFrameFlag) {
                              sCTmp2FloatC[pH] += sCTmp1FloatC[pH];
                            }
                        }
                    }
                }

                if (savingFlag) {

                    float *dataC;

                    // Select accumulated value
                    if (oddFrameFlag) {
                        dataC = sCTmp1;
                    } else {
                        dataC = sCTmp2;
                    }

                    // Drop first "half" frame
                    if (log)
                        Helper::printDebugMsgStart("Saving frame " + std::to_string(frame + 1) + "/" + std::to_string(outputSteps));

                    if (dims.getLength() == 4) // 4D dataset
                        dstDataset->writeDataset(H5Helper::Vector4D(frame, 0, 0, 0), H5Helper::Vector4D(1, outputDims[1], outputDims[2], outputDims[3]), dataC, log);
                    else if (dims.getLength() == 3) // 3D dataset
                        dstDataset->writeDataset(H5Helper::Vector3D(0, frame, 0), H5Helper::Vector3D(1, 1, outputDims[2]), dataC, log);

                    if (log)
                        Helper::printDebugMsgEnd("saved");

                    // Set zeros for next accumulation
                    #pragma omp parallel for
                    for (ssize_t i = 0; i < ssize_t(outputStepSize); i++) {
                        dataC[i] = 0.0f;
                    }

                    // Increment frame
                    frame++;
                }
            }
        }
        // Delete buffers
        _mm_free(sCTmp1);
        sCTmp1 = nullptr;
        if (!flagNoOverlap) {
            _mm_free(sCTmp2);
            sCTmp2 = nullptr;
        }
    } else {
        // Not implemented yet
        Helper::printErrorMsg("Not implemented for such big datasets yet");
        _mm_free(data);
        data = nullptr;
        delete compressHelper;
        compressHelper = nullptr;
        getOutputFile()->closeDataset(dstDataset, log);
        return;
    }

    // Delete some memory
    _mm_free(data);
    data = nullptr;
    delete compressHelper;
    compressHelper = nullptr;

    // Copy attributes
    copyAttributes(srcDataset, dstDataset);

    // Set attributes
    //dstDataset->setAttribute(H5Helper::MIN_ATTR, minV, log);
    //dstDataset->setAttribute(H5Helper::MAX_ATTR, maxV, log);
    //dstDataset->setAttribute(H5Helper::MIN_INDEX_ATTR, minVIndex, log);
    //dstDataset->setAttribute(H5Helper::MAX_INDEX_ATTR, maxVIndex, log);
    dstDataset->setAttribute(H5Helper::C_HARMONICS_ATTR, getSettings()->getHarmonics(), log);
    dstDataset->setAttribute(H5Helper::C_TYPE_ATTR, "c", log);
    dstDataset->setAttribute(H5Helper::C_PERIOD_ATTR, getSettings()->getPeriod(), log);
    dstDataset->setAttribute(H5Helper::C_MOS_ATTR, getSettings()->getMOS(), log);
    dstDataset->setAttribute(H5Helper::SRC_DATASET_NAME_ATTR, srcDataset->getName(), log);
    dstDataset->setAttribute("c_shift", hsize_t(getSettings()->getFlagShift()), log);
    dstDataset->setAttribute("c_complex_size", getSettings()->getFlagC40bit() ? H5Helper::CompressHelper::complexSize40bit : 2.0f, log);
    if (getSettings()->getFlagC40bit())
        dstDataset->setAttribute("c_max_exp", kMaxExp, log);

    double t1 = H5Helper::getTime();
    Helper::printDebugTime("dataset compression", t0, t1);

    getOutputFile()->closeDataset(dstDataset, log);
}
