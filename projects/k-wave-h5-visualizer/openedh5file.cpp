/**
 * @file        openedh5file.cpp
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        30 July      2014 (created) \n
 *              27 February  2017 (updated)
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
 *              license. A copy of the LGPL license should have been recieved with this file.
 *              Otherwise, it can be found at: http://www.gnu.org/copyleft/lesser.html.
 *
 * @copyright   Copyright © 2017, Petr Kleparnik, VUT FIT Brno. All Rights Reserved.
 *
 */

#include "openedh5file.h"
#include "h5objecttovisualize.h"
#include "h5subobjecttovisualize.h"

/**
 * @brief Creates OpenedH5File obejct
 * @param[in] fileName Path to HDF5 file
 * @param[in] parent Parent (optional)
 */
OpenedH5File::OpenedH5File(QString fileName, QObject *parent) :
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
    }

    qRegisterMetaType<OpenedH5File::H5ObjectToVisualize *>("OpenedH5File::H5ObjectToVisualize");

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
OpenedH5File::~OpenedH5File()
{
    foreach (QString key, objects.keys())
        delete objects[key];
    delete file;
}

/**
 * @brief Sets object
 * @param[in] name Object name
 * @param[in] dataset Dataset
 * @param[in] type Obejct type
 */
void OpenedH5File::setObject(QString name, HDF5Helper::Dataset *dataset, ObjectType type)
{
    QString nameTmp = name;
    if (nameTmp.at(0) == '/')
        nameTmp.remove(0, 1);
    if (!objects.contains(nameTmp)) {
        objects.insert(nameTmp, new OpenedH5File::H5ObjectToVisualize(nameTmp, type, this));
        objects[nameTmp]->addSubobject(dataset);
    } else {
        objects[nameTmp]->addSubobject(dataset);
    }
}

/**
 * @brief Returns objects
 * @return objects
 */
QMap<QString, OpenedH5File::H5ObjectToVisualize *> OpenedH5File::getObjects()
{
    return objects;
}

/**
 * @brief Returns object by name
 * @param[in] name Name of object
 * @return object or 0
 */
OpenedH5File::H5ObjectToVisualize *OpenedH5File::getObject(QString name)
{
    if (objects.contains(name)) {
        return objects[name];
        //if (objects[name]->getNames().contains(name))
    } else
        return 0;
}

/**
 * @brief Returns object by subobject name
 * @param[in] name Name of subobject (dataset or group)
 * @return subobject or 0
 */
OpenedH5File::H5ObjectToVisualize *OpenedH5File::getObjectBySubobjectName(QString name)
{
    foreach (QString key, objects.keys()) {
        if (objects[key]->getSubobjectNames().contains(name)) {
            objects[key]->setSelectedSubobject(name);
            return objects[key];
        }
    }
    return 0;
}

/**
 * @brief Sets subobject as selected
 * @param[in] name Name of subobject
 *
 * From group of subobjects one can be selected.
 */
void OpenedH5File::setSelectedSubobject(QString name)
{
    foreach (QString key, objects.keys()) {
        if (objects[key]->getSubobjectNames().contains(name)) {
            objects[key]->setSelectedSubobject(name);
        }
    }
}

/**
 * @brief Sets object selected/unselected
 * @param[in] name Name of object
 * @param[in] value True/False
 */
void OpenedH5File::setObjectSelected(QString name, bool value)
{
    if (objects.contains(name)) {
        objects[name]->setSelected(value);
    }
}

/**
 * @brief Toogles object selected
 * @param[in] name Name of object
 */
void OpenedH5File::toogleObjectSelected(QString name)
{
    if (objects.contains(name)) {
        objects[name]->toogleSelected();
    }
}

/**
 * @brief Finds datasets for visualization
 * @param[in] group Group to search
 */
