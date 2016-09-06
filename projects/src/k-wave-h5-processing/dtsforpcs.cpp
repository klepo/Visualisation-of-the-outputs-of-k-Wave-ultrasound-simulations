#include "dtsforpcs.h"

DtsForPcs::DtsForPcs(FilesContext *filesContext, Settings *settings)
{
    // Find and get sensor mask dataset
    sensorMaskIndexDataset = findAndGetSensorMaskIndexDataset(filesContext->getHDF5SimOutputFile(), filesContext->getHDF5SimInputFile());
    sensorMaskCornersDataset = findAndGetSensorMaskCornersDataset(filesContext->getHDF5SimOutputFile(), filesContext->getHDF5SimInputFile());

    if (sensorMaskIndexDataset != NULL) {
        // Get sensor mask size
        HDF5Helper::HDF5Vector3D size = sensorMaskIndexDataset->getDims();
        if (sensorMaskIndexDataset->getRank() == 3 && size.z() == 1 && size.y() == 1) {
            sensorMaskSize = size.x();
            sensorMaskType = 0;
        } else {
            Helper::printErrorMsg("Wrong sensor mask index dataset");
            exit(EXIT_FAILURE);
        }
    } else if (sensorMaskCornersDataset != NULL) {
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

    // Save original dims
    nDims.w(filesContext->getHDF5SimOutputFile()->getNT());
    nDims.z(filesContext->getHDF5SimOutputFile()->getNZ());
    nDims.y(filesContext->getHDF5SimOutputFile()->getNY());
    nDims.x(filesContext->getHDF5SimOutputFile()->getNX());

    // Find datasets for processing
    Helper::printDebugTitle("Find datasets for processing");
    findDatasetsForProcessing(filesContext->getHDF5SimOutputFile(), settings);
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

hsize_t DtsForPcs::getSensorMaskType() const
{
    return sensorMaskType;
}

hsize_t DtsForPcs::getSensorMaskSize() const
{
    return sensorMaskSize;
}

HDF5Helper::MapOfDatasets DtsForPcs::getDts3DType() const
{
    return dts3DType;
}

HDF5Helper::MapOfDatasets DtsForPcs::getDts3DTypeDsp() const
{
    return dts3DTypeDsp;
}

HDF5Helper::MapOfDatasets DtsForPcs::getDtsMaskType() const
{
    return dtsMaskType;
}

HDF5Helper::MapOfGroups DtsForPcs::getGroupsCuboidType() const
{
    return groupsCuboidType;
}

HDF5Helper::MapOfGroups DtsForPcs::getGroupsCuboidTypeAttr() const
{
    return groupsCuboidTypeAttr;
}

HDF5Helper::MapOfGroups DtsForPcs::getGroupsCuboidTypeAttrDsp() const
{
    return groupsCuboidTypeAttrDsp;
}

HDF5Helper::HDF5Dataset *DtsForPcs::findAndGetSensorMaskIndexDataset(HDF5Helper::File *hDF5SimOutputFile, HDF5Helper::File *hDF5SimInputFile)
{
    HDF5Helper::HDF5Dataset *sensorMaskIndexDataset = NULL;
    Helper::printDebugTitle("Find and get sensor mask index dataset");

    if (hDF5SimOutputFile->objExistsByName(Settings::SENSOR_MASK_INDEX_DATASET)) {
        // Try to load sensor mask index from simulation output file
        try {
            sensorMaskIndexDataset = hDF5SimOutputFile->openDataset(Settings::SENSOR_MASK_INDEX_DATASET);
        } catch(std::exception &e) {
            Helper::printErrorMsg(e.what());
            std::exit(EXIT_FAILURE);
        }
    } else if (hDF5SimInputFile != NULL && hDF5SimInputFile->objExistsByName(Settings::SENSOR_MASK_INDEX_DATASET)){
        // Try to load sensor mask index from simulation input file
        try {
            sensorMaskIndexDataset = hDF5SimInputFile->openDataset(Settings::SENSOR_MASK_INDEX_DATASET);
        } catch(std::exception &e) {
            Helper::printErrorMsg(e.what());
            std::exit(EXIT_FAILURE);
        }
    } else {
        Helper::printDebugMsg("Sensor mask index dataset is not in simulation output or input file");
    }
    return sensorMaskIndexDataset;
}

HDF5Helper::HDF5Dataset *DtsForPcs::findAndGetSensorMaskCornersDataset(HDF5Helper::File *hDF5SimOutputFile, HDF5Helper::File *hDF5SimInputFile)
{
    HDF5Helper::HDF5Dataset *sensorMaskCornersDataset = NULL;
    Helper::printDebugTitle("Find and get sensor mask corners dataset");

    if (hDF5SimOutputFile->objExistsByName(Settings::SENSOR_MASK_CORNERS_DATASET)) {
        // Try to load sensor mask corners from simulation output file
        try {
            sensorMaskCornersDataset = hDF5SimOutputFile->openDataset(Settings::SENSOR_MASK_CORNERS_DATASET);
        } catch(std::exception &e) {
            Helper::printErrorMsg(e.what());
            std::exit(EXIT_FAILURE);
        }
    } else if (hDF5SimInputFile != NULL && hDF5SimInputFile->objExistsByName(Settings::SENSOR_MASK_CORNERS_DATASET)){
        // Try to load sensor mask corners from simulation input file
        try {
            sensorMaskCornersDataset = hDF5SimInputFile->openDataset(Settings::SENSOR_MASK_CORNERS_DATASET);
        } catch(std::exception &e) {
            Helper::printErrorMsg(e.what());
            std::exit(EXIT_FAILURE);
        }
    } else {
        Helper::printDebugMsg("Sensor mask corners dataset is not in simulation output or input file");
    }
    return sensorMaskCornersDataset;
}

void DtsForPcs::findDatasetsForProcessing(HDF5Helper::File *hDF5SimOutputFile, Settings *settings)
{
    for (hsize_t i = 0; i < hDF5SimOutputFile->getNumObjs(); i++) {
        H5G_obj_t type = hDF5SimOutputFile->getObjTypeByIdx(i);
        std::string name = hDF5SimOutputFile->getObjNameByIdx(i);


        // Filter by selected names
        std::list<std::string> names = settings->getNames();
        if (settings->getFlagNames() && std::find(names.begin(), names.end(), name) == names.end()) {
            continue;
        }

        // Datasets
        if (type == H5G_DATASET) {
            HDF5Helper::HDF5Dataset *dataset = hDF5SimOutputFile->openDataset(i);

            // 3D type
            if (dataset->getDims().getLength() == 3) {
                HDF5Helper::HDF5Vector3D size = dataset->getDims();
                if (dataset->getDataTypeClass() == H5T_FLOAT
                        && size.z() == nDims.z()
                        && size.y() == nDims.y()
                        && size.x() == nDims.x()) {
                    dts3DType.insert(HDF5Helper::PairOfDatasets(name, dataset));
                    std::cout << "----> 3D type dataset: " << name << "; size: " << size << std::endl << std::endl;
                }
                // Downsampled 3D type
                else if (dataset->hasAttribute("dwnsmpl")
                         && dataset->getDataTypeClass() == H5T_FLOAT
                         && size.z() < nDims.z()
                         && size.y() < nDims.y()
                         && size.x() < nDims.x()) {
                    dts3DTypeDsp.insert(HDF5Helper::PairOfDatasets(name, dataset));
                    std::cout << "----> 3D type downsampled dataset: " << name << "; size: " << size << std::endl << std::endl;
                }
                // Sensor mask type
                else if (dataset->getDataTypeClass() == H5T_FLOAT
                         && sensorMaskIndexDataset != NULL
                         && size.z() == 1
                         && size.y() <= nDims.w()
                         && size.x() == sensorMaskSize) {
                    dtsMaskType.insert(HDF5Helper::PairOfDatasets(name, dataset));
                    std::cout << "----> Mask type dataset: " << name << "; size: " << size << std::endl << std::endl;
                }
                // Unknown type
                else {
                    hDF5SimOutputFile->closeDataset(name);
                }
            }
            // Unknown type
            else {
                hDF5SimOutputFile->closeDataset(name);
            }
        }
        // Groups
        else if (type == H5G_GROUP) {
            HDF5Helper::HDF5Group *group = hDF5SimOutputFile->openGroup(i);
            hsize_t count = group->getNumObjs();
            // Nonempty group
            if (count > 0) {
                HDF5Helper::HDF5Dataset *dataset = group->openDataset(0);
                // 4D dataset
                if (dataset->getRank() == 4) {
                    HDF5Helper::HDF5Vector4D dims = dataset->getDims();
                    if (dims.w() <= nDims.w() &&
                        dims.z() <= nDims.z() &&
                        dims.y() <= nDims.y() &&
                        dims.x() <= nDims.x()
                        ) {
                        // Downsampled cuboids with attributes
                        if (dataset->hasAttribute("positionX") &&
                            dataset->hasAttribute("positionY") &&
                            dataset->hasAttribute("positionZ") &&
                            group->hasAttribute("dwnsmpl")
                            ) {
                            groupsCuboidTypeAttrDsp.insert(HDF5Helper::PairOfGroups(name, group));
                            std::cout << "----> Cuboid attr dwnsml type dataset: " << name << "; size: " << dims << std::endl << std::endl;
                        }
                        // Cuboids with attributes
                        else if (dataset->hasAttribute("positionX") &&
                                 dataset->hasAttribute("positionY") &&
                                 dataset->hasAttribute("positionZ")
                                 ) {
                            groupsCuboidTypeAttr.insert(HDF5Helper::PairOfGroups(name, group));
                            std::cout << "----> Cuboid attr type dataset: " << name << "; size: " << dims << std::endl << std::endl;
                        }
                        // Original cuboids without attributes
                        else {
                            groupsCuboidType.insert(HDF5Helper::PairOfGroups(name, group));
                            std::cout << "----> Cuboid type dataset: " << name << "; size: " << dims << std::endl << std::endl;
                        }
                        group->closeDataset(0);
                    } else {
                        group->closeDataset(0);
                        hDF5SimOutputFile->closeGroup(name);
                    }
                } else {
                    group->closeDataset(0);
                    hDF5SimOutputFile->closeGroup(name);
                }
            }
            // Unknown type - empty group
            else {
                hDF5SimOutputFile->closeGroup(name);
            }
        }
    }
}

