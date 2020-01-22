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
        // Compare 2 selected datasets
        if (getSettings()->getNames().size() == 2) {
            H5Helper::Dataset *dataset1 = getDtsForPcs()->getDatasets().at(getSettings()->getNames().back());
            H5Helper::Dataset *dataset2 = getDtsForPcs()->getDatasets2().at(getSettings()->getNames().front());
            H5Helper::DatasetType dataset1Type = dataset1->getType(sensorMaskSize);
            H5Helper::DatasetType dataset2Type = dataset2->getType(sensorMaskSize);

            if (dataset1->getRank() == dataset1->getRank()) {
                if( ((dataset1Type == H5Helper::DatasetType::TIME_STEPS_D_INDEX || dataset1Type == H5Helper::DatasetType::TIME_STEPS_INDEX)
                     && (dataset2Type == H5Helper::DatasetType::TIME_STEPS_D_INDEX || dataset2Type == H5Helper::DatasetType::TIME_STEPS_INDEX)
                     && (H5Helper::Vector3D(dataset1->getDims()).x() == H5Helper::Vector3D(dataset2->getDims()).x()))
                    || (H5Helper::Vector3D(dataset1->getDims()) == H5Helper::Vector3D(dataset2->getDims()))) {

                    if (H5Helper::Vector3D(dataset1->getDims()).y() < H5Helper::Vector3D(dataset2->getDims()).y()
                        || (dataset1->getRank() == 4 && H5Helper::Vector4D(dataset1->getDims()).t() < H5Helper::Vector4D(dataset2->getDims()).t())) {
                        Helper::printDebugMsg("Subtraction of datasets " + dataset2->getName() + " and " + dataset1->getName());
                        subtractDatasets(dataset2, dataset1, getSettings()->getFlagLog());
                    } else {
                        Helper::printDebugMsg("Subtraction of datasets " + dataset1->getName() + " and " + dataset2->getName());
                        subtractDatasets(dataset1, dataset2, getSettings()->getFlagLog());
                    }
                    Helper::printDebugMsg("Subtraction of datasets done");
                    count++;
                }
            }
        } else { // Compare original and decoded datasets
          for (H5Helper::MapOfDatasetsIt it = map.begin(); it != map.end(); ++it) {
              H5Helper::Dataset *datasetOriginal = it->second;
              H5Helper::DatasetType datasetType = datasetOriginal->getType(sensorMaskSize);
              if (checkDatasetType(datasetType, types)) {
                  H5Helper::Dataset *datasetDecoded = nullptr;

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

    std::string diffName = datasetOriginal->getName() + "-" + datasetDecoded->getOnlyName();
    if (datasetDecoded->hasAttribute(H5Helper::SRC_DATASET_NAME_ATTR)) {
        diffName = datasetDecoded->readAttributeS(H5Helper::SRC_DATASET_NAME_ATTR, log);
    }
    getOutputFile()->createDatasetF(diffName + "_s", outputDims, outputChunkDims, true, log);
    H5Helper::Dataset *dstDataset = getOutputFile()->openDataset(diffName + "_s", log);

    datasetDecoded->setNumberOfElmsToLoad(datasetOriginal->getNumberOfElmsToLoad());

    // Variables for block reading
    float *dataO = new float[datasetOriginal->getGeneralBlockDims().getSize()]();
    float *dataD = new float[datasetOriginal->getGeneralBlockDims().getSize()]();
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

    const float r0 = (float(511) / (2 * float(M_PI)));

    // Reading and subtraction
    for (hsize_t i = 0; i < datasetOriginal->getNumberOfBlocks(); i++) {
        // First read decoded -> it is larger
        datasetDecoded->readBlock(i, offset, count, dataD, log);
        datasetOriginal->readBlock(i, offset, count, dataO, log);
        // The count is from original dataset and is smaller for last step than is in the decoded dataset

        //#pragma omp parallel for reduction(+ : sum, sumO2, sum2)
        //for (hssize_t i = 0; i < hssize_t(count.getSize()) / 2; i += 2) {
        for (hssize_t i = 0; i < hssize_t(count.getSize()); i++) {

            //float dValue = dataD[i];
            //dValue = dataO[i];

            /*H5Helper::floatC dValueFC(dataO[i], dataO[i + 1]);
            //std::cout << dValueFC << std::endl;
            float dValueFC32 = H5Helper::CompressHelper::convertFloatCTo32b(dValueFC, 120000.0f);
            H5Helper::floatC dValueFCn = H5Helper::CompressHelper::convert32bToFloatC(dValueFC32, 120000.0f);*/


            /*int16_t angleI = 0;
            int32_t r;

            float sC1Abs = abs(dValueFC);
            float sC1Arg = arg(dValueFC);

            angleI = int16_t(roundf(sC1Arg * r0));
            sC1Arg = float(angleI) / r0;

            std::memcpy(&r, &sC1Abs, sizeof r);
            r >>= 10;
            r <<= 10;
            std::memcpy(&sC1Abs, &r, sizeof r);

            dValueFC = std::polar(sC1Abs, sC1Arg);

            dValue = dValueFC.imag();*/

            /*unsigned r;
            std::memcpy(&r, &dValue, sizeof r);
            //std::bitset<32> f0(r);
            r >>= 16;
            r <<= 16;
            //std::bitset<32> f1(r);
            std::memcpy(&dValue, &r, sizeof dValue);*/

            /*float rC = float(32768);
            //if (abs(dValue) > 16777216) {
            //    rC /= 67108864;
            //} else if (abs(dValue) > 4) {
                rC /= 16777216;
            //} else if (abs(dValue) > 1) {
            //    rC /= 4;
            //}
            int16_t dValueI = int16_t(roundf(dValue * rC));
            dValueI >>= 1;
            dValueI <<= 1;
            //std::bitset<16> dValueIb(dValueI);
            //dValueI = (dValueI & ~(1UL << 15)) | (1 << 15);
            //dValueIb[15] = 1;
            //std::cout << dValueIb << " " << dValueIb.to_ulong()  << " " << dValueI << '\n';

            dValue = float(dValueI) / rC;*/

            dataD[i] = dataO[i] - dataD[i];
            //dataD[i] = dataO[i] - dValueFCn.real();
            //dataD[i + 1] = dataO[i + 1] - dValueFCn.imag();

            sum += double(abs(dataD[i]));
            //sum += double(abs(dataD[i + 1]));
            sumO2 += double(dataO[i] * dataO[i]);
            //sumO2 += double(dataO[i + 1] * dataO[i + 1]);
            sum2 += double(dataD[i] * dataD[i]);
            //sum2 += double(dataD[i + 1] * dataD[i + 1]);
            // Min/max values
            hsize_t linearOffset;
            convertMultiDimToLinear(offset, linearOffset, datasetDecoded->getDims());
            H5Helper::checkOrSetMinMaxValue(minV, maxV, dataD[i], minVIndex, maxVIndex, linearOffset + hsize_t(i));
            //H5Helper::checkOrSetMinMaxValue(minV, maxV, dataD[i + 1], minVIndex, maxVIndex, linearOffset + hsize_t(i + 1));
            H5Helper::checkOrSetMinMaxValue(minVO, maxVO, dataO[i], minVOIndex, maxVOIndex, linearOffset + hsize_t(i));
            //H5Helper::checkOrSetMinMaxValue(minVO, maxVO, dataO[i + 1], minVOIndex, maxVOIndex, linearOffset + hsize_t(i + 1));
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

    Helper::printDebugTwoColumnsS("Mean error", std::to_string((meanError / maxValue) * 100) + " %");
    Helper::printDebugTwoColumnsS("Max error", std::to_string((maxError / maxValue) * 100) + " %");
    Helper::printDebugTwoColumnsS("MSE", mse);
    Helper::printDebugTwoColumnsS("SNR", std::to_string(10 * log10(meanO2 / float(mse))) + " dB");
    Helper::printDebugTwoColumnsS("PSNR", std::to_string(10 * log10((maxValue * maxValue) / float(mse))) + " dB");

    double t1 = H5Helper::getTime();
    Helper::printDebugTime("datasets difference", t0, t1);

    getOutputFile()->closeDataset(dstDataset, log);
}
