/**
 * @file        decompress.cpp
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        8  September 2016 (created) <br>
 *              27 March     2019 (updated)
 *
 * @brief       The implementation file containing Decompress class definition.
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

#include "decompress.h"

/**
 * @brief Creates Decompress object
 * @param[in] outputFile Output file
 * @param[in] dtsForPcs Datasets for processing
 * @param[in] settings Processing settings
 */
Decompress::Decompress(H5Helper::File *outputFile, DtsForPcs *dtsForPcs, const Settings *settings)
    : Processing(outputFile, dtsForPcs, settings)
{

}

/**
 * @brief Executes processing
 */
void Decompress::execute()
{
    std::vector<H5Helper::DatasetType> types = {
        H5Helper::DatasetType::TIME_STEPS_C_INDEX,
        H5Helper::DatasetType::CUBOID_C,
        H5Helper::DatasetType::CUBOID_ATTR_C
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
                Helper::printDebugMsg("Decompression of dataset " + dataset->getName());
                decompressDataset(dataset, getSettings()->getFlagLog());
                count++;
                Helper::printDebugMsg("Decompression of dataset " + dataset->getName() + " done");
            }
        }
        if (count == 0) {
            Helper::printErrorMsg("No datasets for decompression in simulation output file");
        }
    } catch(std::exception &e) {
        Helper::printErrorMsg(e.what());
        std::exit(EXIT_FAILURE);
    }
}

/**
 * @brief Decompresses datasets
 * @param[in] srcDataset Source dataset
 * @param[in] log Logging flag (optional)
 */
