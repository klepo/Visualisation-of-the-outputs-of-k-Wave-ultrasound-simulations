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
                compressDataset(dataset, false);
                count++;
                Helper::printDebugMsg("Compression of dataset " + dataset->getName() + "done");
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
    if (dims.getLength() == 4) { // 4D dataset
        steps = HDF5Helper::Vector4D(dims).w();
        outputSteps = hsize_t(floor(float(steps) / oSize));
        outputDims[0] = outputSteps;
        stepSize = outputDims[1] * outputDims[2] * outputDims[3];
    } else if (dims.getLength() == 3) { // 3D dataset (defined by sensor mask)
        steps = HDF5Helper::Vector3D(dims).y();
        outputSteps = hsize_t(floor(float(steps) / oSize));
        outputDims[1] = outputSteps;
        stepSize = outputDims[2];
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

    if (log) {
        Helper::printDebugTwoColumns2S("steps", steps);
        Helper::printDebugTwoColumns2S("outputSteps", outputSteps);
        Helper::printDebugTwoColumns2S("dims", dims);
        Helper::printDebugTwoColumns2S("outputDims", outputDims);
        Helper::printDebugTwoColumns2S("stepSize", stepSize);
        Helper::printDebugTwoColumns2S("chunkDims", chunkDims);
    }

    HDF5Helper::VectorOfDatasets datasetsPhi;
    HDF5Helper::VectorOfDatasets datasetsK;

    // Create destination datasets
    for (hssize_t ih = 0; ih < hssize_t(harmonics); ih++) {
        getOutputFile()->createDatasetF(srcDataset->getName() + "_k_" + std::to_string(ih + 1), outputDims, chunkDims, true, log);
        getOutputFile()->createDatasetF(srcDataset->getName() + "_phi_" + std::to_string(ih + 1), outputDims, chunkDims, true, log);
        datasetsK.push_back(getOutputFile()->openDataset(srcDataset->getName() + "_k_" + std::to_string(ih + 1), log));
        datasetsPhi.push_back(getOutputFile()->openDataset(srcDataset->getName() + "_phi_" + std::to_string(ih + 1), log));
    }

    double t0 = HDF5Helper::getTime();

    // Variables for block reading
    float *data = 0;
    HDF5Helper::Vector offset;
    HDF5Helper::Vector count;
    float *maxVK = new float[harmonics](), *maxVPhi = new float[harmonics]();
    float *minVK = new float[harmonics](), *minVPhi = new float[harmonics]();
    hsize_t *maxVKIndex = new hsize_t[harmonics](), *maxVPhiIndex = new hsize_t[harmonics]();
    hsize_t *minVKIndex = new hsize_t[harmonics](), *minVPhiIndex = new hsize_t[harmonics]();
    bool first = true;

    // If we have enough memory - minimal for one full step in 3D space
    if (srcDataset->getNumberOfElmsToLoad() >= stepSize * (1 + 4 * harmonics)) {

        // Complex buffers for accumulation
        floatC *sCTmp1 = new floatC[harmonics * stepSize]();
        floatC *sCTmp2 = new floatC[harmonics * stepSize]();

        hsize_t frame = 1;

        float *dataK = new float[harmonics * stepSize]();
        float *dataPhi = new float[harmonics * stepSize]();

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
                        sCTmp1[stepSize * ih + hsize_t(p)] += bE[ih * bSize + stepLocal] * data[step * stepSize + hsize_t(p)];
                        sCTmp2[stepSize * ih + hsize_t(p)] += bE_1[ih * bSize + stepLocal] * data[step * stepSize + hsize_t(p)];
                    }

                    // Check if we are at saving point
                    if ((stepLocal + 1) % oSize == 0) {
                        for (hsize_t ih = 0; ih < harmonics; ih++) {
                            floatC sC;
                            hsize_t pH = stepSize * ih + hsize_t(p);
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
                                HDF5Helper::checkOrSetMinMaxValue(first, minVK[ih], maxVK[ih], k);
                                HDF5Helper::checkOrSetMinMaxValue(first, minVPhi[ih], maxVPhi[ih], phi);

                                #pragma omp critical
                                {
                                    if (k == minVK[ih])
                                        minVKIndex[ih] = (frame - 2) * stepSize + hsize_t(p);
                                    if (k == maxVK[ih])
                                        maxVKIndex[ih] = (frame - 2) * stepSize + hsize_t(p);
                                    if (k == minVPhi[ih])
                                        minVPhiIndex[ih] = (frame - 2) * stepSize + hsize_t(p);
                                    if (k == maxVPhi[ih])
                                        maxVPhiIndex[ih] = (frame - 2) * stepSize + hsize_t(p);
                                }
                            }
                        }
                    }
                }

                if ((stepLocal + 1) % oSize == 0) {
                    if (frame > 1) {
                        if (log) {
                            Helper::printDebugMsgStart("Saving frame " + std::to_string(frame - 1));
                        }
                        if (dims.getLength() == 4) { // 4D dataset
                            for (hsize_t h = 0; h < harmonics; h++) {
                                datasetsK.at(h)->writeDataset(HDF5Helper::Vector4D(frame - 2, 0, 0, 0), HDF5Helper::Vector4D(1, dims[1], dims[2], dims[3]), &dataK[stepSize * h]);
                                datasetsPhi.at(h)->writeDataset(HDF5Helper::Vector4D(frame - 2, 0, 0, 0), HDF5Helper::Vector4D(1, dims[1], dims[2], dims[3]), &dataPhi[stepSize * h]);
                            }
                        } else if (dims.getLength() == 3) {
                            for (hsize_t h = 0; h < harmonics; h++) {
                                datasetsK.at(h)->writeDataset(HDF5Helper::Vector3D(0, frame - 2, 0), HDF5Helper::Vector3D(1, 1, dims[2]), &dataK[stepSize * h]);
                                datasetsPhi.at(h)->writeDataset(HDF5Helper::Vector3D(0, frame - 2, 0), HDF5Helper::Vector3D(1, 1, dims[2]), &dataPhi[stepSize * h]);
                            }
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
                    Helper::printDebugMsgStart("Saving frame " + std::to_string(frame - 1));
                }
                if (dims.getLength() == 4) { // 4D dataset
                    for (hsize_t h = 0; h < harmonics; h++) {
                        datasetsK.at(h)->writeDataset(HDF5Helper::Vector4D(frame - 2, 0, 0, 0), HDF5Helper::Vector4D(1, dims[1], dims[2], dims[3]), &dataK[stepSize * h]);
                        datasetsPhi.at(h)->writeDataset(HDF5Helper::Vector4D(frame - 2, 0, 0, 0), HDF5Helper::Vector4D(1, dims[1], dims[2], dims[3]), &dataPhi[stepSize * h]);
                    }
                } else if (dims.getLength() == 3) {
                    for (hsize_t h = 0; h < harmonics; h++) {
                        datasetsK.at(h)->writeDataset(HDF5Helper::Vector3D(0, frame - 2, 0), HDF5Helper::Vector3D(1, 1, dims[2]), &dataK[stepSize * h]);
                        datasetsPhi.at(h)->writeDataset(HDF5Helper::Vector3D(0, frame - 2, 0), HDF5Helper::Vector3D(1, 1, dims[2]), &dataPhi[stepSize * h]);
                    }
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

        for (hsize_t h = 0; h < harmonics; h++) {
            getOutputFile()->closeDataset(datasetsK.at(h), log);
            getOutputFile()->closeDataset(datasetsPhi.at(h), log);
        }

        delete[] minVK;
        delete[] maxVK;
        delete[] minVKIndex;
        delete[] maxVKIndex;
        delete[] minVPhi;
        delete[] maxVPhi;
        delete[] minVPhiIndex;
        delete[] maxVPhiIndex;

        return;
    }

    // Delete some memory
    delete[] b;
    delete[] e;
    delete[] bE;
    delete[] bE_1;

    for (hsize_t h = 0; h < harmonics; h++) {
        // Copy attributes
        copyAttributes(srcDataset, datasetsK.at(h));
        copyAttributes(srcDataset, datasetsPhi.at(h));

        // Set min/max values
        datasetsK.at(h)->setAttribute(HDF5Helper::MIN_ATTR, minVK[h], log);
        datasetsK.at(h)->setAttribute(HDF5Helper::MAX_ATTR, maxVK[h], log);
        datasetsK.at(h)->setAttribute(HDF5Helper::MIN_INDEX_ATTR, minVKIndex[h], log);
        datasetsK.at(h)->setAttribute(HDF5Helper::MAX_INDEX_ATTR, maxVKIndex[h], log);
        datasetsK.at(h)->setAttribute(HDF5Helper::SRC_DATASET_NAME_ATTR, srcDataset->getName(), log);
        datasetsK.at(h)->setAttribute(HDF5Helper::C_TYPE_ATTR, "k", log);
        datasetsK.at(h)->setAttribute(HDF5Helper::C_PERIOD_ATTR, period, log);
        datasetsK.at(h)->setAttribute(HDF5Helper::C_HARMONIC_ATTR, h + 1, log);
        datasetsK.at(h)->setAttribute(HDF5Helper::C_MOS_ATTR, mos, log);

        datasetsPhi.at(h)->setAttribute(HDF5Helper::MIN_ATTR, minVPhi[h], log);
        datasetsPhi.at(h)->setAttribute(HDF5Helper::MAX_ATTR, maxVPhi[h], log);
        datasetsPhi.at(h)->setAttribute(HDF5Helper::MIN_INDEX_ATTR, minVPhiIndex[h], log);
        datasetsPhi.at(h)->setAttribute(HDF5Helper::MAX_INDEX_ATTR, maxVPhiIndex[h], log);
        datasetsPhi.at(h)->setAttribute(HDF5Helper::SRC_DATASET_NAME_ATTR, srcDataset->getName(), log);
        datasetsPhi.at(h)->setAttribute(HDF5Helper::C_TYPE_ATTR, "phi", log);
        datasetsPhi.at(h)->setAttribute(HDF5Helper::C_PERIOD_ATTR, period, log);
        datasetsPhi.at(h)->setAttribute(HDF5Helper::C_HARMONIC_ATTR, h + 1, log);
        datasetsPhi.at(h)->setAttribute(HDF5Helper::C_MOS_ATTR, mos, log);
    }

    delete[] minVK;
    delete[] maxVK;
    delete[] minVKIndex;
    delete[] maxVKIndex;
    delete[] minVPhi;
    delete[] maxVPhi;
    delete[] minVPhiIndex;
    delete[] maxVPhiIndex;

    double t1 = HDF5Helper::getTime();
    Helper::printDebugTime("dataset compression", t0, t1);

    for (hsize_t h = 0; h < harmonics; h++) {
        getOutputFile()->closeDataset(datasetsK.at(h), log);
        getOutputFile()->closeDataset(datasetsPhi.at(h), log);
    }
}
