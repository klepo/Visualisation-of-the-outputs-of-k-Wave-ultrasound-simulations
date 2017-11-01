/**
 * @file        compress.cpp
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        8  September 2016 (created) \n
 *              11 September 2017 (updated)
 *
 * @brief       The implementation file containing Compress class definition.
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

#include "compress.h"

/**
 * @brief Creates Compress object
 * @param[in] outputFile Output file
 * @param[in] dtsForPcs Datasets for processing
 * @param[in] settings Processing settings
 */
Compress::Compress(HDF5Helper::File *outputFile, DtsForPcs *dtsForPcs, Settings *settings)
    : Processing(outputFile, dtsForPcs, settings)
{

}

/**
 * @brief Executes processing
 */
void Compress::execute()
{
    std::vector<HDF5Helper::DatasetType> types = {
        HDF5Helper::DatasetType::TIME_STEPS_MASK,
        HDF5Helper::DatasetType::CUBOID,
        HDF5Helper::DatasetType::CUBOID_ATTR
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
                Helper::printDebugMsg("Compression of dataset " + dataset->getName());
                compressDataset(dataset, getSettings()->getFlagLog());
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
 */
void Compress::compressDataset(HDF5Helper::Dataset *srcDataset, bool log)
{
    if (!getSettings()->getPeriod()) {
        Helper::printErrorMsg("No known period for compression");
        return;
    }

    // First encoding parameter - multiple of overlap size
    hsize_t mos = getSettings()->getMOS();

    // Second encoding parameter - period
    hsize_t period = getSettings()->getPeriod();

    // Harmonic frequencies
    hsize_t harmonics = getSettings()->getHarmonic();

    if (log) {
         Helper::printDebugMsg("Compression with period " + std::to_string(period) + " steps "+ "and " + std::to_string(harmonics) + " harmonic frequencies");
    }

    // Overlap size
    hsize_t oSize = period * mos;
    // Base size
    hsize_t bSize = oSize * 2 + 1;

    // Get dims
    HDF5Helper::Vector dims = srcDataset->getDims();
    HDF5Helper::Vector outputDims = dims;

    // Compute steps, step size and output dims
    hsize_t steps = 0;
    hsize_t outputSteps = 0;
    hsize_t stepSize = 0;
    hsize_t outputStepSize = 0;
    if (dims.getLength() == 4) { // 4D dataset
        steps = HDF5Helper::Vector4D(dims).w();
        outputSteps = hsize_t(floor(float(steps) / oSize));
        outputDims[0] = outputSteps;
        outputDims[3] *= harmonics * 2;
        stepSize = dims[1] * dims[2] * dims[3];
        outputStepSize = outputDims[1] * outputDims[2] * outputDims[3];
    } else if (dims.getLength() == 3) { // 3D dataset (defined by sensor mask)
        steps = HDF5Helper::Vector3D(dims).y();
        outputSteps = hsize_t(floor(float(steps) / oSize));
        outputDims[1] = outputSteps;
        outputDims[2] *= harmonics * 2;
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
    generateFunctions(bSize, oSize, period, harmonics, b, e, bE, bE_1, true);

    // Chunk dims
    HDF5Helper::Vector chunkDims(srcDataset->getChunkDims());

    if (dims.getLength() == 4) { // 4D dataset
        chunkDims[3] *= harmonics;
    } else if (dims.getLength() == 3) { // 3D dataset (defined by sensor mask)
        chunkDims[2] *= harmonics;
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

    HDF5Helper::Dataset *dstDataset;

    getOutputFile()->createDatasetF(srcDataset->getName() + "_c", outputDims, chunkDims, true, log);
    dstDataset = getOutputFile()->openDataset(srcDataset->getName() + "_c", log);

    double t0 = HDF5Helper::getTime();

    // Variables for block reading
    float *data = 0;
    HDF5Helper::Vector offset;
    HDF5Helper::Vector count;
    float maxV, minV;
    hsize_t maxVIndex = 0, minVIndex = 0;
    bool first = true;

    // If we have enough memory - minimal for one full step in 3D space
    if (srcDataset->getNumberOfElmsToLoad() >= outputStepSize * 2) {

        // Complex buffers for accumulation
        floatC *sCTmp1 = new floatC[outputStepSize / 2]();
        floatC *sCTmp2 = new floatC[outputStepSize / 2]();

        hsize_t frame = 1;

        float *dataC = new float[outputStepSize]();

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
                //Helper::printDebugMsgStart("Encoding step " + std::to_string(stepsOffset + step));

                // Compute local index
                hsize_t stepLocal = (stepsOffset + step) % (bSize - 1);

                // For every point
                #pragma omp parallel for
                for (hssize_t p = 0; p < hssize_t(stepSize); p++) {
                    // Correlation step
                    for (hsize_t ih = 0; ih < harmonics; ih++) {
                        sCTmp1[harmonics * hsize_t(p) + ih] += bE[ih * bSize + stepLocal] * data[step * stepSize + hsize_t(p)];
                        sCTmp2[harmonics * hsize_t(p) + ih] += bE_1[ih * bSize + stepLocal] * data[step * stepSize + hsize_t(p)];
                    }

                    // Check if we are at saving point
                    if ((stepLocal + 1) % oSize == 0) {
                        for (hsize_t ih = 0; ih < harmonics; ih++) {
                            floatC sC;
                            hsize_t pH = harmonics * hsize_t(p) + ih;
                            // Select accumulated value
                            if ((frame % 2) == 0) {
                                sC = sCTmp1[pH];
                                sCTmp1[pH] = 0;
                            } else {
                                sC = sCTmp2[pH];
                                sCTmp2[pH] = 0;
                            }

                            // Save complex coefficients
                            // Drop first "half" frame
                            if (frame > 1) {
                                dataC[harmonics * hsize_t(p) + 2 * ih] = real(sC);
                                dataC[harmonics * hsize_t(p) + 2 * ih + 1] = imag(sC);

                                // Min/max values
                                HDF5Helper::checkOrSetMinMaxValue(first, minV, maxV, real(sC));
                                HDF5Helper::checkOrSetMinMaxValue(first, minV, maxV, imag(sC));

                                #pragma omp critical
                                {
                                    if (real(sC) == minV)
                                        minVIndex = (frame - 2) * outputStepSize + harmonics * hsize_t(p) + 2 * ih;
                                    if (real(sC) == maxV)
                                        maxVIndex = (frame - 2) * outputStepSize + harmonics * hsize_t(p) + 2 * ih;
                                    if (imag(sC) == minV)
                                        minVIndex = (frame - 2) * outputStepSize + harmonics * hsize_t(p) + 2 * ih + 1;
                                    if (imag(sC) == maxV)
                                        maxVIndex = (frame - 2) * outputStepSize + harmonics * hsize_t(p) + 2 * ih + 1;
                                }
                            }
                        }
                    }
                }

                if ((stepLocal + 1) % oSize == 0) {
                    if (frame > 1) {
                        if (log) {
                            Helper::printDebugMsgStart("Saving frame " + std::to_string(frame - 1) + " / " + std::to_string(outputSteps));
                        }
                        if (dims.getLength() == 4) { // 4D dataset
                            dstDataset->writeDataset(HDF5Helper::Vector4D(frame - 2, 0, 0, 0), HDF5Helper::Vector4D(1, outputDims[1], outputDims[2], outputDims[3]), dataC);
                        } else if (dims.getLength() == 3) {
                            dstDataset->writeDataset(HDF5Helper::Vector3D(0, frame - 2, 0), HDF5Helper::Vector3D(1, 1, outputDims[2]), dataC);
                        }
                        if (log) {
                            Helper::printDebugMsg("saved");
                        }
                    }
                    // Increment frame
                    frame++;
                }
                //Helper::printDebugMsg("encoded");
            }

            // Last frame (copy last)
            if (frame - 1 == outputSteps) {
                if (log) {
                    Helper::printDebugMsgStart("Saving frame " + std::to_string(frame - 1) + " / " + std::to_string(outputSteps));
                }
                if (dims.getLength() == 4) { // 4D dataset
                    dstDataset->writeDataset(HDF5Helper::Vector4D(frame - 2, 0, 0, 0), HDF5Helper::Vector4D(1, outputDims[1], outputDims[2], outputDims[3]), dataC);
                } else if (dims.getLength() == 3) {
                    dstDataset->writeDataset(HDF5Helper::Vector3D(0, frame - 2, 0), HDF5Helper::Vector3D(1, 1, outputDims[2]), dataC);
                }
                if (log) {
                    Helper::printDebugMsg("saved");
                }
            }

            delete[] data;
        }

        delete[] dataC;

        // Delete complex buffers
        delete[] sCTmp1;
        delete[] sCTmp2;
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

    // Set min/max values
    dstDataset->setAttribute(HDF5Helper::MIN_ATTR, minV, log);
    dstDataset->setAttribute(HDF5Helper::MAX_ATTR, maxV, log);
    dstDataset->setAttribute(HDF5Helper::MIN_INDEX_ATTR, minVIndex, log);
    dstDataset->setAttribute(HDF5Helper::MAX_INDEX_ATTR, maxVIndex, log);
    dstDataset->setAttribute(HDF5Helper::SRC_DATASET_NAME_ATTR, srcDataset->getName(), log);
    dstDataset->setAttribute(HDF5Helper::C_TYPE_ATTR, "c", log);
    dstDataset->setAttribute(HDF5Helper::C_PERIOD_ATTR, period, log);
    dstDataset->setAttribute(HDF5Helper::C_HARMONICS_ATTR, harmonics, log);
    dstDataset->setAttribute(HDF5Helper::C_MOS_ATTR, mos, log);

    double t1 = HDF5Helper::getTime();
    Helper::printDebugTime("dataset compression", t0, t1);

    getOutputFile()->closeDataset(dstDataset, log);
}
