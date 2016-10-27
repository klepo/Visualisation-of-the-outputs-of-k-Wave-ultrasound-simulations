/**
 * @file        openedh5file.cpp
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.0
 * @date        30 July      2014 (created)
 *              6  December  2015 (updated)
 *              8  September 2016 (updated)
 *
 * @brief       The implementation file containing the OpenedH5File class.
 *              Object of class H5ObjectToVisualize contains class H5SubobjectToVisualize and H5ObjectToVisualize.
 *              This class is for encapsulation of HDF5 file properties and datasets or groups with data to visualize.
 *

 * @license     This file is partof k-Wave visualiser application
 * for visualizing HDF5 data created by the k-Wave toolbox - http://www.k-wave.org.
 *
 * @copyright   Copyright © 2016, Petr Kleparnik, VUT FIT Brno. All Rights Reserved.
 *
 * k-Wave visualiser is free software.
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
    HDF5Helper::HDF5Group *group = file->openGroup("/");
    for (hsize_t i = 0; i < group->getNumAttrs(); i++) {
        HDF5Helper::HDF5Attribute *attribute = group->getAttribute(i);
        QString value(static_cast<const char *>(attribute->getData()));
        info.insert(QString::fromStdString(attribute->getName()), value);
    }
    file->closeGroup(group);

    qRegisterMetaType<OpenedH5File::H5ObjectToVisualize *>("OpenedH5File::H5ObjectToVisualize");

    qDebug() << "Find datasets for visualization...";

    // Find datasets for visualization
    for (hsize_t i = 0; i < file->getNumObjs(); i++) {
        H5G_obj_t type = file->getObjTypeByIdx(i);
        if (type == H5G_DATASET) {
            try {
                HDF5Helper::HDF5Dataset *dataset = file->openDataset(i);
                HDF5Helper::HDF5Vector3D size = dataset->getDims();
                // 3D type
                if (dataset->getRank() == 3 && dataset->getDataTypeClass() == H5T_FLOAT) {
                    if (size == HDF5Helper::HDF5Vector3D(nDims)) {
                        QString name = QString::fromStdString(dataset->getName());
                        qDebug() << "----> 3D type dataset: " << name << "; size: " << QString::fromStdString(size);
                        setObject(name, dataset);
                    }
                    // Downsampled 3D type
                    else if (dataset->hasAttribute("dwnsmpl") && dataset->hasAttribute("src_dataset_name")) {
                        QString name = QString::fromStdString(dataset->readAttributeS("src_dataset_name"));
                        qDebug() << "----> 3D type dataset (downsampled): " << name << "; size: " << QString::fromStdString(size);
                        setObject(name, dataset);
                    }
                    // Something other
                    else {
                        file->closeDataset(dataset);
                    }
                }
                // Something other
                else {
                    file->closeDataset(dataset);
                }
            } catch(std::exception &e) {
                std::cerr << e.what() << std::endl;
            }
        } else if (type == H5G_GROUP) {
            // Reshaped mask type group
            try {
                HDF5Helper::HDF5Group *group = file->openGroup(i);
                HDF5Helper::HDF5Dataset *dataset = group->openDataset(0);
                // 4D type
                if (dataset->getRank() == 4 && dataset->getDataTypeClass() == H5T_FLOAT) {
                    // Downsampled reshaped mask type group
                    if (group->hasAttribute("dwnsmpl") && group->hasAttribute("src_group_name")) {
                        QString name = QString::fromStdString(group->readAttributeS("src_group_name"));
                        qDebug() << "----> Reshaped mask type group (downsampled): "<< name;
                        setObject(name, group);
                    }
                    // Original reshaped mask type group
                    else {
                        QString name = QString::fromStdString(group->getName());
                        qDebug() << "----> Reshaped mask type group: "<< name;
                        setObject(name, group);
                    }
                }
                // Something other
                else {
                    file->closeGroup(group);
                }
            } catch(std::exception &e) {
                std::cerr << e.what() << std::endl;
            }
        }
    }
}

void OpenedH5File::setObject(QString name, HDF5Helper::HDF5Dataset *dataset)
{
    if (!objects.contains(name)) {
        objects.insert(name, new OpenedH5File::H5ObjectToVisualize(name, H5G_DATASET, this));
        objects[name]->addSubobject(dataset);
    } else {
        objects[name]->addSubobject(dataset);
    }
}

void OpenedH5File::setObject(QString name, HDF5Helper::HDF5Group *group)
{
    if (!objects.contains(name)) {
        objects.insert(name, new H5ObjectToVisualize(name, H5G_GROUP, this));
        objects[name]->addSubobject(group);
    } else {
        objects[name]->addSubobject(group);
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
 * @return object or NULL
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
 * @return subobject or NULL
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

HDF5Helper::HDF5Vector4D OpenedH5File::getNDims() const
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
uint64_t OpenedH5File::getNT() const
{
    return nDims.w();
}

/**
 * @brief OpenedH5File::getNX
 * @return Nx dimension
 */
uint64_t OpenedH5File::getNX() const
{
    return nDims.x();
}

/**
 * @brief OpenedH5File::getNY
 * @return Ny dimension
 */
uint64_t OpenedH5File::getNY() const
{
    return nDims.y();
}

/**
 * @brief OpenedH5File::getNZ
 * @return Nz dimension
 */
uint64_t OpenedH5File::getNZ() const
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
