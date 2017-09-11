/**
 * @file        compress.cpp
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        8  September 2016 (created) \n
 *              28 August    2017 (updated)
 *
 * @brief       The implementation file containing Compress class definition.
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

#include "compress.h"

Compress::Compress(HDF5Helper::File *outputFile, DtsForPcs *dtsForPcs, Settings *settings)
    : Processing(outputFile, dtsForPcs, settings)
{

}

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
                std::cout << "Compression of dataset " << dataset->getName() << std::endl;
                compressDataset(dataset);
                count++;
                std::cout << "Compression of dataset " << dataset->getName() << " done" << std::endl << std::endl;
            }
        }
        if (count == 0) {
            std::cout << "No datasets for compression in simulation output file" << std::endl;
        }
    } catch(std::exception &e) {
        std::cerr << e.what() << std::endl;
        std::exit(EXIT_FAILURE);
    }
}

void Compress::compressDataset(HDF5Helper::Dataset *srcDataset)
{
    if (!getSettings()->getPeriod()) {
        std::cout << "No known period for compression" << std::endl;
        return;
    }
    std::cout << "Compression with period " << getSettings()->getPeriod() << " steps of " << getSettings()->getHarmonic() << ". harmonic frequency" << std::endl;

    // Only one coding parameter - multiple of overlap size
    // Overlap size
    hsize_t oSize = getSettings()->getPeriod() * getSettings()->getMOS();
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
        std::cout << "Something wrong with dataset dims" << std::endl;
        return;
    }

    // Memory for helper functions data
    float *b = new float[bSize]();
    Helper::floatC *e = new Helper::floatC[bSize]();
    Helper::floatC *bE = new Helper::floatC[bSize]();
    Helper::floatC *bE_1 = new Helper::floatC[bSize]();

    // Generate basis function (window)
    Helper::triangular(oSize, b);  // Triangular window
    //HDF5Helper::hann(oSize, b);        // Hann window

    // Generate complex exponential functions
    Helper::floatC i(0, -1);
    #pragma omp parallel for
    for (hssize_t x = 0; x < hssize_t(bSize); x++) {
        e[x] = std::exp(i * (2.0f * float(M_PI) / (float(getSettings()->getPeriod()) / getSettings()->getHarmonic())) * float(x));
    }
    #pragma omp parallel for
    for (hssize_t x = 0; x < hssize_t(bSize); x++) {
        bE[x] = b[x] * e[x] * (2.0f / float(oSize));
        bE_1[x] = b[(hsize_t(x) + oSize) % (bSize - 1)] * e[(hsize_t(x) + oSize) % (bSize - 1)] * (2.0f / float(oSize));
    }

    // Compute new chunk size
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

    std::cout << "steps:       " << steps << std::endl;
    std::cout << "outputSteps: " << outputSteps << std::endl;
    std::cout << "dims:        " << dims << std::endl;
    std::cout << "outputDims:  " << outputDims << std::endl;
    std::cout << "stepSize:    " << stepSize << std::endl;

    // Create dst datasets
    getOutputFile()->createDatasetF(srcDataset->getName() + "_k_" + std::to_string(getSettings()->getHarmonic()), outputDims, chunkDims, true);
    getOutputFile()->createDatasetF(srcDataset->getName() + "_fi_" + std::to_string(getSettings()->getHarmonic()), outputDims, chunkDims, true);
    HDF5Helper::Dataset *dstDatasetK = getOutputFile()->openDataset(srcDataset->getName() + "_k_" + std::to_string(getSettings()->getHarmonic()));
    HDF5Helper::Dataset *dstDatasetFi = getOutputFile()->openDataset(srcDataset->getName() + "_fi_" + std::to_string(getSettings()->getHarmonic()));

    double t0 = HDF5Helper::getTime();

    // Variables for block reading
    float *data = 0;
    HDF5Helper::Vector offset;
    HDF5Helper::Vector count;
    float maxVK = 0, maxVFi = 0;
    float minVK = 0, minVFi = 0;
    hsize_t maxVKIndex = 0, maxVFiIndex = 0;
    hsize_t minVKIndex = 0, minVFiIndex = 0;
    bool first = true;

    // If we have enough memory - minimal for one full step in 3D space
    if (srcDataset->getNumberOfElmsToLoad() >= stepSize) {

        // Complex buffers for accumulation
        Helper::floatC *sCTmp1 = new Helper::floatC[stepSize]();
        Helper::floatC *sCTmp2 = new Helper::floatC[stepSize]();

        //hsize_t step = 0;
        hsize_t frame = 1;

        float *dataK = new float[stepSize];
        float *dataFi = new float[stepSize];

        // Reading and compression
        for (hsize_t i = 0; i < srcDataset->getNumberOfBlocks(); i++) {
            srcDataset->readBlock(i, offset, count, data);

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
                //std::cout << "Encoding step " << stepsOffset + step << std::endl;

                // Compute local index
                hsize_t stepLocal = (stepsOffset + step) % (bSize - 1);

                // For every point
                #pragma omp parallel for
                for (hssize_t p = 0; p < hssize_t(stepSize); p++) {
                    // Correlation step
                    sCTmp1[p] += bE[stepLocal] * data[step * stepSize + hsize_t(p)];
                    sCTmp2[p] += bE_1[stepLocal] * data[step * stepSize + hsize_t(p)];

                    //std::cout << "step: " << step << std::endl;
                    //std::cout << "step * stepSize: " << step << std::endl;

                    // Check if we are at saving point
                    //if (stepLocal + 1 == oSize || stepLocal + 1 == oSize * 2) {
                    if ((stepLocal + 1) % oSize == 0) {

                        Helper::floatC sC;

                        // Select accumulated value
                        if ((frame % 2) == 0) {
                            sC = sCTmp1[p];
                            sCTmp1[p] = 0;
                        } else {
                            sC = sCTmp2[p];
                            sCTmp2[p] = 0;
                        }

                        // Normalization
                        //sC = sC * 2.0f / float(oSize);

                        // Computing amplitude
                        float k = abs(sC);

                        // Computing phase
                        float fi = arg(sC);

                        // Save complex coefficients
                        // Drop first "half" frame
                        if (frame > 1) {
                            dataK[p] = k;
                            dataFi[p] = fi;

                            // Min/max values
                            HDF5Helper::checkOrSetMinMaxValue(first, minVK, maxVK, k);
                            HDF5Helper::checkOrSetMinMaxValue(first, minVFi, maxVFi, fi);

                            #pragma omp critical
                            {
                                if (k == minVK)
                                    minVKIndex = (frame - 2) * stepSize + p;
                                if (k == maxVK)
                                    maxVKIndex = (frame - 2) * stepSize + p;
                                if (k == minVFi)
                                    minVFiIndex = (frame - 2) * stepSize + p;
                                if (k == maxVFi)
                                    maxVFiIndex = (frame - 2) * stepSize + p;
                            }
                        }

                    }
                }

                // Increment frame
                //if (stepLocal == oSize - 1 || stepLocal == oSize * 2 - 1) {
                if ((stepLocal + 1) % oSize == 0) {
                    if (frame > 1) {
                        std::cout << "Saving frame " << frame - 1 << " ... ";
                        if (dims.getLength() == 4) { // 4D dataset
                            dstDatasetK->writeDataset(HDF5Helper::Vector4D(frame - 2, 0, 0, 0), HDF5Helper::Vector4D(1, dims[1], dims[2], dims[3]), dataK);
                            dstDatasetFi->writeDataset(HDF5Helper::Vector4D(frame - 2, 0, 0, 0), HDF5Helper::Vector4D(1, dims[1], dims[2], dims[3]), dataFi);
                        } else if (dims.getLength() == 3) {
                            dstDatasetK->writeDataset(HDF5Helper::Vector3D(0, frame - 2, 0), HDF5Helper::Vector3D(1, 1, dims[2]), dataK);
                            dstDatasetFi->writeDataset(HDF5Helper::Vector3D(0, frame - 2, 0), HDF5Helper::Vector3D(1, 1, dims[2]), dataFi);
                        }
                        std::cout << "saved" << std::endl;
                    }
                    frame++;
                }
                //std::cout << "encoded" << std::endl;
            }

            // Last frame (copy last)
            if (frame - 1 == outputSteps) {
                std::cout << "Saving frame " << frame - 1 << " ... ";
                if (dims.getLength() == 4) { // 4D dataset
                    dstDatasetK->writeDataset(HDF5Helper::Vector4D(frame - 2, 0, 0, 0), HDF5Helper::Vector4D(1, dims[1], dims[2], dims[3]), dataK);
                    dstDatasetFi->writeDataset(HDF5Helper::Vector4D(frame - 2, 0, 0, 0), HDF5Helper::Vector4D(1, dims[1], dims[2], dims[3]), dataFi);
                } else if (dims.getLength() == 3) {
                    dstDatasetK->writeDataset(HDF5Helper::Vector3D(0, frame - 2, 0), HDF5Helper::Vector3D(1, 1, dims[2]), dataK);
                    dstDatasetFi->writeDataset(HDF5Helper::Vector3D(0, frame - 2, 0), HDF5Helper::Vector3D(1, 1, dims[2]), dataFi);
                }
                std::cout << "saved" << std::endl;
            }

            delete[] data;
        }

        delete[] dataK;
        delete[] dataFi;

        // Delete complex buffers
        delete[] sCTmp1;
        delete[] sCTmp2;
    } else {
        // Not implemented yet
        std::cout << "Not implemented for such big datasets yet" << std::endl;
        delete[] b;
        delete[] e;
        delete[] bE;
        delete[] bE_1;
        getOutputFile()->closeDataset(dstDatasetK);
        getOutputFile()->closeDataset(dstDatasetFi);
        return;
    }

    // Delete some memory
    delete[] b;
    delete[] e;
    delete[] bE;
    delete[] bE_1;

    // Copy attributes
    copyAttributes(srcDataset, dstDatasetK);
    copyAttributes(srcDataset, dstDatasetFi);

    // Set min/max values
    dstDatasetK->setAttribute(HDF5Helper::MIN_ATTR, minVK);
    dstDatasetK->setAttribute(HDF5Helper::MAX_ATTR, maxVK);
    dstDatasetK->setAttribute(HDF5Helper::MIN_INDEX_ATTR, minVKIndex);
    dstDatasetK->setAttribute(HDF5Helper::MAX_INDEX_ATTR, maxVKIndex);
    dstDatasetK->setAttribute(HDF5Helper::SRC_DATASET_NAME_ATTR, srcDataset->getName());
    dstDatasetK->setAttribute(HDF5Helper::C_TYPE_ATTR, "k");
    dstDatasetK->setAttribute(HDF5Helper::C_PERIOD_ATTR, getSettings()->getPeriod());
    dstDatasetK->setAttribute(HDF5Helper::C_HARMONIC_ATTR, getSettings()->getHarmonic());
    dstDatasetK->setAttribute(HDF5Helper::C_MOS_ATTR, getSettings()->getMOS());
    dstDatasetFi->setAttribute(HDF5Helper::MIN_ATTR, minVFi);
    dstDatasetFi->setAttribute(HDF5Helper::MAX_ATTR, maxVFi);
    dstDatasetFi->setAttribute(HDF5Helper::MIN_INDEX_ATTR, minVFiIndex);
    dstDatasetFi->setAttribute(HDF5Helper::MAX_INDEX_ATTR, maxVFiIndex);
    dstDatasetFi->setAttribute(HDF5Helper::SRC_DATASET_NAME_ATTR, srcDataset->getName());
    dstDatasetFi->setAttribute(HDF5Helper::C_TYPE_ATTR, "fi");
    dstDatasetFi->setAttribute(HDF5Helper::C_PERIOD_ATTR, getSettings()->getPeriod());
    dstDatasetFi->setAttribute(HDF5Helper::C_HARMONIC_ATTR, getSettings()->getHarmonic());
    dstDatasetFi->setAttribute(HDF5Helper::C_MOS_ATTR, getSettings()->getMOS());

    double t1 = HDF5Helper::getTime();
    std::cout << "Time of the whole dataset compression: " << (t1 - t0) << " ms; \t" << std::endl;

    getOutputFile()->closeDataset(dstDatasetK);
    getOutputFile()->closeDataset(dstDatasetFi);
}
