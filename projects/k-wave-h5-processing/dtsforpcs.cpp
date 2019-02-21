/**
 * @file        dtsforpcs.cpp
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        8  September 2016 (created) <br>
 *              20 February  2019 (updated)
 *
 * @brief       The implementation file containing DtsForPcs class definition.
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

#include "dtsforpcs.h"

/**
 * @brief Creates DtsForPcs with given files context and settings
 * @param[in] filesContext Files context
 * @param[in] settings Settings
 */
DtsForPcs::DtsForPcs(FilesContext *filesContext, Settings *settings)
{
    // Find and get sensor mask dataset
    sensorMaskIndexDataset = findAndGetDataset(H5Helper::SENSOR_MASK_INDEX_DATASET, filesContext->getSimOutputFile(), filesContext->getSimInputFile());
    sensorMaskCornersDataset = findAndGetDataset(H5Helper::SENSOR_MASK_CORNERS_DATASET, filesContext->getSimOutputFile(), filesContext->getSimInputFile());

    if (sensorMaskIndexDataset) {
        // Get sensor mask size
        H5Helper::Vector3D size = sensorMaskIndexDataset->getDims();
        if (sensorMaskIndexDataset->getRank() == 3 && size.z() == 1 && size.y() == 1) {
            sensorMaskSize = size.x();
            sensorMaskType = 0;
        } else {
            Helper::printErrorMsg("Wrong sensor mask index dataset");
            exit(EXIT_FAILURE);
        }
    } else if (sensorMaskCornersDataset) {
        // Get sensor mask size
        H5Helper::Vector3D size = sensorMaskCornersDataset->getDims();
        if ((size.x() % 6) == 0 && size.z() == 1/* && size.y() == 1*/) {
            sensorMaskSize = size.x();
            sensorMaskType = 1;
        } else {
            Helper::printErrorMsg("Wrong sensor mask corners dataset");
            exit(EXIT_FAILURE);
        }
    } else {
        Helper::printDebugMsg("Sensor mask is not in simulation output or input file");
    }

    if (settings->getFrequency() > 0 && settings->getPeriod() > 0) {
        Helper::printMsg(settings->getParamsDefinition().getHelp());
        Helper::printErrorMsg("Set period or frequency, not both");
        exit(EXIT_FAILURE);
    }

    // Get period from signal
    if (settings->getFlagComputePeriod()) {
        // Try to open the p_source_input dataset for getting the simulation frequency
        H5Helper::Dataset *dataset = findAndGetDataset(H5Helper::P_SOURCE_INPUT_DATASET, filesContext->getSimOutputFile(), filesContext->getSimInputFile());
        if (dataset == nullptr) {
            dataset = findAndGetDataset(H5Helper::P_INDEX_DATASET, filesContext->getSimOutputFile());
        }
        if (dataset == nullptr) {
            dataset = findAndGetDataset(H5Helper::P_CUBOID_DATASET, filesContext->getSimOutputFile());
        }
        if (dataset) {
            float *data = nullptr;
            hsize_t length = 0;
            hsize_t limit = 500;
            if (dataset->getRank() == 3) {
                H5Helper::Vector3D dims = dataset->getDims();
                length = dims.y() > limit ? limit : dims.y();
                dataset->readDataset(H5Helper::Vector3D(0, dims.y() - length, hsize_t(dims.x() / 2)), H5Helper::Vector3D(1, length, 1), data, Helper::enableDebugMsgs);
            } else if (dataset->getRank() == 4) {
                H5Helper::Vector4D dims = dataset->getDims();
                length = dims.t() > limit ? limit : dims.t();
                dataset->readDataset(H5Helper::Vector4D(dims.t() - length, hsize_t(dims.z() / 2), hsize_t(dims.y() / 2), hsize_t(dims.x() / 2)), H5Helper::Vector4D(length, 1, 1, 1), data, Helper::enableDebugMsgs);
            }
            settings->setPeriod(Helper::roundf(H5Helper::CompressHelper::findPeriod(data, length), 3));
            dataset->setAttribute(H5Helper::PERIOD_ATTR, settings->getPeriod(), Helper::enableDebugMsgs);
            delete[] data;
            data = nullptr;
            dataset->getFile()->closeDataset(dataset, Helper::enableDebugMsgs);
        } else {
            Helper::printErrorMsg("Missing signal for the computing of period");
            exit(EXIT_FAILURE);
        }
    }

    // TODO check this
    if (settings->getFrequency() > 0) {
        if (filesContext->getSimOutputFile()->getDValues().t() > 0) {
            settings->setPeriod(filesContext->getSimOutputFile()->getPeriod(settings->getFrequency()));
        } else if (filesContext->getSimInputFile() != nullptr && filesContext->getSimInputFile()->getDValues().t() > 0) {
            settings->setPeriod(filesContext->getSimInputFile()->getPeriod(settings->getFrequency()));
        } else {
            Helper::printErrorMsg("Cannot compute period from frequency, missing dt dataset");
            exit(EXIT_FAILURE);
        }
    }

    if (settings->getPeriod() == 0.0f) {
        H5Helper::Dataset *dataset = findAndGetDataset(H5Helper::P_SOURCE_INPUT_DATASET, filesContext->getSimOutputFile(), filesContext->getSimInputFile());
        if (dataset == nullptr) {
            dataset = findAndGetDataset(H5Helper::P_INDEX_DATASET, filesContext->getSimOutputFile());
        }
        if (dataset == nullptr) {
            dataset = findAndGetDataset(H5Helper::P_CUBOID_DATASET, filesContext->getSimOutputFile());
        }

        if (dataset) {
            if (dataset->hasAttribute(H5Helper::PERIOD_ATTR)) {
                settings->setPeriod(dataset->readAttributeF(H5Helper::PERIOD_ATTR, Helper::enableDebugMsgs));
            }
            dataset->getFile()->closeDataset(dataset, Helper::enableDebugMsgs);
        }
    }

    // Save original dims
    nDims = filesContext->getSimOutputFile()->getNDims();

    // Find datasets for processing
    Helper::printDebugTitle("Find datasets for processing");
    H5Helper::Group *group = filesContext->getSimOutputFile()->openGroup("/", Helper::enableDebugMsgs);
    findDatasetsForProcessing(group, settings);
    filesContext->getSimOutputFile()->closeGroup("/", Helper::enableDebugMsgs);

    // Find datasets for processing in HDF5PcsInputFile
    if (filesContext->getPcsInputFile()) {
        group = filesContext->getPcsInputFile()->openGroup("/", Helper::enableDebugMsgs);
        findDatasetsForProcessing(group, settings);
        filesContext->getPcsInputFile()->closeGroup("/", Helper::enableDebugMsgs);
    }
}

