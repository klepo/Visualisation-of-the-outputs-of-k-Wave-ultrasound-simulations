/**
 * @file        difference.cpp
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        8  September 2016 (created) <br>
 *              27 March     2019 (updated)
 *
 * @brief       The implementation file containing Difference class definition.
 *
 * @license     This file is part of the k-Wave-h5-processing tool for processing the HDF5 data
 *              created by the k-Wave toolbox - http://www.k-wave.org. This file may be used,
 *              distributed and modified under the terms of the LGPL version 3 open source
 *              license. A copy of the LGPL license should have been received with this file.
 *              Otherwise, it can be found at: http://www.gnu.org/copyleft/lesser.html.
 *
 * @copyright   Copyright © 2019, Petr Kleparnik, VUT FIT Brno. All Rights Reserved.
 *
 */

#include <bitset>

#include "difference.h"

/**
 * @brief Creates Difference object
 * @param[in] outputFile Output file
 * @param[in] dtsForPcs Datasets for processing
 * @param[in] settings Processing settings
 */
Difference::Difference(H5Helper::File *outputFile, DtsForPcs *dtsForPcs, const Settings *settings)
    : Processing(outputFile, dtsForPcs, settings)
{

}

/**
 * @brief Executes processing
 */
void Difference::execute()
{
    std::vector<H5Helper::DatasetType> maskTypes = {
        H5Helper::DatasetType::TIME_STEPS_INDEX,
        H5Helper::DatasetType::TIME_STEPS_D_INDEX,
        };
    std::vector<H5Helper::DatasetType> compressedMaskTypes = {
        H5Helper::DatasetType::TIME_STEPS_C_INDEX,
        };
    std::vector<H5Helper::DatasetType> cuboidTypes = {
        H5Helper::DatasetType::CUBOID,
        H5Helper::DatasetType::CUBOID_D,
        H5Helper::DatasetType::CUBOID_ATTR,
        H5Helper::DatasetType::CUBOID_ATTR_D,
        };
    std::vector<H5Helper::DatasetType> compressedCuboidTypes = {
        H5Helper::DatasetType::CUBOID_C,
        H5Helper::DatasetType::CUBOID_ATTR_C,
        };
    // TODO downsampled datasets?

    try {
        H5Helper::MapOfDatasets map = getDtsForPcs()->getDatasets();
        H5Helper::MapOfDatasets map2 = getDtsForPcs()->getDatasets2();
        hsize_t sensorMaskSize = getDtsForPcs()->getSensorMaskSize();
        int count = 0;
        // Compare 2 selected datasets
        if (getSettings()->getNames().size() == 2) {
            H5Helper::Dataset *dataset1 = nullptr;
            H5Helper::Dataset *dataset2 = nullptr;
            try {
                if (map.size() == 2 && map2.size() == 0) {
                    dataset1 = map.at(getSettings()->getNames().back());
                    dataset2 = map.at(getSettings()->getNames().front());
                } else {
                    dataset1 = map.at(getSettings()->getNames().back());
                    dataset2 = map2.at(getSettings()->getNames().front());
                }
            } catch(std::exception) {
                Helper::printErrorMsg("No datasets for making difference in files");
                return;
            }

            H5Helper::DatasetType dataset1Type = dataset1->getType(sensorMaskSize);
            H5Helper::DatasetType dataset2Type = dataset2->getType(sensorMaskSize);

            if (dataset1->getRank() == dataset2->getRank()) {
                if (// Time (y) may differ - sensor mask type
                    (checkDatasetType(dataset1Type, maskTypes)
                     && checkDatasetType(dataset2Type, maskTypes)
                     && H5Helper::Vector3D(dataset1->getDims()).x() == H5Helper::Vector3D(dataset2->getDims()).x())
                    // or time may differ - cuboid type
                    || (checkDatasetType(dataset1Type, cuboidTypes)
                        && checkDatasetType(dataset2Type, cuboidTypes)
                        && H5Helper::Vector3D(dataset1->getDims()) == H5Helper::Vector3D(dataset2->getDims()))
                    ) {
                    if (H5Helper::Vector3D(dataset1->getDims()).y() > H5Helper::Vector3D(dataset2->getDims()).y()
                        || (dataset1->getRank() == 4 && H5Helper::Vector4D(dataset1->getDims()).t() > H5Helper::Vector4D(dataset2->getDims()).t())) {
                        Helper::printDebugMsg("Subtraction of datasets " + dataset2->getName() + " and " + dataset1->getName());
                        subtractDatasets(dataset2, dataset1, getSettings()->getFlagLog());
                    } else {
                        Helper::printDebugMsg("Subtraction of datasets " + dataset1->getName() + " and " + dataset2->getName());
                        subtractDatasets(dataset1, dataset2, getSettings()->getFlagLog());
                        Helper::printDebugMsg("Subtraction of datasets done");
                        count++;
                    }
                } else if (
                    // 40-bit compression difference (40-bit is the second dataset)
                    (((checkDatasetType(dataset1Type, compressedCuboidTypes) && checkDatasetType(dataset2Type, compressedCuboidTypes))
                      || (checkDatasetType(dataset1Type, compressedMaskTypes) && checkDatasetType(dataset2Type, compressedMaskTypes)))
                     && dataset2->hasAttribute("c_complex_size")
                     && dataset2->readAttributeF("c_complex_size", true) == H5Helper::CompressHelper::complexSize40bit)
                    // Or same dims
                    || dataset1->getDims() == dataset2->getDims()
                    ) {
                    Helper::printDebugMsg("Subtraction of datasets " + dataset1->getName() + " and " + dataset2->getName());
                    subtractDatasets(dataset1, dataset2, getSettings()->getFlagLog());
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
 * @param[in] log Logging flag (optional)
 */
void Difference::subtractDatasets(H5Helper::Dataset *datasetOriginal, H5Helper::Dataset *datasetDecoded, bool log)
{
    double t0 = H5Helper::getTime();

    H5Helper::Vector outputDims = datasetOriginal->getDims();
    H5Helper::Vector outputChunkDims = datasetOriginal->getChunkDims();

    std::string dstName = datasetDecoded->getSuffixName("_s");
    getOutputFile()->createDatasetF(dstName, outputDims, outputChunkDims, true, log);
    H5Helper::Dataset *dstDataset = getOutputFile()->openDataset(dstName, log);

    datasetDecoded->setNumberOfElmsToLoad(datasetOriginal->getNumberOfElmsToLoad());

    // Variables for block reading
    float *dataO = nullptr;
    float *dataD = nullptr;
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

    if (datasetDecoded->hasAttribute("c_complex_size")
        && datasetDecoded->readAttributeF("c_complex_size", false) == H5Helper::CompressHelper::complexSize40bit
        && (datasetDecoded->getType() == H5Helper::DatasetType::CUBOID_C
            || datasetDecoded->getType() == H5Helper::DatasetType::CUBOID_ATTR_C
            || datasetDecoded->getType() == H5Helper::DatasetType::TIME_STEPS_C_INDEX)
        && (datasetOriginal->getType() == H5Helper::DatasetType::CUBOID_C
            || datasetOriginal->getType() == H5Helper::DatasetType::CUBOID_ATTR_C
            || datasetOriginal->getType() == H5Helper::DatasetType::TIME_STEPS_C_INDEX)
        ) {
        hsize_t stepSizeO = 0;
        hsize_t stepSizeD = 0;
        H5Helper::Vector dimsO = datasetOriginal->getDims();
        H5Helper::Vector dimsD = datasetDecoded->getDims();
        if (dimsO.getLength() == 4) { // 4D dataset
            stepSizeO = dimsO[1] * dimsO[2] * dimsO[3];
            stepSizeD = dimsD[1] * dimsD[2] * dimsD[3];
        } else if (dimsO.getLength() == 3) { // 3D dataset (defined by sensor mask)
            stepSizeO = dimsO[2];
            stepSizeD = dimsD[2];
        } else { // Something wrong.
            Helper::printErrorMsg("Something wrong with dataset dims");
            return;
        }
        dataO = new float[stepSizeO]();
        dataD = new float[stepSizeD]();

        int kMaxExp = H5Helper::CompressHelper::kMaxExpU;
        if (datasetDecoded->hasAttribute("c_max_exp")) {
            kMaxExp = int(datasetDecoded->readAttributeI("c_max_exp", false));
        } else if (datasetDecoded->getName() == "/" + H5Helper::P_INDEX_DATASET_C || datasetDecoded->getName() == "/" + H5Helper::P_CUBOID_DATASET_C) {
            kMaxExp = H5Helper::CompressHelper::kMaxExpP;
        }

        datasetOriginal->setNumberOfElmsToLoad(stepSizeO);
        datasetDecoded->setNumberOfElmsToLoad(stepSizeD);
        hsize_t numberOfBlocks = datasetOriginal->getNumberOfBlocks();

        // Reading and subtraction
        for (hsize_t i = 0; i < numberOfBlocks; i++) {
            datasetDecoded->readBlock(i, offset, count, dataD, log);
            datasetOriginal->readBlock(i, offset, count, dataO, log);
            uint8_t *dataD8 = reinterpret_cast<uint8_t*>(dataD);
            hssize_t p8 = 0;
            for (hssize_t i = 0; i < hssize_t(count.getSize()); i += 2) {
                H5Helper::floatC sCO(dataO[i], dataO[i + 1]);
                H5Helper::floatC sCD;
                H5Helper::CompressHelper::convert40bToFloatC(&dataD8[p8], sCD, kMaxExp);
                dataO[i] = sCD.real() - sCO.real();
                dataO[i + 1] = sCD.imag() - sCO.imag();
                sum += double(abs(dataO[i]));
                sum += double(abs(dataO[i + 1]));
                sumO2 += double(sCO.real() * sCO.real());
                sumO2 += double(sCO.imag() * sCO.imag());
                sum2 += double(dataO[i] * dataO[i]);
                sum2 += double(dataO[i + 1] * dataO[i + 1]);
                // Min/max values
                hsize_t linearOffset;
                convertMultiDimToLinear(offset, linearOffset, datasetOriginal->getDims());
                H5Helper::checkOrSetMinMaxValue(minV, maxV, dataO[i], minVIndex, maxVIndex, linearOffset + hsize_t(i));
                H5Helper::checkOrSetMinMaxValue(minV, maxV, dataO[i + 1], minVIndex, maxVIndex, linearOffset + hsize_t(i + 1));
                H5Helper::checkOrSetMinMaxValue(minVO, maxVO, sCO.real(), minVOIndex, maxVOIndex, linearOffset + hsize_t(i));
                H5Helper::checkOrSetMinMaxValue(minVO, maxVO, sCO.imag(), minVOIndex, maxVOIndex, linearOffset + hsize_t(i + 1));
                p8 += 5;
            }
            dstDataset->writeDataset(offset, count, dataO, log);
        }
    } else {
        dataO = new float[datasetOriginal->getGeneralBlockDims().getSize()]();
        dataD = new float[datasetOriginal->getGeneralBlockDims().getSize()]();

        hsize_t numberOfElmsToLoad = std::min(datasetDecoded->getRealNumberOfElmsToLoad(), datasetOriginal->getRealNumberOfElmsToLoad());
        datasetDecoded->setNumberOfElmsToLoad(numberOfElmsToLoad);
        datasetOriginal->setNumberOfElmsToLoad(numberOfElmsToLoad);
        hsize_t numberOfBlocks = std::min(datasetDecoded->getNumberOfBlocks(), datasetOriginal->getNumberOfBlocks());

        // Reading and subtraction
        for (hsize_t i = 0; i < numberOfBlocks; i++) {
            datasetDecoded->readBlock(i, offset, count, dataD, log);
            datasetOriginal->readBlock(i, offset, count, dataO, log);
            for (hssize_t i = 0; i < hssize_t(count.getSize()); i++) {
                dataD[i] = dataD[i] - dataO[i];
                sum += double(abs(dataD[i]));
                sumO2 += double(dataO[i] * dataO[i]);
                sum2 += double(dataD[i] * dataD[i]);
                // Min/max values
                hsize_t linearOffset;
                convertMultiDimToLinear(offset, linearOffset, datasetOriginal->getDims());
                H5Helper::checkOrSetMinMaxValue(minV, maxV, dataD[i], minVIndex, maxVIndex, linearOffset + hsize_t(i));
                H5Helper::checkOrSetMinMaxValue(minVO, maxVO, dataO[i], minVOIndex, maxVOIndex, linearOffset + hsize_t(i));
            }
            dstDataset->writeDataset(offset, count, dataD, log);
        }
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
    //dstDataset->setAttribute(H5Helper::C_TYPE_ATTR, "s", log);
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

    Helper::printDebugTwoColumnsS("Mean error", std::to_string((meanError / maxValue) * 100) + " %");
    Helper::printDebugTwoColumnsS("Max error", std::to_string((maxError / maxValue) * 100) + " %");
    Helper::printDebugTwoColumnsS("MSE", mse);
    Helper::printDebugTwoColumnsS("SNR", std::to_string(10 * log10(meanO2 / float(mse))) + " dB");
    Helper::printDebugTwoColumnsS("PSNR", std::to_string(10 * log10((maxValue * maxValue) / float(mse))) + " dB");

    double t1 = H5Helper::getTime();
    Helper::printDebugTime("datasets difference", t0, t1);

    getOutputFile()->closeDataset(dstDataset, log);
}
