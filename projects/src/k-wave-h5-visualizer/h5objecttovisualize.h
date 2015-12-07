/*
 * @file        h5objecttovisualize.h
 * @author      Petr Kleparnik, VUT FIT Brno, xklepa01@stud.fit.vutbr.cz
 * @version     0.0
 * @date        30 July      2014 (created)
 *              6  December  2015 (updated)
 *
 * @brief       The header file with H5ObjectToVisualize class declaration.
 *
 * @section     Licence
 * This file is part of k-Wave visualiser application
 * for visualizing HDF5 data created by the k-Wave toolbox - http://www.k-wave.org.
 * Copyright © 2014, Petr Kleparnik, VUT FIT Brno.
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
    explicit H5ObjectToVisualize(QString name, const int type, OpenedH5File *openedH5File, QObject *parent = 0);
    ~H5ObjectToVisualize();

    void addSubobject(HDF5Helper::File::HDF5Dataset *dataset);
    void addSubobject(HDF5Helper::File::HDF5Group *group);
    void setSelectedSubobject(QString name);
    OpenedH5File::H5SubobjectToVisualize *getSelectedSubobject();
    QString getName();
    int getType();
    QList<QString> getSubobjectNames();
    QMap<QString, OpenedH5File::H5SubobjectToVisualize *> getSubobjects();

signals:

public slots:
    void toogleSelected();
    void setSelected(bool value);

private:
    // One subobject is selected
    OpenedH5File::H5SubobjectToVisualize *selectedSubobject;
    QMap<QString, OpenedH5File::H5SubobjectToVisualize *> subobjects;
    int type;
    OpenedH5File *openedH5File;
    QString name;
    // This is for future implementation of multiple selection of datasets
    bool selected;
};

#endif // H5OBJECTTOVISUALIZE_H
