/**
 * @file        h5openedfile.cpp
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        30 July      2014 (created) <br>
 *              30 October   2018 (updated)
 *
 * @brief       The implementation file containing OpenedH5File class definition.
 *
 * Object of class H5ObjectToVisualize contains class H5SubobjectToVisualize
 * and H5ObjectToVisualize. This class is for encapsulation of HDF5 file properties
 * and datasets or groups with data to visualize.
 *
 * @license     This file is part of the k-Wave-h5-visualizer tool for processing the HDF5 data
 *              created by the k-Wave toolbox - http://www.k-wave.org. This file may be used,
 *              distributed and modified under the terms of the LGPL version 3 open source
 *              license. A copy of the LGPL license should have been received with this file.
 *              Otherwise, it can be found at: http://www.gnu.org/copyleft/lesser.html.
 *
 * @copyright   Copyright © 2018, Petr Kleparnik, VUT FIT Brno. All Rights Reserved.
 *
 */

#include "h5openedfile.h"
#include "h5objecttovisualize.h"

/**
 * @brief Creates OpenedH5File object
 * @param[in] filename Path to HDF5 file
 * @param[in] parent Parent (optional)
 */
H5OpenedFile::H5OpenedFile(QString filename, QObject *parent) :
    QObject(parent)
{
    // Open HDF5 file, operations with file can throw exceptions
    file = new H5Helper::File(filename.toStdString(), H5Helper::File::OPEN);

    // Load dimensions
    nDims = file->getNDims();

    qDebug() << "Load info (root attributes)...";

    // Load info
    H5Helper::Group *group = file->openGroup("/", false);
    for (hsize_t i = 0; i < group->getNumAttrs(); i++) {
        H5Helper::Attribute *attribute = group->getAttribute(i);
        QString value = QString::fromStdString(attribute->getStringValue());
        info.insert(QString::fromStdString(attribute->getName()), value);
        delete attribute;
        attribute = nullptr;
    }

    qRegisterMetaType<H5ObjectToVisualize *>("H5ObjectToVisualize");

    qDebug() << "Find datasets for visualization...";

    // Find datasets for visualization
    findDatasetsForVisualization(group);
    file->closeGroup(group);
}

/**
 * @brief Destructor of OpenedH5File
 *
 * Deletes objects and file.
 */
H5OpenedFile::~H5OpenedFile()
{
    for (int i = 0; i < objects.size(); i++) {
        delete objects[i];
        objects[i] = nullptr;
    }
    objects.clear();
    delete file;
    file = nullptr;
}

/**
 * @brief Returns objects
 * @return objects
 */
QVector<H5ObjectToVisualize *> H5OpenedFile::getObjects() const
{
    return objects;
}

/**
 * @brief Returns objects with selection flag
 * @return objects with selection flag
 */
QVector<H5ObjectToVisualize *> H5OpenedFile::getObjectsSelected() const
{
    QVector<H5ObjectToVisualize *> selectedObjects;
    foreach (H5ObjectToVisualize *object, objects) {
        if (object->isSelected()) {
            selectedObjects.push_back(object);
        }
    }
    return selectedObjects;
}

/**
 * @brief Returns nDims
 * @return nDims
 */
H5Helper::Vector4D H5OpenedFile::getNDims() const
{
    return nDims;
}

/**
 * @brief Returns simulation info from HDF5 file
 * @return Info data structure
 */
QMap<QString, QString> H5OpenedFile::getInfo() const
{
    return info;
}

/**
 * @brief Returns HDF5 file
 * @return HDF5 file
 */
const H5Helper::File *H5OpenedFile::getFile() const
{
    return file;
}

/**
 * @brief Returns filename
 * @return Filename
 */
QString H5OpenedFile::getFilename() const
{
    return QString::fromStdString(file->getFilename());
}

/**
 * @brief Returns filename without extension
 * @return Filename without extension
 */
QString H5OpenedFile::getRawFilename() const
{
    std::string filename = file->getFilename();
    size_t lastindex = filename.find_last_of(".");
    std::string rawname = filename.substr(0, lastindex);
    return QString::fromStdString(rawname);
}

/**
 * @brief Finds datasets for visualization
 * @param[in] group Group to search
 */
void H5OpenedFile::findDatasetsForVisualization(const H5Helper::Group *group)
{
    for (hsize_t i = 0; i < group->getNumObjs(); i++) {
        H5G_obj_t type = group->getObjTypeByIdx(i);
        std::string name = group->getObjNameByIdx(i);

        // Datasets
        if (type == H5G_DATASET) {
            H5Helper::Dataset *dataset = group->openDataset(i);
            H5Helper::DatasetType datasetType = dataset->getType();

            if (datasetType == H5Helper::DatasetType::BASIC_3D
                    || datasetType == H5Helper::DatasetType::RESHAPED_3D
                    || datasetType == H5Helper::DatasetType::BASIC_3D_DWNSMPL
                    || datasetType == H5Helper::DatasetType::RESHAPED_3D_DWNSMPL
                    ) {
                setObject(dataset, DATASET_3D);
                std::cout << "----> " << dataset->getTypeString(datasetType) << ": " << name << ", size: " << dataset->getDims() << std::endl;
            } else if (datasetType == H5Helper::DatasetType::CUBOID
                       || datasetType == H5Helper::DatasetType::CUBOID_C
                       || datasetType == H5Helper::DatasetType::CUBOID_D
                       || datasetType == H5Helper::DatasetType::CUBOID_S
                       || datasetType == H5Helper::DatasetType::CUBOID_DWNSMPL
                       || datasetType == H5Helper::DatasetType::CUBOID_DWNSMPL_C
                       || datasetType == H5Helper::DatasetType::CUBOID_DWNSMPL_D
                       || datasetType == H5Helper::DatasetType::CUBOID_DWNSMPL_S
                       || datasetType == H5Helper::DatasetType::CUBOID_ATTR
                       || datasetType == H5Helper::DatasetType::CUBOID_ATTR_C
                       || datasetType == H5Helper::DatasetType::CUBOID_ATTR_D
                       || datasetType == H5Helper::DatasetType::CUBOID_ATTR_S
                       || datasetType == H5Helper::DatasetType::CUBOID_ATTR_DWNSMPL
                       || datasetType == H5Helper::DatasetType::CUBOID_ATTR_DWNSMPL_C
                       || datasetType == H5Helper::DatasetType::CUBOID_ATTR_DWNSMPL_D
                       || datasetType == H5Helper::DatasetType::CUBOID_ATTR_DWNSMPL_S
                       ) {
                setObject(dataset, DATASET_4D);
                std::cout << "----> " << dataset->getTypeString(datasetType) << ": " << name << ", size: " << dataset->getDims() << std::endl;
            } else { // Unknown type
                group->closeDataset(dataset);
            }
        }
        // Groups
        if (type == H5G_GROUP) {
            H5Helper::Group *nextGroup = group->openGroup(i);
            findDatasetsForVisualization(nextGroup);
            group->closeGroup(nextGroup);
        }
    }
}

/**
 * @brief Sets object
 * @param[in] dataset Dataset
 * @param[in] type Object type
 */
void H5OpenedFile::setObject(H5Helper::Dataset *dataset, ObjectType type)
{
    bool objectExists = false;
    foreach (H5ObjectToVisualize *object, objects) {
        if (object->getName() == QString::fromStdString(dataset->getName())) {
            objectExists = true;
            break;
        }
    }

    if (!objectExists)
        objects.push_back(new H5ObjectToVisualize(dataset, type, this));
}
