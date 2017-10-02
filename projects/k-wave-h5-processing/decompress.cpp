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
        HDF5Helper::DatasetType::TIME_STEPS_PHI_MASK,
        HDF5Helper::DatasetType::CUBOID_PHI,
        HDF5Helper::DatasetType::CUBOID_ATTR_PHI
    };
    // TODO downsampled datasets
    //HDF5Helper::datasetType::CUBOID_DWNSMPL_PHI
    //HDF5Helper::datasetType::CUBOID_ATTR_DWNSMPL_PHI

    try {
        HDF5Helper::MapOfDatasets map = getDtsForPcs()->getDatasets();
        hsize_t sensorMaskSize = getDtsForPcs()->getSensorMaskSize();
        int count = 0;
        HDF5Helper::VectorOfDatasets datasetsFi;
        HDF5Helper::VectorOfDatasets datasetsK;

        for (HDF5Helper::MapOfDatasetsIt it = map.begin(); it != map.end(); ++it) {
            HDF5Helper::Dataset *datasetFi = it->second;
            HDF5Helper::DatasetType datasetType = datasetFi->getType(sensorMaskSize);

            if (checkDatasetType(datasetType, types)) {
                std::string nameFi = datasetFi->readAttributeS(HDF5Helper::SRC_DATASET_NAME_ATTR, false);
                hsize_t periodFi = datasetFi->readAttributeI(HDF5Helper::C_PERIOD_ATTR, false);
                hsize_t harmonicFi = 1;
                if (datasetFi->hasAttribute(HDF5Helper::C_HARMONIC_ATTR)) {
                    harmonicFi = datasetFi->readAttributeI(HDF5Helper::C_HARMONIC_ATTR, false);
                }

                HDF5Helper::Dataset *datasetK = 0;

                for (HDF5Helper::MapOfDatasetsIt it2 = map.begin(); it2 != map.end(); ++it2) {
                    HDF5Helper::Dataset *dataset = it2->second;
                    HDF5Helper::DatasetType datasetKType = dataset->getType(sensorMaskSize);

                    if ((datasetKType == HDF5Helper::DatasetType::TIME_STEPS_K_MASK && datasetType == HDF5Helper::DatasetType::TIME_STEPS_PHI_MASK)
                            || (datasetKType == HDF5Helper::DatasetType::CUBOID_K && datasetType == HDF5Helper::DatasetType::CUBOID_PHI)
                            || (datasetKType == HDF5Helper::DatasetType::CUBOID_ATTR_K && datasetType == HDF5Helper::DatasetType::CUBOID_ATTR_PHI)
                            ) {
                        hsize_t harmonicK = 1;
                        if (dataset->hasAttribute(HDF5Helper::C_HARMONIC_ATTR)) {
                            harmonicK = dataset->readAttributeI(HDF5Helper::C_HARMONIC_ATTR, false);
                        }
                        // TODO Check C_MOS_ATTR, number of coefficients, same periods, harmonic indices
                        if (periodFi == dataset->readAttributeI(HDF5Helper::C_PERIOD_ATTR, false)
                                && nameFi == dataset->readAttributeS(HDF5Helper::SRC_DATASET_NAME_ATTR, false)
                                && harmonicK == harmonicFi
                                ) {
                            datasetK = dataset;
                            break;
                        }
                    }
                }

                if (datasetK) {
                    datasetsFi.push_back(datasetFi);
                    datasetsK.push_back(datasetK);
                    count++;
                }
            }
        }

        if (count == 0) {
            Helper::printErrorMsg("No datasets for decompression in simulation output file");
        } else {
            Helper::printDebugMsg("Decompression of datasets ...");
            decompressDatasets(datasetsFi, datasetsK, false);
            Helper::printDebugMsg("Decompression of datasets done");
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
void Decompress::decompressDatasets(HDF5Helper::VectorOfDatasets srcDatasetsPhi, HDF5Helper::VectorOfDatasets srcDatasetsK, bool log)
{
    // First decoding parameter - multiple of overlap size
    hsize_t mos = 1;
    if (srcDatasetsPhi.at(0)->hasAttribute(HDF5Helper::C_MOS_ATTR))
        mos = srcDatasetsPhi.at(0)->readAttributeI(HDF5Helper::C_MOS_ATTR, log);

    // Second decoding parameter - period
    hsize_t period = srcDatasetsPhi.at(0)->readAttributeI(HDF5Helper::C_PERIOD_ATTR, log);

    // Harmonic frequencies
    hsize_t harmonics = srcDatasetsPhi.size();

    if (log) {
        Helper::printDebugMsg("Decompression with period " + std::to_string(period) + " steps "+ "and " + std::to_string(harmonics) + " harmonic frequencies");
    }

    // Overlap size
    hsize_t oSize = period * mos;
    // Base size
    hsize_t bSize = oSize * 2 + 1;

    // Get dims
    HDF5Helper::Vector dims = srcDatasetsPhi.at(0)->getDims();
    HDF5Helper::Vector outputDims = dims;

    // TODO - check same dims of srcDatasetPhi and srcDatasetK

    // Compute steps, step size and output dims
    hsize_t steps = 0;
    hsize_t outputSteps = 0;
    hsize_t stepSize = 0;
    if (dims.getLength() == 4) { // 4D dataset
        steps = HDF5Helper::Vector4D(dims).w();
        outputSteps = (steps + 1) * oSize ;
        outputDims[0] = outputSteps;
        stepSize = outputDims[1] * outputDims[2] * outputDims[3];
    } else if (dims.getLength() == 3) { // 3D dataset (defined by sensor mask)
        steps = HDF5Helper::Vector3D(dims).y();
        outputSteps = (steps + 1) * oSize;
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
    generateFunctions(bSize, oSize, period, harmonics, b, e, bE, bE_1, srcDatasetsPhi);

    // Chunk dims
    HDF5Helper::Vector chunkDims(srcDatasetsPhi.at(0)->getChunkDims());

    if (log) {
        Helper::printDebugTwoColumns2S("steps", steps);
        Helper::printDebugTwoColumns2S("outputSteps", outputSteps);
        Helper::printDebugTwoColumns2S("dims", dims);
        Helper::printDebugTwoColumns2S("outputDims", outputDims);
        Helper::printDebugTwoColumns2S("stepSize", stepSize);
        Helper::printDebugTwoColumns2S("chunkDims", chunkDims);
    }

    // Create destination dataset
    std::string srcName = srcDatasetsPhi.at(0)->readAttributeS(HDF5Helper::SRC_DATASET_NAME_ATTR, log);
    getOutputFile()->createDatasetF(srcName + "_d", outputDims, chunkDims, true, log);
    HDF5Helper::Dataset *dstDataset = getOutputFile()->openDataset(srcName + "_d", log);

    double t0 = HDF5Helper::getTime();

    // Variables for block reading
    float **dataPhi = new float*[harmonics];
    float **dataK = new float*[harmonics];
    float *dataPhiL = 0;
    float *dataKL = 0;
    HDF5Helper::Vector offset;
    HDF5Helper::Vector count;
    float maxV = 0, minV = 0;
    hsize_t maxVIndex = 0, minVIndex = 0;
    bool first = true;

    // If we have enough memory - minimal for one full step * (1 (data) + 4 buffers (k, phi, lastK, lastPhi) * number of harmonics) in 3D space
    if (srcDatasetsPhi.at(0)->getFile()->getNumberOfElmsToLoad() >= stepSize * (1 + 4 * harmonics)) {
        // Buffers for last K and Phi
        float *k = new float[harmonics * stepSize]();
        float *phi = new float[harmonics * stepSize]();
        float *lastK = new float[harmonics * stepSize]();
        float *lastPhi = new float[harmonics * stepSize]();
        float *data = new float[stepSize]();

        hsize_t frameDst = 0;

        // Reading and decompression
        for (hsize_t i = 0; i < srcDatasetsPhi.at(0)->getNumberOfBlocks(); i++) {
            for (hsize_t ih = 0; ih < harmonics; ih++) {
                srcDatasetsPhi.at(ih)->readBlock(i, offset, count, dataPhiL, log);
                srcDatasetsK.at(ih)->readBlock(i, offset, count, dataKL, log);
                dataPhi[ih] = dataPhiL;
                dataK[ih] = dataKL;
            }

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
                    for (hssize_t p = 0; p < hssize_t(stepSize); p++) {
                        if (stepLocal == 0) {
                            // Save last coefficients
                            for (hsize_t ih = 0; ih < harmonics; ih++) {
                                hsize_t pH = stepSize * ih + hsize_t(p);
                                lastK[pH] = k[pH];
                                lastPhi[pH] = phi[pH];

                                // Copy first coefficients
                                if (frame == 0) {
                                    lastK[pH] = dataK[ih][p];
                                    lastPhi[pH] = dataPhi[ih][p];
                                }

                                if (frame == framesCount - 1 && lastFlag) {
                                    hsize_t pF1 = (frame - 1) * stepSize + hsize_t(p);
                                    // Duplicate last coefficient
                                    k[pH] = dataK[ih][pF1];
                                    phi[pH] = dataPhi[ih][pF1];
                                } else {
                                    hsize_t pF = frame * stepSize + hsize_t(p);
                                    // Read coefficient
                                    k[pH] = dataK[ih][pF];
                                    phi[pH] = dataPhi[ih][pF];
                                }
                            }
                        }

                        // Compute new point value
                        data[p] = 0;
                        floatC i(0, -1);
                        for (hsize_t ih = 0; ih < harmonics; ih++) {
                            hsize_t pH = stepSize * ih + hsize_t(p);
                            hsize_t sH = ih * bSize + stepLocal;
                            data[p] += real(k[pH] * std::exp(i * phi[pH]) * bE[sH]) + real(lastK[pH] * std::exp(i * lastPhi[pH]) * bE_1[sH]);
                        }

                        // Min/max values
                        HDF5Helper::checkOrSetMinMaxValue(first, minV, maxV, data[p]);
                        #pragma omp critical
                        {
                            if (data[p] == minV)
                                minVIndex = (frameDst * oSize + stepLocal) * stepSize  + hsize_t(p);
                            if (data[p] == maxV)
                                maxVIndex = (frameDst * oSize + stepLocal) * stepSize  + hsize_t(p);
                        }
                    }

                    //double t1 = HDF5Helper::getTime();
                    //Helper::printDebugTime("one step decoding", t0, t1);

                    if (log) {
                        Helper::printDebugMsgStart("Saving frame " + std::to_string(frameDst * oSize + stepLocal));
                    }
                    if (dims.getLength() == 4) { // 4D dataset
                        dstDataset->writeDataset(HDF5Helper::Vector4D(frameDst * oSize + stepLocal, 0, 0, 0), HDF5Helper::Vector4D(1, dims[1], dims[2], dims[3]), data, log);
                    } else if (dims.getLength() == 3) {
                        dstDataset->writeDataset(HDF5Helper::Vector3D(0, frameDst * oSize + stepLocal, 0), HDF5Helper::Vector3D(1, 1, dims[2]), data, log);
                    }
                    if (log) {
                        Helper::printDebugMsg("saved");
                    }
                }
                frameDst++;
                //Helper::printDebugMsg("encoded");
            }

            #pragma omp parallel for
            for (hssize_t h = 0; h < hssize_t(harmonics); h++) {
                delete[] dataK[h];
                delete[] dataPhi[h];
            }
            delete[] dataK;
            delete[] dataPhi;
        }

        // Delete buffers
        delete[] data;
        delete[] k;
        delete[] phi;
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
    copyAttributes(srcDatasetsPhi.at(0), dstDataset);

    dstDataset->removeAttribute(HDF5Helper::C_HARMONIC_ATTR, log);
    dstDataset->setAttribute(HDF5Helper::C_HARMONICS_ATTR, harmonics, log);

    // Set min/max values
    dstDataset->setAttribute(HDF5Helper::MIN_ATTR, minV, log);
    dstDataset->setAttribute(HDF5Helper::MAX_ATTR, maxV, log);
    dstDataset->setAttribute(HDF5Helper::MIN_INDEX_ATTR, minVIndex, log);
    dstDataset->setAttribute(HDF5Helper::MAX_INDEX_ATTR, maxVIndex, log);
    dstDataset->setAttribute(HDF5Helper::C_TYPE_ATTR, "d", log);

    double t1 = HDF5Helper::getTime();
    Helper::printDebugTime("datasets decompression", t0, t1);

    getOutputFile()->closeDataset(dstDataset, log);
}
