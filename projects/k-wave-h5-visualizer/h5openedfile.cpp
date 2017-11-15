/**
 * @file        openedh5file.cpp
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        30 July      2014 (created) \n
 *              11 September 2017 (updated)
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
 * @copyright   Copyright © 2017, Petr Kleparnik, VUT FIT Brno. All Rights Reserved.
 *
 */

#include "h5openedfile.h"
#include "h5objecttovisualize.h"
#include "h5objecttovisualize.h"

/**
 * @brief Creates OpenedH5File object
 * @param[in] fileName Path to HDF5 file
 * @param[in] parent Parent (optional)
 */
H5OpenedFile::H5OpenedFile(QString fileName, QObject *parent) :
    QObject(parent)
{
    // Open HDF5 file, operations with file can throw exceptions
    file = new HDF5Helper::File(fileName.toStdString(), HDF5Helper::File::OPEN);

    // Load dimensions
    nDims = file->getNdims();

    qDebug() << "Load info (root attributes)...";

    // Load info
    HDF5Helper::Group *group = file->openGroup("/", false);
    for (hsize_t i = 0; i < group->getNumAttrs(); i++) {
        HDF5Helper::Attribute *attribute = group->getAttribute(i);
        QString value(static_cast<const char *>(attribute->getData()));
        info.insert(QString::fromStdString(attribute->getName()), value);
        delete attribute;
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
    }
    delete file;
}

/**
 * @brief Sets object
 * @param[in] dataset Dataset
 * @param[in] type Object type
 */
void H5OpenedFile::setObject(HDF5Helper::Dataset *dataset, ObjectType type)
{
    bool objectExists = false;
    foreach (H5ObjectToVisualize *object, objects) {
        if (object->getDataset()->getName() == dataset->getName()) {
            objectExists = true;
            break;
        }
    }

    if (!objectExists)
        objects.push_back(new H5ObjectToVisualize(dataset, type, this));
}

/**
 * @brief Returns objects
 * @return objects
 */
QVector<H5ObjectToVisualize *> H5OpenedFile::getObjects()
{
    return objects;
}

QVector<H5ObjectToVisualize *> H5OpenedFile::getObjectsSelected()
{
    QVector<H5ObjectToVisualize *> selectedObjects;
    foreach (H5ObjectToVisualize *object, objects) {
        if (object->isSelected()) {
            selectedObjects.push_back(object);
        }
    }
    return selectedObjects;
}

H5ObjectToVisualize *H5OpenedFile::getObject(QString name)
{
    foreach (H5ObjectToVisualize *object, objects) {
        if (object->getName() == name) {
            return object;
        }
    }
    return 0;
}

/**
 * @brief Finds datasets for visualization
 * @param[in] group Group to search
 */
void H5OpenedFile::findDatasetsForVisualization(HDF5Helper::Group *group)
{
    for (hsize_t i = 0; i < group->getNumObjs(); i++) {
        H5G_obj_t type = group->getObjTypeByIdx(i);
        std::string name = group->getObjNameByIdx(i);

        // Datasets
        if (type == H5G_DATASET) {
            HDF5Helper::Dataset *dataset = group->openDataset(i);
            HDF5Helper::DatasetType datasetType = dataset->getType();

            if (datasetType == HDF5Helper::DatasetType::BASIC_3D
                    || datasetType == HDF5Helper::DatasetType::DWNSMPL_3D
                    || datasetType == HDF5Helper::DatasetType::MASK_3D
                    ) {
                setObject(dataset, DATASET_3D);
                std::cout << "----> " << dataset->getTypeString(datasetType) << ": " << name << ", size: " << dataset->getDims() << std::endl;
            } else if (datasetType == HDF5Helper::DatasetType::CUBOID
                       || datasetType == HDF5Helper::DatasetType::CUBOID_ATTR
                       || datasetType == HDF5Helper::DatasetType::CUBOID_C
                       || datasetType == HDF5Helper::DatasetType::CUBOID_D
                       || datasetType == HDF5Helper::DatasetType::CUBOID_S
                       || datasetType == HDF5Helper::DatasetType::CUBOID_DWNSMPL
                       || datasetType == HDF5Helper::DatasetType::CUBOID_DWNSMPL_C
                       || datasetType == HDF5Helper::DatasetType::CUBOID_DWNSMPL_D
                       || datasetType == HDF5Helper::DatasetType::CUBOID_DWNSMPL_S
                       || datasetType == HDF5Helper::DatasetType::CUBOID_ATTR_C
                       || datasetType == HDF5Helper::DatasetType::CUBOID_ATTR_D
                       || datasetType == HDF5Helper::DatasetType::CUBOID_ATTR_S
                       || datasetType == HDF5Helper::DatasetType::CUBOID_ATTR_DWNSMPL
                       || datasetType == HDF5Helper::DatasetType::CUBOID_ATTR_DWNSMPL_C
                       || datasetType == HDF5Helper::DatasetType::CUBOID_ATTR_DWNSMPL_D
                       || datasetType == HDF5Helper::DatasetType::CUBOID_ATTR_DWNSMPL_S
                       ) {
                setObject(dataset, DATASET_4D);
                std::cout << "----> " << dataset->getTypeString(datasetType) << ": " << name << ", size: " << dataset->getDims() << std::endl;
            } else { // Unknown type
                group->closeDataset(dataset);
            }
        }
        // Groups
        if (type == H5G_GROUP) {
            HDF5Helper::Group *nextGroup = group->openGroup(i);
            findDatasetsForVisualization(nextGroup);
            group->closeGroup(nextGroup);
        }
    }
}

/**
 * @brief Returns nDims
 * @return nDims
 */
HDF5Helper::Vector4D H5OpenedFile::getNDims() const
{
    return nDims;
}

/**
 * @brief Returns simulation info from HDF5 file
 * @return Info data structure
 */
QMap<QString, QString> H5OpenedFile::getInfo()
{
    return info;
}

/**
 * @brief Returns HDF5 file
 * @return HDF5 file
 */
HDF5Helper::File *H5OpenedFile::getFile()
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
    std::string fileName = file->getFilename();
    size_t lastindex = fileName.find_last_of(".");
    std::string rawname = fileName.substr(0, lastindex);
    return QString::fromStdString(rawname);
}
