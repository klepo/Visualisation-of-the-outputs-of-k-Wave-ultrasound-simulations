/**
 * @file        decompress.cpp
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        8  September 2016 (created) \n
 *              28 August    2017 (updated)
 *
 * @brief       The implementation file containing Decompress class definition.
 *
 * @license     This file is part of the k-Wave-h5-processing tool for processing the HDF5 data
 *              created by the k-Wave toolbox - http://www.k-wave.org. This file may be used,
 *              distributed and modified under the terms of the LGPL version 3 open source
 *              license. A copy of the LGPL license should have been recieved with this file.
 *              Otherwise, it can be found at: http://www.gnu.org/copyleft/lesser.html.
 *
 * @copyright   Copyright © 2017, Petr Kleparnik, VUT FIT Brno. All Rights Reserved.
 *
 */

#include "decompress.h"

Decompress::Decompress(HDF5Helper::File *outputFile, DtsForPcs *dtsForPcs, Settings *settings)
    : Processing(outputFile, dtsForPcs, settings)
{

}

void Decompress::execute()
{
    try {
        HDF5Helper::MapOfDatasets map = getDtsForPcs()->getDatasets();
        hsize_t sensorMaskSize = getDtsForPcs()->getSensorMaskSize();
        int count = 0;
        std::vector<HDF5Helper::Dataset *> datasetsFi;
        std::vector<HDF5Helper::Dataset *> datasetsK;

        for (HDF5Helper::MapOfDatasetsIt it = map.begin(); it != map.end(); ++it) {
            HDF5Helper::Dataset *datasetFi = it->second;
            HDF5Helper::DatasetType datasetType = datasetFi->getType(sensorMaskSize);

            // TODO downsampled datasets
            if (datasetType == HDF5Helper::DatasetType::TIME_STEPS_FI_MASK
                    || datasetType == HDF5Helper::DatasetType::CUBOID_FI
                    || datasetType == HDF5Helper::DatasetType::CUBOID_ATTR_FI
                    //|| datasetType == HDF5Helper::datasetType::CUBOID_DWNSMPL_FI
                    //|| datasetType == HDF5Helper::datasetType::CUBOID_ATTR_DWNSMPL_FI
                    ) {

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
                    if ((datasetKType == HDF5Helper::DatasetType::TIME_STEPS_K_MASK && datasetType == HDF5Helper::DatasetType::TIME_STEPS_FI_MASK)
                            || (datasetKType == HDF5Helper::DatasetType::CUBOID_K && datasetType == HDF5Helper::DatasetType::CUBOID_FI)
                            || (datasetKType == HDF5Helper::DatasetType::CUBOID_ATTR_K && datasetType == HDF5Helper::DatasetType::CUBOID_ATTR_FI)
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
            std::cout << "No datasets for decompression in simulation output file" << std::endl;
        } else {
            std::cout << "Decompression of datasets ..." << std::endl;
            decompressDatasets(datasetsFi, datasetsK);
            std::cout << "Decompression of datasets done" << std::endl << std::endl;
        }

    } catch(std::exception &e) {
        std::cerr << e.what() << std::endl;
        std::exit(EXIT_FAILURE);
    }
}

void Decompress::decompressDatasets(std::vector<HDF5Helper::Dataset *> srcDatasetsFi, std::vector<HDF5Helper::Dataset *> srcDatasetsK)
{
    // First decoding parameter - multiple of overlap size
    hsize_t s = 1;
    if (srcDatasetsFi.at(0)->hasAttribute(HDF5Helper::C_MOS_ATTR))
        s = srcDatasetsFi.at(0)->readAttributeI(HDF5Helper::C_MOS_ATTR);
    // Second decoding parametr - period
    hsize_t period = srcDatasetsFi.at(0)->readAttributeI(HDF5Helper::C_PERIOD_ATTR);
    // Harmonic frequency
    hsize_t harmonics = srcDatasetsFi.size();
    //if (srcDatasetFi->hasAttribute(HDF5Helper::C_HARMONIC_ATTR)) {
    //    harmonic = srcDatasetFi->readAttributeI(HDF5Helper::C_HARMONIC_ATTR);
    //}

    // Overlap size
    hsize_t oSize = period * s;
    // Base size
    hsize_t bSize = oSize * 2 + 1;

    // Get dims
    HDF5Helper::Vector dims = srcDatasetsFi.at(0)->getDims();
    HDF5Helper::Vector outputDims = dims;

    // TODO - check same dims of srcDatasetFi and srcDatasetK

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
        std::cout << "Something wrong with dataset dims" << std::endl;
        return;
    }

    // Memory for helper functions data, 2d arrays for harmonics
    float *b = new float[bSize]();
    Helper::floatC *e = new Helper::floatC[bSize * harmonics]();
    Helper::floatC *bE = new Helper::floatC[bSize* harmonics]();
    Helper::floatC *bE_1 = new Helper::floatC[bSize* harmonics]();

    // Generate basis function (window)
    Helper::triangular(oSize, b);  // Triangular window
    //HDF5Helper::hann(oSize, b);        // Hann window

    // Generate complex exponential functions
    Helper::floatC i(0, -1);
    //#pragma omp parallel for
    for (hssize_t h = 0; h < hssize_t(harmonics); h++) {
        hsize_t ih = 1;
        if (srcDatasetsFi.at(hsize_t(h))->hasAttribute(HDF5Helper::C_HARMONIC_ATTR)) {
            ih = srcDatasetsFi.at(hsize_t(h))->readAttributeI(HDF5Helper::C_HARMONIC_ATTR);
        }
        #pragma omp parallel for
        for (hssize_t x = 0; x < hssize_t(bSize); x++) {
            hssize_t hx = h * hssize_t(bSize) + x;
            e[hx] = std::exp(i * (2.0f * float(M_PI) / (float(period) / (ih))) * float(x));
        }
    }
    #pragma omp parallel for
    for (hssize_t h = 0; h < hssize_t(harmonics); h++) {
        #pragma omp parallel for
        for (hssize_t x = 0; x < hssize_t(bSize); x++) {
            hssize_t hx = h * hssize_t(bSize) + x;
            bE[hx] = b[x] * e[hx];
            bE_1[hx] = b[(hsize_t(x) + oSize) % (bSize - 1)] * e[hsize_t(h) * bSize + ((hsize_t(x) + oSize) % (bSize - 1))];
        }
    }

    // Compute new chunk dims (it is 1 for time dimension)
    HDF5Helper::Vector chunkDims(outputDims.getLength(), 1);
    #pragma omp parallel for
    for (hssize_t i = 1; i < hssize_t(outputDims.getLength()); i++) {
        chunkDims[i] = getSettings()->getMaxChunkSize();
        if (chunkDims[i] > outputDims[i]) chunkDims[i] = outputDims[i];
    }
    if (dims.getLength() == 3) { // Original chunks layout
        chunkDims = outputDims;
        chunkDims[1] = 1;
    }

    std::cout << "steps:        " << steps << std::endl;
    std::cout << "output steps: " << outputSteps << std::endl;
    std::cout << "dims:         " << dims << std::endl;
    std::cout << "output dims:  " << outputDims << std::endl;
    std::cout << "step size:    " << stepSize << std::endl;

    // Create dst dataset
    std::string srcName = srcDatasetsFi.at(0)->readAttributeS(HDF5Helper::SRC_DATASET_NAME_ATTR, false);
    getOutputFile()->createDatasetF(srcName + "_d", outputDims, chunkDims, true);
    HDF5Helper::Dataset *dstDataset = getOutputFile()->openDataset(srcName + "_d");

    double t0 = HDF5Helper::getTime();

    // Variables for block reading
    float **dataFi = new float*[harmonics];
    float **dataK = new float*[harmonics];
    float *dataFiL = 0;
    float *dataKL = 0;
    HDF5Helper::Vector offset;
    HDF5Helper::Vector count;
    float maxV = 0, minV = 0;
    hsize_t maxVIndex = 0, minVIndex = 0;
    bool first = true;

    // If we have enough memory - minimal for one full step * (1 (data) + 4 buffers (k, fi, lastK, lastFi) * number of harmonics) in 3D space
    if (srcDatasetsFi.at(0)->getFile()->getNumberOfElmsToLoad() >= stepSize * (1 + 4 * harmonics)) {
        // Buffers for last K and Fi
        float *k = new float[stepSize * harmonics]();
        float *fi = new float[stepSize * harmonics]();
        float *lastK = new float[stepSize * harmonics]();
        float *lastFi = new float[stepSize * harmonics]();
        float *data = new float[stepSize]();

        hsize_t frameDst = 0;

        // Reading and decompression
        for (hsize_t i = 0; i < srcDatasetsFi.at(0)->getNumberOfBlocks(); i++) {
            for (hsize_t h = 0; h < harmonics; h++) {
                srcDatasetsFi.at(h)->readBlock(i, offset, count, dataFiL);
                srcDatasetsK.at(h)->readBlock(i, offset, count, dataKL);
                dataFi[h] = dataFiL;
                dataK[h] = dataKL;
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
                std::cout << "Decoding frame " << frameDst << std::endl;
                // Decode steps
                for (hsize_t p = 0; p < oSize; p++) {
                    // For every coefficient in step

                    double t0 = HDF5Helper::getTime();

                    #pragma omp parallel for
                    for (hssize_t cKFi = 0; cKFi < hssize_t(stepSize); cKFi++) {
                        if (p == 0) {
                            // Save last coefficients
                            for (hsize_t h = 0; h < harmonics; h++) {
                                hsize_t hcKFi = stepSize * h + hsize_t(cKFi);
                                lastK[hcKFi] = k[hcKFi];
                                lastFi[hcKFi] = fi[hcKFi];

                                // Copy first coefficients
                                if (frame == 0) {
                                    lastK[hcKFi] = dataK[h][cKFi];
                                    lastFi[hcKFi] = dataFi[h][cKFi];
                                }

                                if (frame == framesCount - 1 && lastFlag) {
                                    // Duplicate last coefficient
                                    k[hcKFi] = dataK[h][(frame - 1) * stepSize + hsize_t(cKFi)];
                                    fi[hcKFi] = dataFi[h][(frame - 1) * stepSize + hsize_t(cKFi)];
                                } else {
                                    // Read coefficient
                                    k[hcKFi] = dataK[h][frame * stepSize + hsize_t(cKFi)];
                                    fi[hcKFi] = dataFi[h][frame * stepSize + hsize_t(cKFi)];
                                }
                            }
                        }

                        // Compute new point value
                        data[cKFi] = 0;
                        Helper::floatC i(0, -1);
                        for (hsize_t h = 0; h < harmonics; h++) {
                            hsize_t hcKFi = stepSize * h + hsize_t(cKFi);
                            data[cKFi] += real(k[hcKFi] * std::exp(i * fi[hcKFi]) * bE[h * bSize + p]) + real(lastK[hcKFi] * std::exp(i * lastFi[hcKFi]) * bE_1[h * bSize + p]);
                        }
                        // Min/max values
                        HDF5Helper::checkOrSetMinMaxValue(first, minV, maxV, data[cKFi]);
                        #pragma omp critical
                        {
                            if (data[cKFi] == minV)
                                minVIndex = (frameDst * oSize + p) * stepSize  + cKFi;
                            if (data[cKFi] == maxV)
                                maxVIndex = (frameDst * oSize + p) * stepSize  + cKFi;
                        }
                    }

                    double t1 = HDF5Helper::getTime();
                    std::cout << "Time of the one step decoding: " << (t1 - t0) << " ms; \t" << std::endl;

                    std::cout << "Saving step " << frameDst * oSize + p << " ... ";
                    if (dims.getLength() == 4) { // 4D dataset
                        dstDataset->writeDataset(HDF5Helper::Vector4D(frameDst * oSize + p, 0, 0, 0), HDF5Helper::Vector4D(1, dims[1], dims[2], dims[3]), data, true);
                    } else if (dims.getLength() == 3) {
                        dstDataset->writeDataset(HDF5Helper::Vector3D(0, frameDst * oSize + p, 0), HDF5Helper::Vector3D(1, 1, dims[2]), data, true);
                    }
                    std::cout << "saved" << std::endl;
                }
                frameDst++;
                std::cout << "encoded" << std::endl;
            }

            #pragma omp parallel for
            for (hssize_t h = 0; h < hssize_t(harmonics); h++) {
                delete[] dataK[h];
                delete[] dataFi[h];
            }
            delete[] dataK;
            delete[] dataFi;
        }

        // Delete buffers
        delete[] data;
        delete[] k;
        delete[] fi;
    } else {
        // Not implemented yet
        std::cout << "Not implemented yet" << std::endl;
        delete[] b;
        delete[] e;
        delete[] bE;
        delete[] bE_1;
        getOutputFile()->closeDataset(dstDataset);
        return;
    }

    // Delete some memory
    delete[] b;
    delete[] e;
    delete[] bE;
    delete[] bE_1;

    // Copy attributes
    copyAttributes(srcDatasetsFi.at(0), dstDataset);

    dstDataset->removeAttribute(HDF5Helper::C_HARMONIC_ATTR, false);
    dstDataset->setAttribute(HDF5Helper::C_HARMONICS_ATTR, harmonics);

    // Set min/max values
    dstDataset->setAttribute(HDF5Helper::MIN_ATTR, minV);
    dstDataset->setAttribute(HDF5Helper::MAX_ATTR, maxV);
    dstDataset->setAttribute(HDF5Helper::MIN_INDEX_ATTR, minVIndex);
    dstDataset->setAttribute(HDF5Helper::MAX_INDEX_ATTR, maxVIndex);
    dstDataset->setAttribute(HDF5Helper::C_TYPE_ATTR, "d");

    double t1 = HDF5Helper::getTime();
    std::cout << "Time of the datasets decompression: " << (t1 - t0) << " ms; \t" << std::endl;

    getOutputFile()->closeDataset(dstDataset);
}
