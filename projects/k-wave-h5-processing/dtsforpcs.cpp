/**
 * @file        dtsforpcs.cpp
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        8  September 2016 (created) \n
 *              27 February  2017 (updated)
 *
 * @brief       The implementation file containing datasets list.
 *
 * @license     This application is for preprocessing the HDF5 data created by the k-Wave toolbox - http://www.k-wave.org.
 *              k-Wave h5 processing is free software.
 *
 * @copyright   Copyright © 2017, Petr Kleparnik, VUT FIT Brno. All Rights Reserved.
 *
 */

#include "dtsforpcs.h"

DtsForPcs::DtsForPcs(FilesContext *filesContext, Settings *settings)
{
    // Find and get sensor mask dataset
    sensorMaskIndexDataset = findAndGetDataset(HDF5Helper::SENSOR_MASK_INDEX_DATASET, filesContext->getHDF5SimOutputFile(), filesContext->getHDF5SimInputFile());
    sensorMaskCornersDataset = findAndGetDataset(HDF5Helper::SENSOR_MASK_CORNERS_DATASET, filesContext->getHDF5SimOutputFile(), filesContext->getHDF5SimInputFile());

    if (sensorMaskIndexDataset) {
        // Get sensor mask size
        HDF5Helper::HDF5Vector3D size = sensorMaskIndexDataset->getDims();
        if (sensorMaskIndexDataset->getRank() == 3 && size.z() == 1 && size.y() == 1) {
            sensorMaskSize = size.x();
            sensorMaskType = 0;
        } else {
            Helper::printErrorMsg("Wrong sensor mask index dataset");
            exit(EXIT_FAILURE);
        }
    } else if (sensorMaskCornersDataset) {
        // Get sensor mask size
        HDF5Helper::HDF5Vector3D size = sensorMaskCornersDataset->getDims();
        if ((size.x() % 6) == 0 && size.z() == 1 && size.y() == 1) {
            sensorMaskSize = size.x();
            sensorMaskType = 1;
        } else {
            Helper::printErrorMsg("Wrong sensor mask corners dataset");
            exit(EXIT_FAILURE);
        }
    } else {
        Helper::printDebugMsg("\n----> Sensor mask is not in simulation output or input file");
    }

    // Try to open the p_source_input dataset for getting the simulation frequency
    sourceInputDataset = findAndGetDataset(HDF5Helper::P_SOURCE_INPUT_DATASET, filesContext->getHDF5SimOutputFile(), filesContext->getHDF5SimInputFile());

    // Get period from input signal
    if (!settings->getPeriod() && sourceInputDataset) {
        if (sourceInputDataset->hasAttribute("period")) {
            settings->setPeriod(sourceInputDataset->readAttributeI("period", false));
        } else {
            HDF5Helper::HDF5Vector3D dims = sourceInputDataset->getDims();
            float *data = 0;
            sourceInputDataset->readDataset(HDF5Helper::HDF5Vector3D(0, 0, 0), HDF5Helper::HDF5Vector3D(1, dims.y(), 1), data);
            settings->setPeriod(Helper::getPeriod(data, dims.y()));
            sourceInputDataset->setAttribute("period", settings->getPeriod());
            delete[] data;
        }
    }

    // Save original dims
    nDims = filesContext->getHDF5SimOutputFile()->getNdims();

    // Find datasets for processing
    Helper::printDebugTitle("Find datasets for processing");
    HDF5Helper::HDF5Group *group = filesContext->getHDF5SimOutputFile()->openGroup("/");
    findDatasetsForProcessing(group, settings);
    filesContext->getHDF5SimOutputFile()->closeGroup("/");
}

HDF5Helper::HDF5Vector4D DtsForPcs::getNDims() const
{
    return nDims;
}

HDF5Helper::HDF5Dataset *DtsForPcs::getSensorMaskIndexDataset() const
{
    return sensorMaskIndexDataset;
}

HDF5Helper::HDF5Dataset *DtsForPcs::getSensorMaskCornersDataset() const
{
    return sensorMaskCornersDataset;
}

HDF5Helper::HDF5Dataset *DtsForPcs::getSourceInputDataset() const
{
    return sourceInputDataset;
}

hsize_t DtsForPcs::getSensorMaskType() const
{
    return sensorMaskType;
}

HDF5Helper::MapOfDatasets DtsForPcs::getDatasets(HDF5Helper::HDF5DatasetType datasetType) const
{
    if (datasetType == HDF5Helper::HDF5DatasetType::ALL) {
        return datasets;
    } else {
        HDF5Helper::MapOfDatasets map = datasets;
        HDF5Helper::MapOfDatasets filteredDatasets;
        for (HDF5Helper::MapOfDatasetsIt it = map.begin(); it != map.end(); ++it) {
            HDF5Helper::HDF5Dataset *dataset = it->second;
            if (datasetType == dataset->getType(sensorMaskSize))
                filteredDatasets.insert(HDF5Helper::PairOfDatasets(dataset->getName(), dataset));
        }
        return filteredDatasets;
    }
}

hsize_t DtsForPcs::getSensorMaskSize() const
{
    return sensorMaskSize;
}

HDF5Helper::HDF5Dataset *DtsForPcs::findAndGetDataset(const std::string name, HDF5Helper::File *hDF5SimOutputFile, HDF5Helper::File *hDF5SimInputFile = 0)
{
    HDF5Helper::HDF5Dataset *dataset = 0;
    Helper::printDebugTitle("Find and get "+ name +" dataset");

    if (hDF5SimOutputFile->objExistsByName(name)) {
        // Try to load dataset from simulation output file
        try {
            dataset = hDF5SimOutputFile->openDataset(name);
        } catch(std::exception &e) {
            Helper::printErrorMsg(e.what());
            std::exit(EXIT_FAILURE);
        }
    } else if (hDF5SimInputFile != 0 && hDF5SimInputFile->objExistsByName(name)) {
        // Try to load dataset from simulation input file
        try {
            dataset = hDF5SimInputFile->openDataset(name);
        } catch(std::exception &e) {
            Helper::printErrorMsg(e.what());
            std::exit(EXIT_FAILURE);
        }
    } else {
        Helper::printDebugMsg("The " + name + " dataset is not in simulation output or input file");
    }
    return dataset;
}

void DtsForPcs::findDatasetsForProcessing(HDF5Helper::HDF5Group *group, Settings *settings)
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

            HDF5Helper::HDF5Dataset *dataset = group->openDataset(i);
            HDF5Helper::HDF5DatasetType datasetType = dataset->getType(sensorMaskSize);

            if (datasetType != HDF5Helper::HDF5DatasetType::UNKNOWN) {
                datasets.insert(HDF5Helper::PairOfDatasets(dataset->getName(), dataset));
                std::cout << "----> " << dataset->getTypeString(datasetType) << " dataset: " << dataset->getName() << ", size: " << dataset->getDims() << std::endl;
                std::cout << std::endl;
            }
            // Unknown type
            else {
                group->closeDataset(dataset);
            }
        }
        // Groups
        if (type == H5G_GROUP) {
            HDF5Helper::HDF5Group *nextGroup = group->openGroup(i);
            findDatasetsForProcessing(nextGroup, settings);
            group->closeGroup(nextGroup);
        }
    }
}

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