void OpenedH5File::findDatasetsForVisualization(HDF5Helper::Group *group)
{
    for (hsize_t i = 0; i < group->getNumObjs(); i++) {
        H5G_obj_t type = group->getObjTypeByIdx(i);
        std::string name = group->getObjNameByIdx(i);

        // Datasets
        if (type == H5G_DATASET) {
            HDF5Helper::Dataset *dataset = group->openDataset(i);
            HDF5Helper::DatasetType datasetType = dataset->getType();

            if (datasetType == HDF5Helper::DatasetType::BASIC_3D) {
                setObject(QString::fromStdString(dataset->getName()), dataset, dataset3D_t);
                std::cout << "----> " << dataset->getTypeString(datasetType) << ": " << name << ", size: " << dataset->getDims() << std::endl;
            } else if (datasetType == HDF5Helper::DatasetType::DWNSMPL_3D) {
                setObject(QString::fromStdString(dataset->readAttributeS(HDF5Helper::SRC_DATASET_NAME_ATTR, false)), dataset, dataset3D_t);
                std::cout << "----> " << dataset->getTypeString(datasetType) << ": " << name << ", size: " << dataset->getDims() << std::endl;
            } else if (datasetType == HDF5Helper::DatasetType::MASK_3D) {
                setObject(QString::fromStdString(dataset->getName()), dataset, dataset3D_t);
                std::cout << "----> " << dataset->getTypeString(datasetType) << ": " << name << ", size: " << dataset->getDims() << std::endl;
            } else if (datasetType == HDF5Helper::DatasetType::CUBOID
                       || datasetType == HDF5Helper::DatasetType::CUBOID_ATTR
                       ) {
                setObject(QString::fromStdString(group->getName()), dataset, dataset4D_t);
                std::cout << "----> " << dataset->getTypeString(datasetType) << ": " << name << ", size: " << dataset->getDims() << std::endl;
            } else if (datasetType == HDF5Helper::DatasetType::CUBOID_FI
                       || datasetType == HDF5Helper::DatasetType::CUBOID_K
                       || datasetType == HDF5Helper::DatasetType::CUBOID_D
                       || datasetType == HDF5Helper::DatasetType::CUBOID_S
                       || datasetType == HDF5Helper::DatasetType::CUBOID_DWNSMPL
                       || datasetType == HDF5Helper::DatasetType::CUBOID_DWNSMPL_FI
                       || datasetType == HDF5Helper::DatasetType::CUBOID_DWNSMPL_K
                       || datasetType == HDF5Helper::DatasetType::CUBOID_DWNSMPL_D
                       || datasetType == HDF5Helper::DatasetType::CUBOID_DWNSMPL_S
                       || datasetType == HDF5Helper::DatasetType::CUBOID_ATTR_FI
                       || datasetType == HDF5Helper::DatasetType::CUBOID_ATTR_K
                       || datasetType == HDF5Helper::DatasetType::CUBOID_ATTR_D
                       || datasetType == HDF5Helper::DatasetType::CUBOID_ATTR_S
                       || datasetType == HDF5Helper::DatasetType::CUBOID_ATTR_DWNSMPL
                       || datasetType == HDF5Helper::DatasetType::CUBOID_ATTR_DWNSMPL_FI
                       || datasetType == HDF5Helper::DatasetType::CUBOID_ATTR_DWNSMPL_K
                       || datasetType == HDF5Helper::DatasetType::CUBOID_ATTR_DWNSMPL_D
                       || datasetType == HDF5Helper::DatasetType::CUBOID_ATTR_DWNSMPL_S
                       ) {
                setObject(QString::fromStdString(dataset->readAttributeS(HDF5Helper::SRC_DATASET_NAME_ATTR, false)), dataset, dataset4D_t);
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
HDF5Helper::Vector4D OpenedH5File::getNDims() const
{
    return nDims;
}

/**
 * @brief Returns Nt
 * @return number of time steps (Nt)
 */
hsize_t OpenedH5File::getNT() const
{
    return nDims.w();
}

/**
 * @brief Returns Nx
 * @return Nx dimension
 */
hsize_t OpenedH5File::getNX() const
{
    return nDims.x();
}

/**
 * @brief Returns Ny
 * @return Ny dimension
 */
hsize_t OpenedH5File::getNY() const
{
    return nDims.y();
}

/**
 * @brief Returns Nz
 * @return Nz dimension
 */
hsize_t OpenedH5File::getNZ() const
{
    return nDims.z();
}

/**
 * @brief Returns simulation info from HDF5 file
 * @return Info data structure
 */
QMap<QString, QString> OpenedH5File::getInfo()
{
    return info;
}

/**
 * @brief Returns HDF5 file
 * @return HDF5 file
 */
HDF5Helper::File *OpenedH5File::getFile()
{
    return file;
}

/**
 * @brief Returns filename
 * @return Filename
 */
QString OpenedH5File::getFilename() const
{
    return QString::fromStdString(file->getFilename());
}

/**
 * @brief Returns filename without extension
 * @return Filename without extension
 */
QString OpenedH5File::getRawFilename() const
{
    std::string fileName = file->getFilename();
    size_t lastindex = fileName.find_last_of(".");
    std::string rawname = fileName.substr(0, lastindex);
    return QString::fromStdString(rawname);
}
