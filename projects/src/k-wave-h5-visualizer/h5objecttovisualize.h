/**
 * @file        h5objecttovisualize.h
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        30 July      2014 (created)
 *              6  December  2015 (updated)
 *              8  September 2016 (updated)
 *
 * @brief       The header file with OpenedH5File::H5ObjectToVisualize class declaration.
 *
 *
 * @license     This file is part of k-Wave visualiser application
 * for visualizing HDF5 data created by the k-Wave toolbox - http://www.k-wave.org.
 *
 * @copyright   Copyright © 2016, Petr Kleparnik, VUT FIT Brno. All Rights Reserved.
 *
 * k-Wave visualiser is free software.
 */

#ifndef H5OBJECTTOVISUALIZE_H
#define H5OBJECTTOVISUALIZE_H

#include "openedh5file.h"
#include "h5subobjecttovisualize.h"
#include "hdf5readingthread.h"

class OpenedH5File::H5ObjectToVisualize : public QObject
{
    Q_OBJECT
public:
    explicit H5ObjectToVisualize(QString name, ObjectType type, OpenedH5File *openedH5File, QObject *parent = 0);
    ~H5ObjectToVisualize();

    void addSubobject(HDF5Helper::HDF5Dataset *dataset);
    bool setSelectedSubobject(QString name);
    OpenedH5File::H5SubobjectToVisualize *getSelectedSubobject();
    QString getName();
    ObjectType getType();
    QList<QString> getSubobjectNames();
    QMap<QString, OpenedH5File::H5SubobjectToVisualize *> getSubobjects();

signals:

public slots:
    void toogleSelected();
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
