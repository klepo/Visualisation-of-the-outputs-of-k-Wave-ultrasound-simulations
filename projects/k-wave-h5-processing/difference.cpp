/**
 * @file        difference.cpp
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        8  September 2016 (created) \n
 *              28 August    2017 (updated)
 *
 * @brief       The implementation file containing Difference class definition.
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

#include "difference.h"

/**
 * @brief Creates Difference object
 * @param[in] outputFile Output file
 * @param[in] dtsForPcs Datasets for porcessing
 * @param[in] settings Processing settings
 */
Difference::Difference(HDF5Helper::File *outputFile, DtsForPcs *dtsForPcs, Settings *settings)
    : Processing(outputFile, dtsForPcs, settings)
{

}

/**
 * @brief Executes processing
 */
void Difference::execute()
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
            HDF5Helper::Dataset *datasetOriginal = it->second;
            HDF5Helper::DatasetType datasetType = datasetOriginal->getType(sensorMaskSize);

            if (checkDatasetType(datasetType, types)) {
                HDF5Helper::Dataset *datasetDecoded = 0;

                for (HDF5Helper::MapOfDatasetsIt it2 = map.begin(); it2 != map.end(); ++it2) {
                    HDF5Helper::Dataset *dataset = it2->second;
                    HDF5Helper::DatasetType datasetDecodedType = dataset->getType(sensorMaskSize);
                    if ((datasetDecodedType == HDF5Helper::DatasetType::TIME_STEPS_D_MASK && datasetType == HDF5Helper::DatasetType::TIME_STEPS_MASK)
                            || (datasetDecodedType == HDF5Helper::DatasetType::CUBOID_D && datasetType == HDF5Helper::DatasetType::CUBOID)
                            || (datasetDecodedType == HDF5Helper::DatasetType::CUBOID_ATTR_D && datasetType == HDF5Helper::DatasetType::CUBOID_ATTR)
                            ) {
                        if (datasetOriginal->getName() == dataset->readAttributeS(HDF5Helper::SRC_DATASET_NAME_ATTR, false)) {
                            datasetDecoded = dataset;
                            break;
                        }
                    }
                }

                if (datasetDecoded) {
                    std::cout << "Subtraction of datasets " << datasetOriginal->getName() << " and " << datasetDecoded->getName() << std::endl;
                    subtractDatasets(datasetOriginal, datasetDecoded);
                    std::cout << "Subtraction of datasets done" << std::endl << std::endl;
                    count++;
                }
            }
        }

        if (count == 0) {
            std::cout << "No datasets for making difference in simulation output file" << std::endl;
        }
    } catch(std::exception &e) {
        std::cerr << e.what() << std::endl;
        std::exit(EXIT_FAILURE);
    }
}

/**
 * @brief Subtracts datasets
 * @param[in] datasetOriginal Original dataset
 * @param[in] datasetDecoded Decoded dataset
 */
