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
 * @brief OpenedH5File::OpenedH5File
 * @param fileName path to HDF5 file
 * @param parent
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

void OpenedH5File::setObject(QString nameTmp, HDF5Helper::Dataset *dataset, ObjectType type)
{
    QString name = nameTmp;
    if (name.at(0) == '/')
        name.remove(0, 1);
    if (!objects.contains(name)) {
        objects.insert(name, new OpenedH5File::H5ObjectToVisualize(name, type, this));
        objects[name]->addSubobject(dataset);
    } else {
        objects[name]->addSubobject(dataset);
    }
}

/**
 * @brief OpenedH5File::getObjects
 * @return objects
 */
QMap<QString, OpenedH5File::H5ObjectToVisualize *> OpenedH5File::getObjects()
{
    return objects;
}

/**
 * @brief OpenedH5File::getObject
 * @param mainName name of object
 * @return object or 0
 */
OpenedH5File::H5ObjectToVisualize *OpenedH5File::getObject(QString mainName)
{
    if (objects.contains(mainName)) {
        return objects[mainName];
        //if (objects[name]->getNames().contains(name))
    } else
        return 0;
}

/**
 * @brief OpenedH5File::getObjectBySubobjectName
 * @param name name of subobject (dataset or group)
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
 * @brief OpenedH5File::setSelectedSubobject Set subobject selected. From group
 * of subobjects can be one selected.
 * @param name name of subobject
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
 * @brief OpenedH5File::setObjectSelected Set object selected/unselected
 * @param mainName name of object
 * @param value true/false
 */
void OpenedH5File::setObjectSelected(QString mainName, bool value)
{
    if (objects.contains(mainName)) {
        objects[mainName]->setSelected(value);
    }
}

/**
 * @brief OpenedH5File::toogleObjectSelected
 * @param mainName name of object
 */
void OpenedH5File::toogleObjectSelected(QString mainName)
{
    if (objects.contains(mainName)) {
        objects[mainName]->toogleSelected();
    }
}

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

HDF5Helper::Vector4D OpenedH5File::getNDims() const
{
    return nDims;
}

/**
 * @brief OpenedH5File::~OpenedH5File
 */
OpenedH5File::~OpenedH5File()
{
    foreach (QString key, objects.keys())
        delete objects[key];
    delete file;
}

/**
 * @brief OpenedH5File::getNT
 * @return number of time steps (Nt)
 */
hsize_t OpenedH5File::getNT() const
{
    return nDims.w();
}

/**
 * @brief OpenedH5File::getNX
 * @return Nx dimension
 */
hsize_t OpenedH5File::getNX() const
{
    return nDims.x();
}

/**
 * @brief OpenedH5File::getNY
 * @return Ny dimension
 */
hsize_t OpenedH5File::getNY() const
{
    return nDims.y();
}

/**
 * @brief OpenedH5File::getNZ
 * @return Nz dimension
 */
hsize_t OpenedH5File::getNZ() const
{
    return nDims.z();
}

/**
 * @brief OpenedH5File::getInfo Get simulation info form HDF5 file
 * @return info data structure
 */
QMap<QString, QString> OpenedH5File::getInfo()
{
    return info;
}

/**
 * @brief OpenedH5File::getFile Get HDF5 file
 * @return HDF5 file
 */
HDF5Helper::File *OpenedH5File::getFile()
{
    return file;
}

/**
 * @brief OpenedH5File::getFilename Get filename
 * @return filename
 */
QString OpenedH5File::getFilename() const
{
    return QString::fromStdString(file->getFilename());
}

/**
 * @brief OpenedH5File::getRawFilename Get filename without extension
 * @return filename without extension
 */
QString OpenedH5File::getRawFilename() const
{
    std::string fileName = file->getFilename();
    size_t lastindex = fileName.find_last_of(".");
    std::string rawname = fileName.substr(0, lastindex);
    return QString::fromStdString(rawname);
}
