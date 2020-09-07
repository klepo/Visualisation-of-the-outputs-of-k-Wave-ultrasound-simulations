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
                double t00 = H5Helper::getTime();
                compressDataset(dataset, getSettings()->getFlagLog());
                double t11 = H5Helper::getTime();
                addTime(t11 - t00);
                addSize(dataset->getSize() * 4);
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

    float sizeMultiplier = getSettings()->getFlagC16bit() ? compressHelper->getHarmonics() * 1.25f : compressHelper->getHarmonics() * 2.0f;

    int kMaxExp = H5Helper::CompressHelper::kMaxExpU;
    if (srcDataset->getName() == "/" + H5Helper::P_INDEX_DATASET || srcDataset->getName() == "/" + H5Helper::P_CUBOID_DATASET)
        kMaxExp = H5Helper::CompressHelper::kMaxExpP;

    //float* powers = nullptr;
    //powers = new float[70/*kMaxExp * 2 + 1*/]();
    //for (hsize_t i = 0; i < 70/*kMaxExp * 2 + 1*/; i++)
    //{
    //    powers[i] = powf(2.0f, float(kMaxExp - hssize_t(i)));
    //}

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
    #pragma omp parallel for
    for (ssize_t i = 0; i < srcDataset->getGeneralBlockDims().getSize(); i++) {
        data[i] = 0.0f;
    }
    H5Helper::Vector offset;
    H5Helper::Vector count;
    float maxV = std::numeric_limits<float>::min();
    float minV = std::numeric_limits<float>::max();
    hsize_t maxVIndex = 0;
    hsize_t minVIndex = 0;

    // If we have enough memory - minimal for one full step in 3D space
    if (srcDataset->getNumberOfElmsToLoad() >= outputStepSize * 3) {
        // Complex buffers for accumulation
        float *sCTmp1 = (float*) _mm_malloc(outputStepSize * sizeof(float), 1024);
        float *sCTmp2 = nullptr;
        if (flagNoOverlap) {
            sCTmp2 = sCTmp1;
        } else {
            sCTmp2 = (float*) _mm_malloc(outputStepSize * sizeof(float), 1024);
        }
        #pragma omp parallel for
        for (ssize_t i = 0; i < outputStepSize; i++) {
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
                    //std::cout << p << std::endl;
                    const hsize_t sP = step * stepSize + hsize_t(p);
                    const hsize_t pOffset = compressHelper->getHarmonics() * hsize_t(p);

                    //H5Helper::checkOrSetMinMaxValue(minV, maxV, data[sP], minVIndex, maxVIndex, stepsOffset * stepSize + sP);

                    //For every harmonics
                    for (hsize_t ih = 0; ih < hsize_t(compressHelper->getHarmonics()); ih++) {
                        hsize_t pH = pOffset + ih;
                        const size_t bIndex = ih * bSize + stepLocal;

                        if (getSettings()->getFlagC16bit()) {
                            pH = pH * 5;
                            H5Helper::floatC cc1;
                            H5Helper::floatC cc2;
                            if (flagNoOverlap) {
                                /*if (pH / 5 * 2 == 2936420 && stepsOffset + step + 1 == 24)
                                    std::cout << cc1 << std::endl;*/
                                H5Helper::CompressHelper::convert40bToFloatC(&sCTmp1Int8[pH], cc1, kMaxExp);
                                cc1 += compressHelper->getBE()[bIndex] * data[sP] + compressHelper->getBE_1()[bIndex] * data[sP];
                                /*if (pH / 5 * 2 == 2936420)
                                    std::cout << cc1 << std::endl;*/
                                H5Helper::CompressHelper::convertFloatCTo40b(cc1, &sCTmp1Int8[pH], kMaxExp);
                                //H5Helper::CompressHelper::convert40bToFloatC(&sCTmp1Int8[pH], cc1);
                                /*if (pH / 5 * 2 == 2936420)
                                    std::cout << cc1 << std::endl;*/
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
/*
                            // | 8 + 8 (real part)| 8 + 8 (imaginary part)| 8 (2 (signs) + 6 (exponent index) |
                            pH = pH * 5;
                            uint8_t e = *reinterpret_cast<uint8_t*>(&sCTmp1Int8[pH + 4]);
                            uint8_t s1R = (e >> 6) & 0x1;// neshiftovat
                            uint8_t s1I = e >> 7;
                            e = e & 0x3F;

                            float c1R = compressHelper->getBE()[bIndex].real() * data[sP];
                            float c1I = compressHelper->getBE()[bIndex].imag() * data[sP];
                            float c2R = compressHelper->getBE_1()[bIndex].real() * data[sP];
                            float c2I = compressHelper->getBE_1()[bIndex].imag() * data[sP];

                            // Init exponent
                            if (data[sP] != 0.0f && (e < 1)) {
                                int32_t eL = 0;
                                const float maxC = std::max(std::max(abs(c1R), abs(c1I)), std::max(abs(c2R), abs(c2I)));
                                frexp(maxC, &eL);
                                e = uint8_t(std::max(kMaxExp - (17 - eL), 1));
                            }

                            if (flagNoOverlap) {
                                // Correlation step
                                int64_t cIR = *reinterpret_cast<uint16_t*>(&sCTmp1Int8[pH]) * (s1R ? -1 : 1);
                                int64_t cII = *reinterpret_cast<uint16_t*>(&sCTmp1Int8[pH + 2]) * (s1I ? -1 : 1);

                                if (pH * 2 == 30085380)
                                    std::cout << H5Helper::floatC(float(cIR) / powers[e], float(cII) / powers[e]) << std::endl;

                                cIR += int64_t(roundf((c1R + c2R) * powers[e]));
                                cII += int64_t(roundf((c1I + c2I) * powers[e]));

                                while (abs(cIR) > 0xFFFF || abs(cII) > 0xFFFF) {
                                    e++;
                                    cIR >>= 1;
                                    cII >>= 1;
                                }

                                *reinterpret_cast<uint16_t*>(&sCTmp1Int8[pH]) = uint16_t(abs(cIR));
                                *reinterpret_cast<uint16_t*>(&sCTmp1Int8[pH + 2]) = uint16_t(abs(cII));
                                *reinterpret_cast<uint8_t*>(&sCTmp1Int8[pH + 4]) = ((cII < 0) ? 1 : 0) << 7 | ((cIR < 0) ? 1 : 0) << 6 | e;
                            } else {
                                uint8_t e2 = *reinterpret_cast<uint8_t*>(&sCTmp2Int8[pH + 4]);
                                uint8_t s2R = (e2 >> 6) & 0x1;
                                uint8_t s2I = e2 >> 7;

                                // Correlation step
                                int64_t c1IR = *reinterpret_cast<uint16_t*>(&sCTmp1Int8[pH]) * (s1R ? -1 : 1);
                                int64_t c1II = *reinterpret_cast<uint16_t*>(&sCTmp1Int8[pH + 2]) * (s1I ? -1 : 1);
                                c1IR += int64_t(roundf(c1R * powers[e]));
                                c1II += int64_t(roundf(c1I * powers[e]));

                                int64_t c2IR = *reinterpret_cast<uint16_t*>(&sCTmp2Int8[pH]) * (s2R ? -1 : 1);
                                int64_t c2II = *reinterpret_cast<uint16_t*>(&sCTmp2Int8[pH + 2]) * (s2I ? -1 : 1);
                                c2IR += int64_t(roundf(c2R * powers[e]));
                                c2II += int64_t(roundf(c2I * powers[e]));

                                while (abs(c1IR) > 0xFFFF || abs(c1II) > 0xFFFF || abs(c2IR) > 0xFFFF || abs(c2II) > 0xFFFF) {
                                    e++;
                                    c1IR >>= 1;
                                    c1II >>= 1;
                                    c2IR >>= 1;
                                    c2II >>= 1;
                                }

                                *reinterpret_cast<uint16_t*>(&sCTmp1Int8[pH]) = uint16_t(abs(c1IR));
                                *reinterpret_cast<uint16_t*>(&sCTmp1Int8[pH + 2]) = uint16_t(abs(c1II));
                                *reinterpret_cast<uint8_t*>(&sCTmp1Int8[pH + 4]) = ((c1II < 0) ? 1 : 0) << 7 | ((c1IR < 0) ? 1 : 0) << 6 | e;
                                *reinterpret_cast<uint16_t*>(&sCTmp2Int8[pH]) = uint16_t(abs(c2IR));
                                *reinterpret_cast<uint16_t*>(&sCTmp2Int8[pH + 2]) = uint16_t(abs(c2II));
                                *reinterpret_cast<uint8_t*>(&sCTmp2Int8[pH + 4]) = ((c2II < 0) ? 1 : 0) << 7 | ((c2IR < 0) ? 1 : 0) << 6 | e;

                                // Mirror first "half" frame
                                if (mirrorFirstHalfFrameFlag) {
                                    c2IR += c1IR;
                                    c2II += c1II;
                                    while (abs(c2IR) > 0xFFFF || abs(c2II) > 0xFFFF) {
                                        e++;
                                        c2IR >>= 1;
                                        c2II >>= 1;
                                    }
                                    *reinterpret_cast<uint16_t*>(&sCTmp2Int8[pH]) = uint16_t(abs(c2IR));
                                    *reinterpret_cast<uint16_t*>(&sCTmp2Int8[pH + 2]) = uint16_t(abs(c2II));
                                    *reinterpret_cast<uint8_t*>(&sCTmp2Int8[pH + 4]) = ((c2II < 0) ? 1 : 0) << 7 | ((c2IR < 0) ? 1 : 0) << 6 | e;
                                }
                            }*/
                        } else {
                            // Correlation step
                            sCTmp1FloatC[pH] += compressHelper->getBE()[bIndex] * data[sP];
                            sCTmp2FloatC[pH] += compressHelper->getBE_1()[bIndex] * data[sP];

                            /*if (pH * 2 == 30085380)
                                std::cout << sCTmp1FloatC[pH] << std::endl;*/

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
                        dstDataset->writeDataset(H5Helper::Vector4D(frame, 0, 0, 0), H5Helper::Vector4D(1, outputDims[1], outputDims[2], outputDims[3]), dataC);
                    else if (dims.getLength() == 3) // 3D dataset
                        dstDataset->writeDataset(H5Helper::Vector3D(0, frame, 0), H5Helper::Vector3D(1, 1, outputDims[2]), dataC);

                    if (log)
                        Helper::printDebugMsgEnd("saved");

                    // Set zeros
                    //memset(dataC, 0, outputStepSize * sizeof(float));
                    // Set zeros for next accumulation
                    /*if (getSettings()->getFlagC16bit()) {
                        int8_t* mCurrentStoreBuffer8 = reinterpret_cast<int8_t*>(dataC);
                        #pragma omp parallel for
                        for (ssize_t i = 0; i < outputStepSize * 4; i += 5) { // TODO
                            *reinterpret_cast<int32_t*>(mCurrentStoreBuffer8 + i) = 0;
                        }
                    } else*/ {
                        #pragma omp parallel for
                        for (ssize_t i = 0; i < outputStepSize; i++) {
                            dataC[i] = 0.0f;
                        }
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
    dstDataset->setAttribute(H5Helper::MIN_ATTR, minV, log);
    dstDataset->setAttribute(H5Helper::MAX_ATTR, maxV, log);
    dstDataset->setAttribute(H5Helper::MIN_INDEX_ATTR, minVIndex, log);
    dstDataset->setAttribute(H5Helper::MAX_INDEX_ATTR, maxVIndex, log);
    dstDataset->setAttribute(H5Helper::C_HARMONICS_ATTR, getSettings()->getHarmonics(), log);
    dstDataset->setAttribute(H5Helper::C_TYPE_ATTR, "c", log);
    dstDataset->setAttribute(H5Helper::C_PERIOD_ATTR, getSettings()->getPeriod(), log);
    dstDataset->setAttribute(H5Helper::C_MOS_ATTR, getSettings()->getMOS(), log);
    dstDataset->setAttribute(H5Helper::SRC_DATASET_NAME_ATTR, srcDataset->getName(), log);
    dstDataset->setAttribute("c_shift", hsize_t(getSettings()->getFlagShift()), log);
    dstDataset->setAttribute("c_complex_size", (getSettings()->getFlagC16bit() ? 1.25f : 2.0f), log);
    dstDataset->setAttribute("c_max_exp", kMaxExp, log);

    double t1 = H5Helper::getTime();
    Helper::printDebugTime("dataset compression", t0, t1);

    getOutputFile()->closeDataset(dstDataset, log);
}
