/**
 * @file        difference.cpp
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        8  September 2016 (created) <br>
 *              9  October   2018 (updated)
 *
 * @brief       The implementation file containing Difference class definition.
 *
 * @license     This file is part of the k-Wave-h5-processing tool for processing the HDF5 data
 *              created by the k-Wave toolbox - http://www.k-wave.org. This file may be used,
 *              distributed and modified under the terms of the LGPL version 3 open source
 *              license. A copy of the LGPL license should have been received with this file.
 *              Otherwise, it can be found at: http://www.gnu.org/copyleft/lesser.html.
 *
 * @copyright   Copyright © 2018, Petr Kleparnik, VUT FIT Brno. All Rights Reserved.
 *
 */

#include "difference.h"

/**
 * @brief Creates Difference object
 * @param[in] outputFile Output file
 * @param[in] dtsForPcs Datasets for processing
 * @param[in] settings Processing settings
 */
Difference::Difference(H5Helper::File *outputFile, DtsForPcs *dtsForPcs, Settings *settings)
    : Processing(outputFile, dtsForPcs, settings)
{

}

/**
 * @brief Executes processing
 */
void Difference::execute()
{
    std::vector<H5Helper::DatasetType> types = {
        H5Helper::DatasetType::TIME_STEPS_INDEX,
        H5Helper::DatasetType::CUBOID,
        H5Helper::DatasetType::CUBOID_ATTR
    };
    // TODO downsampled datasets?

    try {
        H5Helper::MapOfDatasets map = getDtsForPcs()->getDatasets();
        hsize_t sensorMaskSize = getDtsForPcs()->getSensorMaskSize();
        int count = 0;
        for (H5Helper::MapOfDatasetsIt it = map.begin(); it != map.end(); ++it) {
            H5Helper::Dataset *datasetOriginal = it->second;
            H5Helper::DatasetType datasetType = datasetOriginal->getType(sensorMaskSize);
            if (checkDatasetType(datasetType, types)) {
                H5Helper::Dataset *datasetDecoded = 0;

                for (H5Helper::MapOfDatasetsIt it2 = map.begin(); it2 != map.end(); ++it2) {
                    H5Helper::Dataset *dataset = it2->second;
                    H5Helper::DatasetType datasetDecodedType = dataset->getType(sensorMaskSize);

                    if ((datasetDecodedType == H5Helper::DatasetType::TIME_STEPS_D_INDEX && datasetType == H5Helper::DatasetType::TIME_STEPS_INDEX)
                            || (datasetDecodedType == H5Helper::DatasetType::CUBOID_D && datasetType == H5Helper::DatasetType::CUBOID)
                            || (datasetDecodedType == H5Helper::DatasetType::CUBOID_ATTR_D && datasetType == H5Helper::DatasetType::CUBOID_ATTR)
                            ) {
                        if (datasetOriginal->getName() == dataset->readAttributeS(H5Helper::SRC_DATASET_NAME_ATTR, false)) {
                            datasetDecoded = dataset;
                            break;
                        }
                    }
                }
                if (datasetDecoded) {
                    Helper::printDebugMsg("Subtraction of datasets " + datasetOriginal->getName() + " and " + datasetDecoded->getName());
                    subtractDatasets(datasetOriginal, datasetDecoded, getSettings()->getFlagLog());
                    Helper::printDebugMsg("Subtraction of datasets done");
                    count++;
                }
            }
        }
        if (count == 0) {
            Helper::printErrorMsg("No datasets for making difference in simulation output file");
        }
    } catch(std::exception &e) {
        Helper::printErrorMsg(e.what());
        std::exit(EXIT_FAILURE);
    }
}

/**
 * @brief Subtracts datasets
 * @param[in] datasetOriginal Original dataset
 * @param[in] datasetDecoded Decoded dataset
 */