void Difference::subtractDatasets(HDF5Helper::Dataset *datasetOriginal, HDF5Helper::Dataset *datasetDecoded)
{
    HDF5Helper::Vector outputDims = datasetOriginal->getDims();
    HDF5Helper::Vector outputChunkDims = datasetOriginal->getChunkDims();

    std::string srcName = datasetDecoded->readAttributeS(HDF5Helper::SRC_DATASET_NAME_ATTR, false);
    getOutputFile()->createDatasetF(srcName + "_s", outputDims, outputChunkDims, true);
    HDF5Helper::Dataset *dstDataset = getOutputFile()->openDataset(srcName + "_s");

    // Variables for block reading
    float *dataO = 0;
    float *dataD = 0;
    HDF5Helper::Vector offset;
    HDF5Helper::Vector count;
    float maxV = 0, minV = 0;
    hsize_t minVIndex = 0, maxVIndex = 0;
    bool first = true;
    double sum = 0.0;
    double sumO = 0.0;
    double sum2 = 0.0;

    datasetDecoded->setNumberOfElmsToLoad(datasetOriginal->getNumberOfElmsToLoad());
    // Reading and substraction
    for (hsize_t i = 0; i < datasetOriginal->getNumberOfBlocks(); i++) {
        // First read decoded -> it is larger
        datasetDecoded->readBlock(i, offset, count, dataD);
        datasetOriginal->readBlock(i, offset, count, dataO);
        // The count is from original dataset and is smaller for last step than is in the decoded dataset

        #pragma omp parallel for reduction(+ : sum, sumO, sum2)
        for (hssize_t i = 0; i < hssize_t(count.getSize()); i++) {
            dataD[i] = dataO[i] - dataD[i];
            sum += double(abs(dataD[i]));
            sumO += double(abs(dataO[i]));
            sum2 += double((dataD[i] * dataD[i]));
            // Min/max values
            HDF5Helper::checkOrSetMinMaxValue(first, minV, maxV, dataD[i]);

            hsize_t linearOffset;
            convertMultiDimToLinear(offset, linearOffset, datasetDecoded->getDims());

            #pragma omp critical
            {
                if (dataD[i] == minV)
                    minVIndex = linearOffset + i;
                if (dataD[i] == maxV)
                    maxVIndex = linearOffset + i;
            }
        }

        dstDataset->writeDataset(offset, count, dataD, true);
        delete[] dataD;
        delete[] dataO;
    }

    // Copy attributes
    copyAttributes(datasetDecoded, dstDataset);

    float minVO;
    float maxVO;
    hsize_t minVOIndex;
    hsize_t maxVOIndex;
    datasetOriginal->getGlobalMinValue(minVO, minVOIndex);
    datasetOriginal->getGlobalMaxValue(maxVO, maxVOIndex);
    float maxError = std::max(abs(minV), abs(maxV));
    double meanError = sum / double(outputDims.getSize());
    float maxValue = std::max(abs(minVO), abs(maxVO));
    double mse = sum2 / double(outputDims.getSize());

    dstDataset->removeAttribute(HDF5Helper::C_HARMONIC_ATTR);

    // Set min/max values
    dstDataset->setAttribute(HDF5Helper::MIN_ATTR, minV);
    dstDataset->setAttribute(HDF5Helper::MAX_ATTR, maxV);
    dstDataset->setAttribute(HDF5Helper::MIN_INDEX_ATTR, minVIndex);
    dstDataset->setAttribute(HDF5Helper::MAX_INDEX_ATTR, maxVIndex);
    dstDataset->setAttribute(HDF5Helper::C_TYPE_ATTR, "s");
    dstDataset->setAttribute("sum", float(sum));
    dstDataset->setAttribute("sum_original", float(sumO));
    dstDataset->setAttribute("sum_2", float(sum2));
    dstDataset->setAttribute("max_original", maxValue);
    dstDataset->setAttribute("mean_error", (meanError / maxValue) * 100);
    dstDataset->setAttribute("mean_error_value", meanError);
    dstDataset->setAttribute("max_error", (maxError / maxValue) * 100);
    dstDataset->setAttribute("max_error_value", maxError);
    dstDataset->setAttribute("mean_squared_error", float(mse));
    dstDataset->setAttribute("snr", float(sumO / mse));
    dstDataset->setAttribute("psnr", 10 * log10((maxValue * maxValue) / float(mse)));

    std::cout << "sum:          " << sum << std::endl;
    std::cout << "sum_original: " << sumO << std::endl;
    std::cout << "sum_2:        " << sum2 << std::endl;


    std::cout << std::endl;
    std::cout << "Mean error: " << (meanError / maxValue) * 100 << " %" << std::endl;
    std::cout << "Max error:  " << (maxError / maxValue) * 100  << " %" << std::endl;
    std::cout << "MSE:        " << mse << std::endl;
    std::cout << "SNR:        " << sumO / mse << std::endl;
    std::cout << "PSNR:       " << 10 * log10((maxValue * maxValue) / float(mse)) << " dB" << std::endl;
    std::cout << std::endl;

    getOutputFile()->closeDataset(dstDataset);
}