/**
 * @brief Returns nDims
 * @return nDims
 */
H5Helper::Vector4D DtsForPcs::getNDims() const
{
    return nDims;
}

/**
 * @brief Returns sensor mask index dataset
 * @return Sensor mask index dataset
 */
H5Helper::Dataset *DtsForPcs::getSensorMaskIndexDataset() const
{
    return sensorMaskIndexDataset;
}

/**
 * @brief Returns sensor mask corner dataset
 * @return Sensor mask corner dataset
 */
H5Helper::Dataset *DtsForPcs::getSensorMaskCornersDataset() const
{
    return sensorMaskCornersDataset;
}

/**
 * @brief Returns datasets (by type)
 * @param[in] datasetType Dataset type (optional)
 * @return Datasets
 */
H5Helper::MapOfDatasets DtsForPcs::getDatasets(H5Helper::DatasetType datasetType) const
{
    if (datasetType == H5Helper::DatasetType::ALL) {
        return datasets;
    } else {
        H5Helper::MapOfDatasets map = datasets;
        H5Helper::MapOfDatasets filteredDatasets;
        for (H5Helper::MapOfDatasetsIt it = map.begin(); it != map.end(); ++it) {
            H5Helper::Dataset *dataset = it->second;
            if (datasetType == dataset->getType(sensorMaskSize))
                filteredDatasets.insert(H5Helper::PairOfDatasets(dataset->getName(), dataset));
        }
        return filteredDatasets;
    }
}

/**
 * @brief Returns sensor mask size
 * @return Sensor mask size
 */
hsize_t DtsForPcs::getSensorMaskSize() const
{
    return sensorMaskSize;
}

/**
 * @brief Returns sensor mask type
 * @return Sensor mask type
 */
hsize_t DtsForPcs::getSensorMaskType() const
{
    return sensorMaskType;
}

/**
 * @brief Finds and get dataset with given name from given files
 * @param[in] name Dataset name
 * @param[in] simOutputFile Simulation output file
 * @param[in] simInputFile Simulation input file (optional)
 * @return Dataset
 */