void Decompress::decompressDataset(H5Helper::Dataset *srcDataset, bool log)
{
    double t0 = H5Helper::getTime();

    // First decoding parameter     - period
    // Second decoding parameter    - multiple of overlap size
    // Third encoding parameter     - number of harmonic frequencies
    // Fourth encoding parameter    - shift flag
    hsize_t mos = srcDataset->hasAttribute(H5Helper::C_MOS_ATTR) ? srcDataset->readAttributeI(H5Helper::C_MOS_ATTR, log) : 1;
    hsize_t harmonics = srcDataset->hasAttribute(H5Helper::C_HARMONICS_ATTR) ? srcDataset->readAttributeI(H5Helper::C_HARMONICS_ATTR, log) : 1;
    bool shift = srcDataset->hasAttribute("c_shift") ? (srcDataset->readAttributeI("c_shift", log) > 0) : getSettings()->getFlagShift();
    H5Helper::CompressHelper *compressHelper = new H5Helper::CompressHelper(srcDataset->readAttributeF(H5Helper::C_PERIOD_ATTR, log), mos, harmonics, false, shift);

    float complexSize = srcDataset->hasAttribute("c_complex_size") ? srcDataset->readAttributeF("c_complex_size", true) : 2.0f;
    float sizeMultiplier = compressHelper->getHarmonics() * complexSize;
    bool flagC40bit = complexSize == H5Helper::CompressHelper::complexSize40bit ? true : false;

    int kMaxExp = H5Helper::CompressHelper::kMaxExpU;
    if (srcDataset->hasAttribute("c_max_exp")) {
        kMaxExp = int(srcDataset->readAttributeI("c_max_exp", false));
    } else if (srcDataset->getName() == "/" + H5Helper::P_INDEX_DATASET_C || srcDataset->getName() == "/" + H5Helper::P_CUBOID_DATASET_C) {
        kMaxExp = H5Helper::CompressHelper::kMaxExpP;
    }

    if (log)
        Helper::printDebugMsg("Decompression with period "
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
        outputSteps = (steps + 1) * oSize ;
        outputDims[0] = outputSteps;
        outputDims[3] = hsize_t(floorf(outputDims[3] / sizeMultiplier));
        stepSize = dims[1] * dims[2] * dims[3];
        outputStepSize = outputDims[1] * outputDims[2] * outputDims[3];
    } else if (dims.getLength() == 3) { // 3D dataset (defined by sensor mask)
        steps = H5Helper::Vector3D(dims).y();
        outputSteps = (steps + 1) * oSize;
        outputDims[1] = outputSteps;
        outputDims[2] = hsize_t(floorf(outputDims[2] / sizeMultiplier));;
        stepSize = dims[2];
        outputStepSize = outputDims[2];
    } else { // Something wrong.
        Helper::printErrorMsg("Something wrong with dataset dims");
        return;
    }

    // Chunk dims
    H5Helper::Vector chunkDims(srcDataset->getChunkDims());
    if (dims.getLength() == 4) { // 4D dataset
        //chunkDims[3] /= compressHelper->getHarmonics() * 2;
        if (chunkDims[3] > outputDims[3]) {
            chunkDims[3] = outputDims[3];
        }
    } else if (dims.getLength() == 3) { // 3D dataset (defined by sensor mask)
        //chunkDims[2] /= compressHelper->getHarmonics() * 2;
        if (chunkDims[2] > outputDims[2]) {
            chunkDims[2] = outputDims[2];
        }
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

    // Create destination dataset
    std::string dstName = srcDataset->getSuffixName("_d");
    getOutputFile()->createDatasetF(dstName, outputDims, chunkDims, true, log);
    H5Helper::Dataset *dstDataset = getOutputFile()->openDataset(dstName, log);

    // Read full steps
    srcDataset->setNumberOfElmsToLoad((srcDataset->getNumberOfElmsToLoad() / stepSize) * stepSize);
    // Variables for block reading
    float *dataC = (float*) _mm_malloc(srcDataset->getGeneralBlockDims().getSize() * sizeof(float), 16);
    H5Helper::Vector offset;
    H5Helper::Vector count;
    float maxV = std::numeric_limits<float>::min();
    float minV = std::numeric_limits<float>::max();
    hsize_t maxVIndex = 0;
    hsize_t minVIndex = 0;

    // If we have enough memory - minimal for one full step in 3D space
    if (srcDataset->getFile()->getNumberOfElmsToLoad() >= stepSize * 2 + outputStepSize) {
        // Complex buffers for last coefficients
        H5Helper::floatC *cC = new H5Helper::floatC[outputStepSize * compressHelper->getHarmonics()]();
        H5Helper::floatC *lC = new H5Helper::floatC[outputStepSize * compressHelper->getHarmonics()]();

        // Variable for writing multiple steps at once
        hsize_t stepsToWrite = (dstDataset->getRealNumberOfElmsToLoad() - stepSize + outputStepSize) / outputStepSize;

        // Output buffer
        float *data = new float[outputStepSize * stepsToWrite]();

        hsize_t step = 0;
        hsize_t stepToWrite = 0;

        // Reading and decompression
        for (hsize_t i = 0; i < srcDataset->getNumberOfBlocks(); i++) {
            srcDataset->readBlock(i, offset, count, dataC, log);

            hsize_t framesCount;
            hsize_t framesOffset;

            if (dims.getLength() == 4) { // 4D dataset
                framesCount = count[0];
                framesOffset = offset[0];
            } else { // 3D dataset
                framesCount = count[1];
                framesOffset = offset[1];
            }

            // Because of last coefficients duplication
            if (framesOffset + framesCount == steps)
                framesCount += 1;

            uint8_t *dataCInt8 = reinterpret_cast<uint8_t*>(dataC);
            H5Helper::floatC *dataCFloatC = reinterpret_cast<H5Helper::floatC*>(dataC);

            // For every frame
            for (hsize_t frame = 0; frame < framesCount; frame++) {
                hsize_t framesOffsetGlobal = framesOffset + frame;
                hsize_t frameOffset = frame * stepSize;
                if (log)
                    Helper::printDebugMsg("Decoding frame " + std::to_string(framesOffsetGlobal + 1) + "/" + std::to_string(steps + 1));
                // Decode steps
                for (hsize_t stepLocal = 0; stepLocal < oSize; stepLocal++) {
                    //hsize_t stepOffset = step * outputStepSize;
                    hsize_t stepsToWriteOffset = stepToWrite * outputStepSize;
                    // For every coefficient (space point) in frame
                    #pragma omp parallel for
                    for (hssize_t p = 0; p < hssize_t(outputStepSize); p++) {
                        hsize_t pOffset = compressHelper->getHarmonics() * hsize_t(p);
                        hsize_t sP = stepsToWriteOffset + hsize_t(p);
                        data[sP] = 0;

                        // For every harmonics
                        for (hsize_t ih = 0; ih < compressHelper->getHarmonics(); ih++) {
                            hsize_t pH = pOffset + ih;

                            if (stepLocal == 0) {
                                hsize_t pHC = pOffset + ih;

                                // Save last coefficients
                                lC[pH] = cC[pH];

                                if (flagC40bit) {
                                    pHC = pHC * 5;
                                }

                                // Copy first coefficients
                                if (framesOffsetGlobal == 0) {
                                    if (flagC40bit) {
                                        H5Helper::CompressHelper::convert40bToFloatC(&dataCInt8[pHC], lC[pH], kMaxExp);
                                        lC[pH] = conj(lC[pH]);
                                    } else {
                                        lC[pH] = conj(dataCFloatC[pHC]);
                                    }
                                    cC[pH] = lC[pH];
                                }
                                // Don't load last coefficient (duplicate)
                                if (framesOffsetGlobal < steps - 1) {
                                    hsize_t fPHC = frameOffset + pHC + stepSize;
                                    // Read coefficient
                                    if (flagC40bit) {
                                        H5Helper::CompressHelper::convert40bToFloatC(&dataCInt8[fPHC], cC[pH], kMaxExp);
                                        cC[pH] = conj(cC[pH]);
                                    } else {
                                        cC[pH] = conj(dataCFloatC[fPHC]);
                                    }
                                }
                            }

                            // Compute new point value
                            hsize_t sH = ih * bSize + stepLocal;
                            data[sP] += real(cC[pH] * compressHelper->getBE()[sH]) + real(lC[pH] * compressHelper->getBE_1()[sH]);
                        }
                        // TODO Min/max values
                        //H5Helper::checkOrSetMinMaxValue(minV, maxV, data[sP], minVIndex, maxVIndex, stepOffset + hsize_t(p));
                    }

                    step++;
                    stepToWrite++;

                    if (step % stepsToWrite == 0 || step == outputSteps) {
                        if (log)
                            Helper::printDebugMsgStart("Saving steps " + std::to_string(step - stepToWrite) + "-" + std::to_string(step) + "/" + std::to_string(outputSteps));

                        if (dims.getLength() == 4) // 4D dataset
                            dstDataset->writeDataset(H5Helper::Vector4D(step - stepToWrite, 0, 0, 0), H5Helper::Vector4D(stepToWrite, outputDims[1], outputDims[2], outputDims[3]), data, log);
                        else if (dims.getLength() == 3) // 3D dataset
                            dstDataset->writeDataset(H5Helper::Vector3D(0, step - stepToWrite, 0), H5Helper::Vector3D(1, stepToWrite, outputDims[2]), data, log);

                        if (log)
                            Helper::printDebugMsgEnd("saved");

                        stepToWrite = 0;
                    }
                }
            }
        }
        // Delete buffers
        delete[] data;
        data = nullptr;
        delete[] cC;
        cC = nullptr;
        delete[] lC;
        lC = nullptr;
    } else {
        // Not implemented yet
        Helper::printErrorMsg("Not implemented for such big datasets yet");
        _mm_free(dataC);
        dataC = nullptr;
        delete compressHelper;
        compressHelper = nullptr;
        getOutputFile()->closeDataset(dstDataset, log);
        return;
    }

    // Delete some memory
    _mm_free(dataC);
    dataC = nullptr;
    delete compressHelper;
    compressHelper = nullptr;

    // Copy attributes
    copyAttributes(srcDataset, dstDataset);

    // Set attributes
    dstDataset->setAttribute(H5Helper::MIN_ATTR, minV, log);
    dstDataset->setAttribute(H5Helper::MAX_ATTR, maxV, log);
    dstDataset->setAttribute(H5Helper::MIN_INDEX_ATTR, minVIndex, log);
    dstDataset->setAttribute(H5Helper::MAX_INDEX_ATTR, maxVIndex, log);
    dstDataset->setAttribute(H5Helper::C_TYPE_ATTR, "d", log);

    double t1 = H5Helper::getTime();
    Helper::printDebugTime("datasets decompression", t0, t1);

    getOutputFile()->closeDataset(dstDataset, log);
}
