/**
 * @file        decompress.cpp
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        8  September 2016 (created) \n
 *              11 September 2017 (updated)
 *
 * @brief       The implementation file containing Decompress class definition.
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

#include "decompress.h"

/**
 * @brief Creates Decompress object
 * @param[in] outputFile Output file
 * @param[in] dtsForPcs Datasets for processing
 * @param[in] settings Processing settings
 */
Decompress::Decompress(HDF5Helper::File *outputFile, DtsForPcs *dtsForPcs, Settings *settings)
    : Processing(outputFile, dtsForPcs, settings)
{

}

/**
 * @brief Executes processing
 */
void Decompress::execute()
{
    std::vector<HDF5Helper::DatasetType> types = {
        HDF5Helper::DatasetType::TIME_STEPS_C_MASK,
        HDF5Helper::DatasetType::CUBOID_C,
        HDF5Helper::DatasetType::CUBOID_ATTR_C
    };
    // TODO downsampled datasets

    try {
        HDF5Helper::MapOfDatasets map = getDtsForPcs()->getDatasets();
        hsize_t sensorMaskSize = getDtsForPcs()->getSensorMaskSize();
        int count = 0;
        for (HDF5Helper::MapOfDatasetsIt it = map.begin(); it != map.end(); ++it) {
            HDF5Helper::Dataset *dataset = it->second;
            HDF5Helper::DatasetType datasetType = dataset->getType(sensorMaskSize);
            if (checkDatasetType(datasetType, types)) {
                Helper::printDebugMsg("Decompression of dataset " + dataset->getName());
                decompressDatasets(dataset, getSettings()->getFlagLog());
                count++;
                Helper::printDebugMsg("Decompression of dataset done");
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
 * @param[in] srcDatasetsFi Source dataset Phi
 * @param[in] srcDatasetsK Source dataset K
 */
void Decompress::decompressDatasets(HDF5Helper::Dataset *srcDataset, bool log)
{
    // First decoding parameter - multiple of overlap size
    hsize_t mos = 1;
    if (srcDataset->hasAttribute(HDF5Helper::C_MOS_ATTR))
        mos = srcDataset->readAttributeI(HDF5Helper::C_MOS_ATTR, log);

    // Second decoding parameter - period
    hsize_t period = srcDataset->readAttributeI(HDF5Helper::C_PERIOD_ATTR, log);

    // Harmonic frequencies
    hsize_t harmonics = srcDataset->readAttributeI(HDF5Helper::C_HARMONICS_ATTR, log);

    if (log) {
        Helper::printDebugMsg("Decompression with period " + std::to_string(period) + " steps "+ "and " + std::to_string(harmonics) + " harmonic frequencies");
    }

    // Overlap size
    hsize_t oSize = period * mos;
    // Base size
    hsize_t bSize = oSize * 2 + 1;

    // Get dims
    HDF5Helper::Vector dims = srcDataset->getDims();
    HDF5Helper::Vector outputDims = dims;

    // TODO - check same dims of srcDatasetPhi and srcDatasetK

    // Compute steps, step size and output dims
    hsize_t steps = 0;
    hsize_t outputSteps = 0;
    hsize_t stepSize = 0;
    hsize_t outputStepSize = 0;
    if (dims.getLength() == 4) { // 4D dataset
        steps = HDF5Helper::Vector4D(dims).w();
        outputSteps = (steps + 1) * oSize ;
        outputDims[0] = outputSteps;
        outputDims[3] /= harmonics * 2;
        stepSize = dims[1] * dims[2] * dims[3];
        outputStepSize = outputDims[1] * outputDims[2] * outputDims[3];
    } else if (dims.getLength() == 3) { // 3D dataset (defined by sensor mask)
        steps = HDF5Helper::Vector3D(dims).y();
        outputSteps = (steps + 1) * oSize;
        outputDims[1] = outputSteps;
        outputDims[2] /= harmonics * 2;
        stepSize = dims[2];
        outputStepSize = outputDims[2];
    } else { // Something wrong.
        Helper::printErrorMsg("Something wrong with dataset dims");
        return;
    }

    // Memory for helper functions data, 2d arrays for harmonics
    float *b = new float[bSize]();
    floatC *e = new floatC[harmonics * bSize]();
    floatC *bE = new floatC[harmonics * bSize]();
    floatC *bE_1 = new floatC[harmonics * bSize]();

    // Fill functions
    generateFunctions(bSize, oSize, period, harmonics, b, e, bE, bE_1);

    // Chunk dims
    HDF5Helper::Vector chunkDims(srcDataset->getChunkDims());

    if (dims.getLength() == 4) { // 4D dataset
        chunkDims[3] /= harmonics * 2;
    } else if (dims.getLength() == 3) { // 3D dataset (defined by sensor mask)
        chunkDims[2] /= harmonics * 2;
    } else { // Something wrong.
        Helper::printErrorMsg("Something wrong with dataset dims");
        return;
    }

    if (log) {
        Helper::printDebugTwoColumns2S("steps", steps);
        Helper::printDebugTwoColumns2S("outputSteps", outputSteps);
        Helper::printDebugTwoColumns2S("dims", dims);
        Helper::printDebugTwoColumns2S("outputDims", outputDims);
        Helper::printDebugTwoColumns2S("stepSize", stepSize);
        Helper::printDebugTwoColumns2S("outputStepSize", outputStepSize);
        Helper::printDebugTwoColumns2S("chunkDims", chunkDims);
    }

    // Create destination dataset
    std::string srcName = srcDataset->readAttributeS(HDF5Helper::SRC_DATASET_NAME_ATTR, log);
    getOutputFile()->createDatasetF(srcName + "_d", outputDims, chunkDims, true, log);
    HDF5Helper::Dataset *dstDataset = getOutputFile()->openDataset(srcName + "_d", log);

    double t0 = HDF5Helper::getTime();

    // Variables for block reading
    float *dataC = 0;
    HDF5Helper::Vector offset;
    HDF5Helper::Vector count;
    float maxV, minV;
    hsize_t maxVIndex = 0, minVIndex = 0;
    bool first = true;

    // If we have enough memory - minimal for one full step in 3D space
    if (srcDataset->getFile()->getNumberOfElmsToLoad() >= stepSize * 2) {

        // Buffers for last coefficients
        floatC *cC = new floatC[stepSize / 2]();
        floatC *lC = new floatC[stepSize / 2]();
        // Output buffer
        float *data = new float[outputStepSize]();

        hsize_t step = 0;

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

            // Because of last coefficient duplication
            bool lastFlag = false;
            if (framesOffset + framesCount == steps) {
                lastFlag = true;
                framesCount++;
            }

            // For every frame
            for (hsize_t frame = 0; frame < framesCount; frame++) {
                //Helper::printDebugMsgStart("Decoding frame " + std::to_string(frameDst));

                // Decode steps
                for (hsize_t stepLocal = 0; stepLocal < oSize; stepLocal++) {
                    //double t0 = HDF5Helper::getTime();

                    // For every coefficient (space point) in step
                    #pragma omp parallel for
                    for (hssize_t p = 0; p < hssize_t(outputStepSize); p++) {
                        if (stepLocal == 0) {
                            // Save last coefficients
                            for (hsize_t ih = 0; ih < harmonics; ih++) {
                                hsize_t pH = harmonics * hsize_t(p) + ih;
                                hsize_t pHC = 2 * harmonics * hsize_t(p) + 2 * ih;
                                lC[pH] = cC[pH];

                                // Copy first coefficient
                                if (frame == 0) {
                                    lC[pH] = floatC(dataC[pHC], dataC[pHC + 1]);
                                }

                                if (frame == framesCount - 1 && lastFlag) {
                                    hsize_t pF1 = (frame - 1) * stepSize + 2 * harmonics * hsize_t(p) + 2 * ih;
                                    // Duplicate last coefficient
                                    cC[pH] = floatC(dataC[pF1], dataC[pF1 + 1]);
                                } else {
                                    hsize_t pF = frame * stepSize + 2 * harmonics * hsize_t(p) + 2 * ih;
                                    // Read coefficient
                                    cC[pH] = floatC(dataC[pF], dataC[pF + 1]);
                                }
                            }
                        }

                        // Compute new point value
                        data[p] = 0;
                        for (hsize_t ih = 0; ih < harmonics; ih++) {
                            hsize_t pH = harmonics * hsize_t(p) + ih;
                            hsize_t sH = ih * bSize + stepLocal;
                            data[p] += real(conj(cC[pH]) * bE[sH]) + real(conj(lC[pH]) * bE_1[sH]);
                        }

                        // Min/max values
                        HDF5Helper::checkOrSetMinMaxValue(first, minV, maxV, data[p]);
                        #pragma omp critical
                        {
                            if (data[p] == minV)
                                minVIndex = (step * oSize + stepLocal) * outputStepSize + hsize_t(p);
                            if (data[p] == maxV)
                                maxVIndex = (step * oSize + stepLocal) * outputStepSize + hsize_t(p);
                        }
                    }
                    //Helper::printDebugTime("one step decoding", t0, t1);

                    if (log) {
                        Helper::printDebugMsgStart("Saving step " + std::to_string(step * oSize + stepLocal + 1) + "/" + std::to_string(outputSteps));
                    }
                    if (dims.getLength() == 4) { // 4D dataset
                        dstDataset->writeDataset(HDF5Helper::Vector4D(step * oSize + stepLocal, 0, 0, 0), HDF5Helper::Vector4D(1, outputDims[1], outputDims[2], outputDims[3]), data);
                    } else if (dims.getLength() == 3) {
                        dstDataset->writeDataset(HDF5Helper::Vector3D(0, step * oSize + stepLocal, 0), HDF5Helper::Vector3D(1, 1, outputDims[2]), data);
                    }
                    if (log) {
                        Helper::printDebugMsg("saved");
                    }
                }
                step++;
                //Helper::printDebugMsg("encoded");
            }
            delete[] dataC;
        }
        // Delete buffers
        delete[] data;
        delete[] cC;
        delete[] lC;
    } else {
        // Not implemented yet
        Helper::printErrorMsg("Not implemented for such big datasets yet");
        delete[] b;
        delete[] e;
        delete[] bE;
        delete[] bE_1;
        getOutputFile()->closeDataset(dstDataset, log);
        return;
    }

    // Delete some memory
    delete[] b;
    delete[] e;
    delete[] bE;
    delete[] bE_1;

    // Copy attributes
    copyAttributes(srcDataset, dstDataset);

    // Set attributes
    dstDataset->setAttribute(HDF5Helper::MIN_ATTR, minV, log);
    dstDataset->setAttribute(HDF5Helper::MAX_ATTR, maxV, log);
    dstDataset->setAttribute(HDF5Helper::MIN_INDEX_ATTR, minVIndex, log);
    dstDataset->setAttribute(HDF5Helper::MAX_INDEX_ATTR, maxVIndex, log);
    dstDataset->setAttribute(HDF5Helper::C_TYPE_ATTR, "d", log);

    double t1 = HDF5Helper::getTime();
    Helper::printDebugTime("datasets decompression", t0, t1);

    getOutputFile()->closeDataset(dstDataset, log);
}