H5Helper::Dataset *DtsForPcs::findAndGetDataset(const std::string name, H5Helper::File *simOutputFile, H5Helper::File *simInputFile)
{
    H5Helper::Dataset *dataset = nullptr;
    Helper::printDebugTitle("Find and get "+ name +" dataset");

    if (simOutputFile->objExistsByName(name)) {
        // Try to load dataset from simulation output file
        try {
            dataset = simOutputFile->openDataset(name, Helper::enableDebugMsgs);
        } catch(std::exception &e) {
            Helper::printErrorMsg(e.what());
            std::exit(EXIT_FAILURE);
        }
    } else if (simInputFile != nullptr && simInputFile->objExistsByName(name)) {
        // Try to load dataset from simulation input file
        try {
            dataset = simInputFile->openDataset(name, Helper::enableDebugMsgs);
        } catch(std::exception &e) {
            Helper::printErrorMsg(e.what());
            std::exit(EXIT_FAILURE);
        }
    } else {
        Helper::printDebugMsg("The " + name + " dataset is not in simulation output or input file");
    }
    return dataset;
}

/**
 * @brief Finds datasets for processing
 * @param[in] group Group to searching
 * @param[in] settings Settings
 */
void DtsForPcs::findDatasetsForProcessing(const H5Helper::Group *group, const Settings *settings)
{
    for (hsize_t i = 0; i < group->getNumObjs(); i++) {
        H5G_obj_t type = group->getObjTypeByIdx(i);
        std::string name = group->getObjNameByIdx(i);

        // Datasets
        if (type == H5G_DATASET) {
            if (group->getName() != "/")
                name = group->getName() + "/" + name;
            else
                name = "/" + name;
            // Filter by selected names
            if (isFiltered(name, settings))
                continue;

            H5Helper::Dataset *dataset = group->openDataset(i, Helper::enableDebugMsgs);
            H5Helper::DatasetType datasetType = dataset->getType(sensorMaskSize);

            if (datasetType != H5Helper::DatasetType::UNKNOWN) {
                bool tmpFlag = Helper::enableDebugMsgs;
                if (settings->getFlagInfo()) {
                    Helper::enableDebugMsgs = true;
                }
                datasets.insert(H5Helper::PairOfDatasets(dataset->getName(), dataset));
                Helper::printDebugMsg("----> " + dataset->getTypeString(datasetType) + " dataset: " + dataset->getName());
                if (settings->getFlagInfo()) {
                    Helper::printDebugTwoColumnsTab("size", dataset->getDims());
                    Helper::printDebugTwoColumnsTab("chunk size", dataset->getChunkDims());
                    if (dataset->getSize() == 1) {
                        if (dataset->isFloatType()) {
                            float data;
                            dataset->readDataset(data, false);
                            Helper::printDebugTwoColumnsTab("value", data);
                        } else if (dataset->isIntegerType()) {
                            hsize_t data;
                            dataset->readDataset(data, false);
                            Helper::printDebugTwoColumnsTab("value", data);
                        }
                    }
                }
                // Find min/max values
                if (settings->getFlagFindMinMax()) {
                    dataset->findAndSetGlobalMinAndMaxValue(false, settings->getFlagLog());
                }
                // Print attributes
                if (settings->getFlagInfo()) {
                    if (dataset->getNumAttrs() > 0) {
                        Helper::printDebugMsg2S("Attributes");
                    }
                    for (hsize_t i = 0; i < dataset->getNumAttrs(); i++) {
                        H5Helper::Attribute *attribute = dataset->getAttribute(i);
                        Helper::printDebugTwoColumnsTab(attribute->getName(), attribute->getStringValue());
                        delete attribute;
                        attribute = nullptr;
                    }
                }
                if (settings->getFlagInfo()) {
                    Helper::enableDebugMsgs = tmpFlag;
                }
            }
            // Unknown type
            else {
                group->closeDataset(dataset, false);
            }
        }
        // Groups
        if (type == H5G_GROUP) {
            H5Helper::Group *nextGroup = group->openGroup(i, false);
            findDatasetsForProcessing(nextGroup, settings);
            group->closeGroup(nextGroup, false);
        }
    }
}

/**
 * @brief Checks name is not filtered
 * @param[in] name Name to check
 * @param[in] settings Settings
 * @return True/False
 */
bool DtsForPcs::isFiltered(std::string name, const Settings *settings)
{
    if (settings->getFlagNames()) {
        std::list<std::string> names = settings->getNames();
        // Iterate
        for (std::list<std::string>::const_iterator it = names.begin(); it != names.end(); ++it) {
            std::string str = *it;
            if (str.find("/") != 0)
                str.insert(0, "/");
            if (str == name)
                return false;
            if (str.find("/") != str.length() - 1)
                str.append("/");
            if (name.find(str) != std::string::npos) {
                return false;
            }
        }
        return true;
    } else {
        return false;
    }
}
