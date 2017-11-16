/**
 * @file        dtsforpcs.cpp
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        8  September 2016 (created) \n
 *              11 September 2017 (updated)
 *
 * @brief       The implementation file containing DtsForPcs class definition.
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

#include "dtsforpcs.h"

/**
 * @brief Creates DtsForPcs with given files context and settings
 * @param[in] filesContext Files context
 * @param[in] settings Settings
 */
DtsForPcs::DtsForPcs(FilesContext *filesContext, Settings *settings)
{
    // Find and get sensor mask dataset
    sensorMaskIndexDataset = findAndGetDataset(HDF5Helper::SENSOR_MASK_INDEX_DATASET, filesContext->getSimOutputFile(), filesContext->getSimInputFile());
    sensorMaskCornersDataset = findAndGetDataset(HDF5Helper::SENSOR_MASK_CORNERS_DATASET, filesContext->getSimOutputFile(), filesContext->getSimInputFile());

    if (sensorMaskIndexDataset) {
        // Get sensor mask size
        HDF5Helper::Vector3D size = sensorMaskIndexDataset->getDims();
        if (sensorMaskIndexDataset->getRank() == 3 && size.z() == 1 && size.y() == 1) {
            sensorMaskSize = size.x();
            sensorMaskType = 0;
        } else {
            Helper::printErrorMsg("Wrong sensor mask index dataset");
            exit(EXIT_FAILURE);
        }
    } else if (sensorMaskCornersDataset) {
        // Get sensor mask size
        HDF5Helper::Vector3D size = sensorMaskCornersDataset->getDims();
        if ((size.x() % 6) == 0 && size.z() == 1 && size.y() == 1) {
            sensorMaskSize = size.x();
            sensorMaskType = 1;
        } else {
            Helper::printErrorMsg("Wrong sensor mask corners dataset");
            exit(EXIT_FAILURE);
        }
    } else {
        Helper::printDebugMsg("Sensor mask is not in simulation output or input file");
    }

    // Try to open the p_source_input dataset for getting the simulation frequency
    sourceInputDataset = findAndGetDataset(HDF5Helper::P_SOURCE_INPUT_DATASET, filesContext->getSimOutputFile(), filesContext->getSimInputFile());

    // Get period from input signal
    if (!settings->getPeriod() && sourceInputDataset) {
        if (sourceInputDataset->hasAttribute("period")) {
            settings->setPeriod(sourceInputDataset->readAttributeI("period", false));
        } else if (settings->getFlagComputePeriod()){
            HDF5Helper::Vector3D dims = sourceInputDataset->getDims();
            float *data = 0;
            sourceInputDataset->readDataset(HDF5Helper::Vector3D(0, 0, 0), HDF5Helper::Vector3D(1, dims.y(), 1), data);
            settings->setPeriod(CompressHelper::getPeriod(data, dims.y()));
            sourceInputDataset->setAttribute("period", settings->getPeriod());
            delete[] data;
        }
    }

    // Save original dims
    nDims = filesContext->getSimOutputFile()->getNdims();

    // Find datasets for processing
    Helper::printDebugTitle("Find datasets for processing");
    HDF5Helper::Group *group = filesContext->getSimOutputFile()->openGroup("/", Helper::enableDebugMsgs);
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
HDF5Helper::Vector4D DtsForPcs::getNDims() const
{
    return nDims;
}

/**
 * @brief Returns sensor mask index dataset
 * @return Sensor mask index dataset
 */
HDF5Helper::Dataset *DtsForPcs::getSensorMaskIndexDataset() const
{
    return sensorMaskIndexDataset;
}

/**
 * @brief Returns sensor mask corner dataset
 * @return Sensor mask corner dataset
 */
HDF5Helper::Dataset *DtsForPcs::getSensorMaskCornersDataset() const
{
    return sensorMaskCornersDataset;
}

/**
 * @brief Returns source input dataset
 * @return Source input dataset
 */
HDF5Helper::Dataset *DtsForPcs::getSourceInputDataset() const
{
    return sourceInputDataset;
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
 * @brief Returns datasets (by type)
 * @param[in] datasetType Dataset type (optional)
 * @return Datasets
 */
HDF5Helper::MapOfDatasets DtsForPcs::getDatasets(HDF5Helper::DatasetType datasetType) const
{
    if (datasetType == HDF5Helper::DatasetType::ALL) {
        return datasets;
    } else {
        HDF5Helper::MapOfDatasets map = datasets;
        HDF5Helper::MapOfDatasets filteredDatasets;
        for (HDF5Helper::MapOfDatasetsIt it = map.begin(); it != map.end(); ++it) {
            HDF5Helper::Dataset *dataset = it->second;
            if (datasetType == dataset->getType(sensorMaskSize))
                filteredDatasets.insert(HDF5Helper::PairOfDatasets(dataset->getName(), dataset));
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
 * @brief Finds and get dataset with given name from given files
 * @param[in] name Dataset name
 * @param[in] simOutputFile Simulation output file
 * @param[in] simInputFile Simulation input file
 * @return Dataset
 */
HDF5Helper::Dataset *DtsForPcs::findAndGetDataset(const std::string name, HDF5Helper::File *simOutputFile, HDF5Helper::File *simInputFile = 0)
{
    HDF5Helper::Dataset *dataset = 0;
    Helper::printDebugTitle("Find and get "+ name +" dataset");

    if (simOutputFile->objExistsByName(name)) {
        // Try to load dataset from simulation output file
        try {
            dataset = simOutputFile->openDataset(name, Helper::enableDebugMsgs);
        } catch(std::exception &e) {
            Helper::printErrorMsg(e.what());
            std::exit(EXIT_FAILURE);
        }
    } else if (simInputFile != 0 && simInputFile->objExistsByName(name)) {
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
void DtsForPcs::findDatasetsForProcessing(HDF5Helper::Group *group, Settings *settings)
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

            HDF5Helper::Dataset *dataset = group->openDataset(i, Helper::enableDebugMsgs);
            HDF5Helper::DatasetType datasetType = dataset->getType(sensorMaskSize);

            if (datasetType != HDF5Helper::DatasetType::UNKNOWN) {
                bool tmpFlag = Helper::enableDebugMsgs;
                if (settings->getFlagInfo()) {
                    Helper::enableDebugMsgs = true;
                }
                datasets.insert(HDF5Helper::PairOfDatasets(dataset->getName(), dataset));
                Helper::printDebugMsg("----> " + dataset->getTypeString(datasetType) + " dataset: " + dataset->getName());
                if (settings->getFlagInfo()) {
                    Helper::printDebugTwoColumnsTab("size", dataset->getDims());
                    Helper::printDebugTwoColumnsTab("chunk size", dataset->getChunkDims());
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
                        HDF5Helper::Attribute *attribute = dataset->getAttribute(i);
                        Helper::printDebugTwoColumnsTab(attribute->getName(), attribute->getStringValue());
                        delete attribute;
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
            HDF5Helper::Group *nextGroup = group->openGroup(i, false);
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
bool DtsForPcs::isFiltered(std::string name, Settings *settings)
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
