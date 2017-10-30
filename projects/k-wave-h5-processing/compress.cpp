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
        outputDims[3] *= harmonics;
        stepSize = dims[1] * dims[2] * dims[3];
        outputStepSize = outputDims[1] * outputDims[2] * outputDims[3];
    } else if (dims.getLength() == 3) { // 3D dataset (defined by sensor mask)
        steps = HDF5Helper::Vector3D(dims).y();
        outputSteps = hsize_t(floor(float(steps) / oSize));
        outputDims[1] = outputSteps;
        outputDims[2] *= harmonics;
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

    //HDF5Helper::VectorOfDatasets datasetsPhi;
    //HDF5Helper::VectorOfDatasets datasetsK;

    HDF5Helper::Dataset *datasetPhi;
    HDF5Helper::Dataset *datasetK;

    // Create destination datasets
    /*for (hssize_t ih = 0; ih < hssize_t(harmonics); ih++) {
        getOutputFile()->createDatasetF(srcDataset->getName() + "_k_" + std::to_string(ih + 1), outputDims, chunkDims, true, log);
        getOutputFile()->createDatasetF(srcDataset->getName() + "_phi_" + std::to_string(ih + 1), outputDims, chunkDims, true, log);
        datasetsK.push_back(getOutputFile()->openDataset(srcDataset->getName() + "_k_" + std::to_string(ih + 1), log));
        datasetsPhi.push_back(getOutputFile()->openDataset(srcDataset->getName() + "_phi_" + std::to_string(ih + 1), log));
    }*/

    getOutputFile()->createDatasetF(srcDataset->getName() + "_k", outputDims, chunkDims, true, log);
    getOutputFile()->createDatasetF(srcDataset->getName() + "_phi", outputDims, chunkDims, true, log);
    datasetK = getOutputFile()->openDataset(srcDataset->getName() + "_k", log);
    datasetPhi = getOutputFile()->openDataset(srcDataset->getName() + "_phi", log);

    double t0 = HDF5Helper::getTime();

    // Variables for block reading
    float *data = 0;
    HDF5Helper::Vector offset;
    HDF5Helper::Vector count;
    float maxVK, maxVPhi;
    float minVK, minVPhi;
    hsize_t maxVKIndex = 0, maxVPhiIndex = 0;
    hsize_t minVKIndex = 0, minVPhiIndex = 0;
    bool first = true;

    // If we have enough memory - minimal for one full step in 3D space
    if (srcDataset->getNumberOfElmsToLoad() >= outputStepSize * 5) {

        // Complex buffers for accumulation
        floatC *sCTmp1 = new floatC[outputStepSize]();
        floatC *sCTmp2 = new floatC[outputStepSize]();

        hsize_t frame = 1;

        float *dataK = new float[outputStepSize]();
        float *dataPhi = new float[outputStepSize]();

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

                            // Computing amplitude
                            float k = abs(sC);

                            // Computing phase
                            float phi = arg(sC);

                            // Save complex coefficients
                            // Drop first "half" frame
                            if (frame > 1) {
                                dataK[pH] = k;
                                dataPhi[pH] = phi;

                                // Min/max values
                                HDF5Helper::checkOrSetMinMaxValue(first, minVK, maxVK, k);
                                HDF5Helper::checkOrSetMinMaxValue(first, minVPhi, maxVPhi, phi);

                                #pragma omp critical
                                {
                                    if (k == minVK)
                                        minVKIndex = (frame - 2) * outputStepSize + harmonics * hsize_t(p) + ih;
                                    if (k == maxVK)
                                        maxVKIndex = (frame - 2) * outputStepSize + harmonics * hsize_t(p) + ih;
                                    if (k == minVPhi)
                                        minVPhiIndex = (frame - 2) * outputStepSize + harmonics * hsize_t(p) + ih;
                                    if (k == maxVPhi)
                                        maxVPhiIndex = (frame - 2) * outputStepSize + harmonics * hsize_t(p) + ih;
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
                            //for (hsize_t h = 0; h < harmonics; h++) {
                                datasetK->writeDataset(HDF5Helper::Vector4D(frame - 2, 0, 0, 0), HDF5Helper::Vector4D(1, outputDims[1], outputDims[2], outputDims[3]), dataK);
                                datasetPhi->writeDataset(HDF5Helper::Vector4D(frame - 2, 0, 0, 0), HDF5Helper::Vector4D(1, outputDims[1], outputDims[2], outputDims[3]), dataPhi);
                            //}
                        } else if (dims.getLength() == 3) {
                            //for (hsize_t h = 0; h < harmonics; h++) {
                                datasetK->writeDataset(HDF5Helper::Vector3D(0, frame - 2, 0), HDF5Helper::Vector3D(1, 1, outputDims[2]), dataK);
                                datasetPhi->writeDataset(HDF5Helper::Vector3D(0, frame - 2, 0), HDF5Helper::Vector3D(1, 1, outputDims[2]), dataPhi);
                            //}
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
                    //for (hsize_t h = 0; h < harmonics; h++) {
                        datasetK->writeDataset(HDF5Helper::Vector4D(frame - 2, 0, 0, 0), HDF5Helper::Vector4D(1, outputDims[1], outputDims[2], outputDims[3]), dataK);
                        datasetPhi->writeDataset(HDF5Helper::Vector4D(frame - 2, 0, 0, 0), HDF5Helper::Vector4D(1, outputDims[1], outputDims[2], outputDims[3]), dataPhi);
                    //}
                } else if (dims.getLength() == 3) {
                    //for (hsize_t h = 0; h < harmonics; h++) {
                        datasetK->writeDataset(HDF5Helper::Vector3D(0, frame - 2, 0), HDF5Helper::Vector3D(1, 1, outputDims[2]), dataK);
                        datasetPhi->writeDataset(HDF5Helper::Vector3D(0, frame - 2, 0), HDF5Helper::Vector3D(1, 1, outputDims[2]), dataPhi);
                    //}
                }
                if (log) {
                    Helper::printDebugMsg("saved");
                }
            }

            delete[] data;
        }

        delete[] dataK;
        delete[] dataPhi;

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

        //for (hsize_t h = 0; h < harmonics; h++) {
            getOutputFile()->closeDataset(datasetK, log);
            getOutputFile()->closeDataset(datasetPhi, log);
        //}

        /*delete[] minVK;
        delete[] maxVK;
        delete[] minVKIndex;
        delete[] maxVKIndex;
        delete[] minVPhi;
        delete[] maxVPhi;
        delete[] minVPhiIndex;
        delete[] maxVPhiIndex;*/

        return;
    }

    // Delete some memory
    delete[] b;
    delete[] e;
    delete[] bE;
    delete[] bE_1;

    //for (hsize_t h = 0; h < harmonics; h++) {
        // Copy attributes
        copyAttributes(srcDataset, datasetK);
        copyAttributes(srcDataset, datasetPhi);

        // Set min/max values
        datasetK->setAttribute(HDF5Helper::MIN_ATTR, minVK, log);
        datasetK->setAttribute(HDF5Helper::MAX_ATTR, maxVK, log);
        datasetK->setAttribute(HDF5Helper::MIN_INDEX_ATTR, minVKIndex, log);
        datasetK->setAttribute(HDF5Helper::MAX_INDEX_ATTR, maxVKIndex, log);
        datasetK->setAttribute(HDF5Helper::SRC_DATASET_NAME_ATTR, srcDataset->getName(), log);
        datasetK->setAttribute(HDF5Helper::C_TYPE_ATTR, "k", log);
        datasetK->setAttribute(HDF5Helper::C_PERIOD_ATTR, period, log);
        datasetK->setAttribute(HDF5Helper::C_HARMONICS_ATTR, harmonics, log);
        datasetK->setAttribute(HDF5Helper::C_MOS_ATTR, mos, log);

        datasetPhi->setAttribute(HDF5Helper::MIN_ATTR, minVPhi, log);
        datasetPhi->setAttribute(HDF5Helper::MAX_ATTR, maxVPhi, log);
        datasetPhi->setAttribute(HDF5Helper::MIN_INDEX_ATTR, minVPhiIndex, log);
        datasetPhi->setAttribute(HDF5Helper::MAX_INDEX_ATTR, maxVPhiIndex, log);
        datasetPhi->setAttribute(HDF5Helper::SRC_DATASET_NAME_ATTR, srcDataset->getName(), log);
        datasetPhi->setAttribute(HDF5Helper::C_TYPE_ATTR, "phi", log);
        datasetPhi->setAttribute(HDF5Helper::C_PERIOD_ATTR, period, log);
        datasetPhi->setAttribute(HDF5Helper::C_HARMONICS_ATTR, harmonics, log);
        datasetPhi->setAttribute(HDF5Helper::C_MOS_ATTR, mos, log);
    //}

    /*delete minVK;
    delete maxVK;
    delete minVKIndex;
    delete maxVKIndex;
    delete minVPhi;
    delete maxVPhi;
    delete minVPhiIndex;
    delete maxVPhiIndex;*/

    double t1 = HDF5Helper::getTime();
    Helper::printDebugTime("dataset compression", t0, t1);

    //for (hsize_t h = 0; h < harmonics; h++) {
        getOutputFile()->closeDataset(datasetK, log);
        getOutputFile()->closeDataset(datasetPhi, log);
    //}
}
