/**
 * @file        h5objecttovisualize.cpp
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        30 July      2014 (created) \n
 *              27 February  2017 (updated)
 *
 * @brief       The implementation file containing OpenedH5File::H5ObjectToVisualize
 *              class definition.
 *
 * Object of class H5ObjectToVisualize can contains many objects of class
 * H5SubobjectToVisualize. This hierarchy is for many resolutions of datasets
 * and for keeping current setting of seleceted dataset.
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

#include "h5objecttovisualize.h"

/**
 * @brief OpenedH5File::H5ObjectToVisualize::H5ObjectToVisualize Creation of H5ObjectToVisualize
 * @param name name of object
 * @param type type of object (GROUP_TYPE or DATASET_TYPE)
 * @param openedH5File hdf5 file
 * @param parent
 */
OpenedH5File::H5ObjectToVisualize::H5ObjectToVisualize(QString name, ObjectType type, OpenedH5File *openedH5File, QObject *parent) : QObject(parent)
{
    // Params
    this->openedH5File = openedH5File;
    this->name = name;
    this->type = type;
}

/**
 * @brief OpenedH5File::H5ObjectToVisualize::~H5ObjectToVisualize
 */
OpenedH5File::H5ObjectToVisualize::~H5ObjectToVisualize()
{
    foreach (QString key, subobjects.keys())
        delete subobjects.value(key);
}

/**
 * @brief OpenedH5File::H5ObjectToVisualize::addSubobject Add dataset to Qmap of subobjects
 * @param dataset HDF5dataset * object
 */
void OpenedH5File::H5ObjectToVisualize::addSubobject(HDF5Helper::Dataset *dataset)
{
    QString name = QString::fromStdString(dataset->getOnlyName());
    if (!subobjects.contains(name)) {
        H5SubobjectToVisualize *subobject = new H5SubobjectToVisualize(dataset, type, openedH5File, this);
        subobjects.insert(name, subobject);
        selectedSubobject = subobject;
    }
}


/**
 * @brief OpenedH5File::H5ObjectToVisualize::getName
 * @return name of object to visualize
 */
QString OpenedH5File::H5ObjectToVisualize::getName()
{
    return name;
}

/**
 * @brief OpenedH5File::H5ObjectToVisualize::getType
 * @return GROUP_TYPE or DATASET_TYPE
 */
OpenedH5File::ObjectType OpenedH5File::H5ObjectToVisualize::getType()
{
    return type;
}

/**
 * @brief OpenedH5File::H5ObjectToVisualize::getSubobjectNames
 * @return QList of subobjects names
 */
QList<QString> OpenedH5File::H5ObjectToVisualize::getSubobjectNames()
{
    return subobjects.keys();
}

/**
 * @brief OpenedH5File::H5ObjectToVisualize::getSubobjects
 * @return QMap<QString, OpenedH5File::H5SubobjectToVisualize *> subobjects;
 */
QMap<QString, OpenedH5File::H5SubobjectToVisualize *> OpenedH5File::H5ObjectToVisualize::getSubobjects()
{
    return subobjects;
}

/**
 * @brief OpenedH5File::H5ObjectToVisualize::getSelectedSubobject
 * @return selected subobject
 */
OpenedH5File::H5SubobjectToVisualize *OpenedH5File::H5ObjectToVisualize::getSelectedSubobject()
{
    return selectedSubobject;
}

/**
 * @brief OpenedH5File::H5ObjectToVisualize::setSelectedSubobject
 * @param name name of subobject
 */
bool OpenedH5File::H5ObjectToVisualize::setSelectedSubobject(QString name)
{
    if (subobjects.contains(name)) {
        selectedSubobject = subobjects[name];
        return true;
    }
    return false;
}

/**
 * @brief OpenedH5File::H5ObjectToVisualize::toogleSelected
 */
void OpenedH5File::H5ObjectToVisualize::toogleSelected()
{
    selected = !selected;
}

/**
 * @brief OpenedH5File::H5ObjectToVisualize::setSelected Set object as selected
 * @param value
 */
void OpenedH5File::H5ObjectToVisualize::setSelected(bool value)
{
    selected = value;
}