void Difference::subtractDatasets(H5Helper::Dataset *datasetOriginal, H5Helper::Dataset *datasetDecoded, bool log)
{
    H5Helper::Vector outputDims = datasetOriginal->getDims();
    H5Helper::Vector outputChunkDims = datasetOriginal->getChunkDims();

    std::string srcName = datasetDecoded->readAttributeS(H5Helper::SRC_DATASET_NAME_ATTR, log);
    getOutputFile()->createDatasetF(srcName + "_s", outputDims, outputChunkDims, true, log);
    H5Helper::Dataset *dstDataset = getOutputFile()->openDataset(srcName + "_s", log);

    datasetDecoded->setNumberOfElmsToLoad(datasetOriginal->getNumberOfElmsToLoad());

    // Variables for block reading
    float *dataO = new float[datasetOriginal->getGeneralBlockDims().getSize()];
    float *dataD = new float[datasetOriginal->getGeneralBlockDims().getSize()];
    H5Helper::Vector offset;
    H5Helper::Vector count;
    float maxV = std::numeric_limits<float>::min();
    float minV = std::numeric_limits<float>::max();
    float maxVO = std::numeric_limits<float>::min();
    float minVO = std::numeric_limits<float>::max();
    hsize_t minVIndex = 0, maxVIndex = 0;
    hsize_t minVOIndex = 0, maxVOIndex = 0;
    double sum = 0.0;
    double sumO2 = 0.0;
    double sum2 = 0.0;

    // Reading and subtraction
    for (hsize_t i = 0; i < datasetOriginal->getNumberOfBlocks(); i++) {
        // First read decoded -> it is larger
        datasetDecoded->readBlock(i, offset, count, dataD, log);
        datasetOriginal->readBlock(i, offset, count, dataO, log);
        // The count is from original dataset and is smaller for last step than is in the decoded dataset

        #pragma omp parallel for reduction(+ : sum, sumO2, sum2)
        for (hssize_t i = 0; i < hssize_t(count.getSize()); i++) {
            dataD[i] = dataO[i] - dataD[i];
            sum += double(abs(dataD[i]));
            sumO2 += double(dataO[i] * dataO[i]);
            sum2 += double(dataD[i] * dataD[i]);
            // Min/max values
            hsize_t linearOffset;
            convertMultiDimToLinear(offset, linearOffset, datasetDecoded->getDims());
            H5Helper::checkOrSetMinMaxValue(minV, maxV, dataD[i], minVIndex, maxVIndex, linearOffset + hsize_t(i));
            H5Helper::checkOrSetMinMaxValue(minVO, maxVO, dataO[i], minVOIndex, maxVOIndex, linearOffset + hsize_t(i));
        }

        dstDataset->writeDataset(offset, count, dataD, log);
    }
    delete[] dataD;
    dataD = nullptr;
    delete[] dataO;
    dataO = nullptr;

    // Copy attributes
    copyAttributes(datasetDecoded, dstDataset);

    float maxError = std::max(abs(minV), abs(maxV));
    float meanError = float(sum / double(outputDims.getSize()));
    float maxValue = std::max(abs(minVO), abs(maxVO));
    float mse = float(sum2 / double(outputDims.getSize()));
    float meanO2 = float(sumO2 / double(outputDims.getSize()));

    dstDataset->removeAttribute(H5Helper::C_HARMONIC_ATTR, log);

    // Set min/max values
    dstDataset->setAttribute(H5Helper::MIN_ATTR, minV, log);
    dstDataset->setAttribute(H5Helper::MAX_ATTR, maxV, log);
    dstDataset->setAttribute(H5Helper::MIN_INDEX_ATTR, minVIndex, log);
    dstDataset->setAttribute(H5Helper::MAX_INDEX_ATTR, maxVIndex, log);
    dstDataset->setAttribute(H5Helper::C_TYPE_ATTR, "s", log);
    dstDataset->setAttribute("sum_abs", float(sum), log);
    dstDataset->setAttribute("sum_original_2", float(sumO2), log);
    dstDataset->setAttribute("sum_2", float(sum2), log);
    dstDataset->setAttribute("max_original", maxValue, log);
    dstDataset->setAttribute("mean_error", (meanError / maxValue) * 100, log);
    dstDataset->setAttribute("mean_error_value", meanError, log);
    dstDataset->setAttribute("max_error", (maxError / maxValue) * 100, log);
    dstDataset->setAttribute("max_error_value", maxError, log);
    dstDataset->setAttribute("mean_squared_error", float(mse), log);
    dstDataset->setAttribute("snr", 10 * log10(meanO2 / float(mse)), log);
    dstDataset->setAttribute("psnr", 10 * log10((maxValue * maxValue) / float(mse)), log);

    Helper::printDebugTwoColumns2S("Mean error", std::to_string((meanError / maxValue) * 100) + " %");
    Helper::printDebugTwoColumns2S("Max error", std::to_string((maxError / maxValue) * 100) + " %");
    Helper::printDebugTwoColumns2S("MSE", mse);
    Helper::printDebugTwoColumns2S("SNR", std::to_string(10 * log10(meanO2 / float(mse))) + " dB");
    Helper::printDebugTwoColumns2S("PSNR", std::to_string(10 * log10((maxValue * maxValue) / float(mse))) + " dB");

    getOutputFile()->closeDataset(dstDataset, log);
}
