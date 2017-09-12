/**
 * @file        h5objecttovisualize.h
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        30 July      2014 (created) \n
 *              11 September 2017 (updated)
 *
 * @brief       The header file with OpenedH5File::H5ObjectToVisualize class declaration.
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

#ifndef H5OBJECTTOVISUALIZE_H
#define H5OBJECTTOVISUALIZE_H

#include "openedh5file.h"
#include "h5subobjecttovisualize.h"
#include "hdf5readingthread.h"

/**
 * @brief The H5ObjectToVisualize class represents wrapper
 * for objects for visualization
 */
class OpenedH5File::H5ObjectToVisualize : public QObject
{
    Q_OBJECT
public:
    explicit H5ObjectToVisualize(QString name, ObjectType type, OpenedH5File *openedH5File, QObject *parent = 0);
    ~H5ObjectToVisualize();

    void addSubobject(HDF5Helper::Dataset *dataset);
    bool setSelectedSubobject(QString name);
    OpenedH5File::H5SubobjectToVisualize *getSelectedSubobject();
    QString getName();
    ObjectType getType();
    QList<QString> getSubobjectNames();
    QMap<QString, OpenedH5File::H5SubobjectToVisualize *> getSubobjects();

signals:

public slots:
    void toggleSelected();
    void setSelected(bool value);

private:
    // One subobject is selected
    OpenedH5File::H5SubobjectToVisualize *selectedSubobject = 0;
    QMap<QString, OpenedH5File::H5SubobjectToVisualize *> subobjects;
    ObjectType type;
    OpenedH5File *openedH5File;
    QString name;
    // This is for future implementation of multiple selection of datasets
    bool selected = false;
};

#endif // H5OBJECTTOVISUALIZE_H
