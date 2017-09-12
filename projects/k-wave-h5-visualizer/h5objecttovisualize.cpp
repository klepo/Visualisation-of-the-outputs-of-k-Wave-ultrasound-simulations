/**
 * @file        h5objecttovisualize.cpp
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        30 July      2014 (created) \n
 *              11 September 2017 (updated)
 *
 * @brief       The implementation file containing OpenedH5File::H5ObjectToVisualize
 *              class definition.
 *
 * Object of class H5ObjectToVisualize can contains many objects of class
 * H5SubobjectToVisualize. This hierarchy is for many resolutions of datasets
 * and for keeping current setting of selected dataset.
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

#include "h5objecttovisualize.h"

/**
 * @brief Creates H5ObjectToVisualize object
 * @param[in] name Name of object
 * @param[in] type Type of object (GROUP_TYPE or DATASET_TYPE)
 * @param[in] openedH5File HDF5 File
 * @param[in] parent Parent (optional)
 */
OpenedH5File::H5ObjectToVisualize::H5ObjectToVisualize(QString name, ObjectType type, OpenedH5File *openedH5File, QObject *parent) : QObject(parent)
{
    // Params
    this->openedH5File = openedH5File;
    this->name = name;
    this->type = type;
}

/**
 * @brief Destructor of H5ObjectToVisualize object
 *
 * Deletes subobjects
 */
OpenedH5File::H5ObjectToVisualize::~H5ObjectToVisualize()
{
    foreach (QString key, subobjects.keys())
        delete subobjects.value(key);
}

/**
 * @brief Adds dataset to Qmap of subobjects
 * @param[in] dataset Dataset
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
 * @brief Returns name of object
 * @return Name of object
 */
QString OpenedH5File::H5ObjectToVisualize::getName()
{
    return name;
}

/**
 * @brief Returns type of object
 * @return GROUP_TYPE or DATASET_TYPE
 */
OpenedH5File::ObjectType OpenedH5File::H5ObjectToVisualize::getType()
{
    return type;
}

/**
 * @brief Returns subobject names
 * @return QList of subobjects names
 */
QList<QString> OpenedH5File::H5ObjectToVisualize::getSubobjectNames()
{
    return subobjects.keys();
}

/**
 * @brief Returns subobjects
 * @return QMap<QString, OpenedH5File::H5SubobjectToVisualize *> of subobjects;
 */
QMap<QString, OpenedH5File::H5SubobjectToVisualize *> OpenedH5File::H5ObjectToVisualize::getSubobjects()
{
    return subobjects;
}

/**
 * @brief Returns selected subobject
 * @return Selected subobject
 */
OpenedH5File::H5SubobjectToVisualize *OpenedH5File::H5ObjectToVisualize::getSelectedSubobject()
{
    return selectedSubobject;
}

/**
 * @brief Sets selected subobject
 * @param[in] name Subobject name
 * @return True if object exists
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
 * @brief Toggles selected
 */
void OpenedH5File::H5ObjectToVisualize::toggleSelected()
{
    selected = !selected;
}

/**
 * @brief Set object selected or not selected
 * @param[in] value True/False
 */
void OpenedH5File::H5ObjectToVisualize::setSelected(bool value)
{
    selected = value;
}

