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
Compress::Compress(H5Helper::File *outputFile, DtsForPcs *dtsForPcs, Settings *settings)
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
 */
void Compress::compressDataset(H5Helper::Dataset *srcDataset, bool log)
{
    if (!getSettings()->getPeriod()) {
        Helper::printErrorMsg("No known period for compression");
        return;
    }

    // First encoding parameter - multiple of overlap size
    // Second encoding parameter - period
    // Third encoding parameter - number of harmonic frequencies
    H5Helper::CompressHelper *compressHelper = new H5Helper::CompressHelper(getSettings()->getPeriod(), getSettings()->getMOS(), getSettings()->getHarmonic(), true);

    if (log)
         Helper::printDebugMsg("Compression with period " + std::to_string(compressHelper->getPeriod()) + " steps "+ "and " + std::to_string(compressHelper->getHarmonics()) + " harmonic frequencies");

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
        outputSteps = hsize_t(floor(float(steps) / oSize)) - 1;
        outputDims[0] = outputSteps;
        outputDims[3] *= compressHelper->getHarmonics() * 2;
        stepSize = dims[1] * dims[2] * dims[3];
        outputStepSize = outputDims[1] * outputDims[2] * outputDims[3];
    } else if (dims.getLength() == 3) { // 3D dataset (defined by sensor mask)
        steps = H5Helper::Vector3D(dims).y();
        outputSteps = hsize_t(floor(float(steps) / oSize)) - 1;
        outputDims[1] = outputSteps;
        outputDims[2] *= compressHelper->getHarmonics() * 2;
        stepSize = dims[2];
        outputStepSize = outputDims[2];
    } else { // Something wrong.
        Helper::printErrorMsg("Something wrong with dataset dims");
        return;
    }

    // Chunk dims
    H5Helper::Vector chunkDims(srcDataset->getChunkDims());
    if (dims.getLength() == 4) { // 4D dataset
        chunkDims[3] *= compressHelper->getHarmonics() * 2;
    } else if (dims.getLength() == 3) { // 3D dataset (defined by sensor mask)
        chunkDims[2] *= compressHelper->getHarmonics() * 2;
    } else { // Something wrong.
        Helper::printErrorMsg("Something wrong with dataset dims");
        return;
    }

    Helper::printDebugTwoColumns2S("steps", steps);
    Helper::printDebugTwoColumns2S("outputSteps", outputSteps);
    Helper::printDebugTwoColumns2S("dims", dims);
    Helper::printDebugTwoColumns2S("outputDims", outputDims);
    Helper::printDebugTwoColumns2S("stepSize", stepSize);
    Helper::printDebugTwoColumns2S("outputStepSize", outputStepSize);
    Helper::printDebugTwoColumns2S("chunkDims", chunkDims);

    // Create destination dataset
    getOutputFile()->createDatasetF(srcDataset->getName() + "_c", outputDims, chunkDims, true, log);
    H5Helper::Dataset *dstDataset = getOutputFile()->openDataset(srcDataset->getName() + "_c", log);

    double t0 = H5Helper::getTime();

    // Variables for block reading
    float *data = 0;
    H5Helper::Vector offset;
    H5Helper::Vector count;
    float maxV, minV;
    hsize_t maxVIndex = 0, minVIndex = 0;
    bool first = true;

    // If we have enough memory - minimal for one full step in 3D space
    if (srcDataset->getNumberOfElmsToLoad() >= outputStepSize * 3) {
        // Complex buffers for accumulation
        H5Helper::floatC *sCTmp1 = new H5Helper::floatC[outputStepSize / 2]();
        H5Helper::floatC *sCTmp2 = new H5Helper::floatC[outputStepSize / 2]();
        // Output buffer
        H5Helper::floatC *dataC = new H5Helper::floatC[outputStepSize / 2]();

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
                // Compute local index
                hsize_t stepLocal = (stepsOffset + step) % (bSize - 1);
                bool savingFlag = ((stepLocal + 1) % oSize == 0) ? true : false;
                bool oddFrameFlag = ((frame + 1) % 2 == 0) ? true : false;

                // For every point
                //#pragma omp parallel for
                for (hssize_t p = 0; p < hssize_t(stepSize); p++) {
                    hsize_t sP = step * stepSize + hsize_t(p);
                    hsize_t pOffset = compressHelper->getHarmonics() * hsize_t(p);

                    //For every harmonics
                    for (hssize_t ih = 0; ih < hssize_t(compressHelper->getHarmonics()); ih++) {
                        hsize_t pH = pOffset + ih;

                        // Correlation step
                        sCTmp1[pH] += compressHelper->getBE()[ih * bSize + stepLocal] * data[sP];
                        sCTmp2[pH] += compressHelper->getBE_1()[ih * bSize + stepLocal] * data[sP];

                        // Check if we are at saving point
                        if (savingFlag) {
                            H5Helper::floatC sC;

                            // Select accumulated value
                            if (oddFrameFlag) {
                                sC = sCTmp1[pH];
                                sCTmp1[pH] = 0;
                            } else {
                                sC = sCTmp2[pH];
                                sCTmp2[pH] = 0;
                            }

                            // Drop first "half" frame
                            if (frame > 0) {
                                hsize_t pHC = 2 * pOffset + 2 * ih;

                                // Save complex coefficients
                                dataC[pH] = sC;

                                // Min/max values
                                H5Helper::checkOrSetMinMaxValue(first, minV, maxV, real(sC), minVIndex, maxVIndex, (frame - 1) * outputStepSize + pHC);
                                H5Helper::checkOrSetMinMaxValue(first, minV, maxV, imag(sC), minVIndex, maxVIndex, (frame - 1) * outputStepSize + pHC + 1);
                            }
                        }
                    }
                }

                if (savingFlag) {
                    if (frame > 0) {
                        if (log)
                            Helper::printDebugMsgStart("Saving frame " + std::to_string(frame) + "/" + std::to_string(outputSteps));

                        if (dims.getLength() == 4) // 4D dataset
                            dstDataset->writeDataset(H5Helper::Vector4D(frame - 1, 0, 0, 0), H5Helper::Vector4D(1, outputDims[1], outputDims[2], outputDims[3]), reinterpret_cast<float *>(dataC));
                        else if (dims.getLength() == 3) // 3D dataset
                            dstDataset->writeDataset(H5Helper::Vector3D(0, frame - 1, 0), H5Helper::Vector3D(1, 1, outputDims[2]), reinterpret_cast<float *>(dataC));

                        if (log)
                            Helper::printDebugMsg("saved");
                    }
                    // Increment frame
                    frame++;
                }
            }
            delete[] data;
        }
        // Delete buffers
        delete[] dataC;
        delete[] sCTmp1;
        delete[] sCTmp2;
    } else {
        // Not implemented yet
        Helper::printErrorMsg("Not implemented for such big datasets yet");
        delete compressHelper;
        getOutputFile()->closeDataset(dstDataset, log);
        return;
    }

    // Delete some memory
    delete compressHelper;

    // Copy attributes
    copyAttributes(srcDataset, dstDataset);

    // Set attributes
    dstDataset->setAttribute(H5Helper::MIN_ATTR, minV, log);
    dstDataset->setAttribute(H5Helper::MAX_ATTR, maxV, log);
    dstDataset->setAttribute(H5Helper::MIN_INDEX_ATTR, minVIndex, log);
    dstDataset->setAttribute(H5Helper::MAX_INDEX_ATTR, maxVIndex, log);
    dstDataset->setAttribute(H5Helper::C_HARMONICS_ATTR, getSettings()->getHarmonic(), log);
    dstDataset->setAttribute(H5Helper::C_TYPE_ATTR, "c", log);
    dstDataset->setAttribute(H5Helper::C_PERIOD_ATTR, getSettings()->getPeriod(), log);
    dstDataset->setAttribute(H5Helper::C_MOS_ATTR, getSettings()->getMOS(), log);
    dstDataset->setAttribute(H5Helper::SRC_DATASET_NAME_ATTR, srcDataset->getName(), log);

    double t1 = H5Helper::getTime();
    Helper::printDebugTime("dataset compression", t0, t1);

    getOutputFile()->closeDataset(dstDataset, log);
}
