/**
 * @file        openedh5file.h
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.0
 * @date        30 July      2014 (created)
 *              6  December  2015 (updated)
 *              8  September 2015 (updated)
 *
 * @brief       The header file with OpenedH5File class declaration.
 *

 * @license     This file is partof k-Wave visualiser application
 * for visualizing HDF5 data created by the k-Wave toolbox - http://www.k-wave.org.
 *
 * @copyright   Copyright © 2016, Petr Kleparnik, VUT FIT Brno. All Rights Reserved.
 *
 * k-Wave visualiser is free software.
 */

#ifndef H5FILE_H
#define H5FILE_H

#include <QtCore>

#include <hdf5helper.h>

class OpenedH5File : public QObject
{
    Q_OBJECT
public:
    explicit OpenedH5File(QString fileName, QObject *parent = 0);
    ~OpenedH5File();

    class H5ObjectToVisualize;
    class H5SubobjectToVisualize;

    QMap<QString, H5ObjectToVisualize *> getObjects();

    H5ObjectToVisualize *getObject(QString mainName);
    H5ObjectToVisualize *getObjectBySubobjectName(QString name);

    QMap<QString, QString> getInfo();

    HDF5Helper::File *getFile();

    QString getFilename();
    QString getRawFilename();

    HDF5Helper::HDF5Vector4D getNDims() const;
    uint64_t getNT();
    uint64_t getNX();
    uint64_t getNY();
    uint64_t getNZ();

    static const int DATASET_TYPE = 0;
    static const int GROUP_TYPE = 1;


signals:

public slots:
    void setSelectedSubobject(QString name);
    void setObjectSelected(QString mainName, bool value);
    void toogleObjectSelected(QString mainName);

private:
    HDF5Helper::File *file;
    HDF5Helper::HDF5Vector4D nDims;

    QMap<QString, QString> info;
    QMap<QString, H5ObjectToVisualize *> objects;

    H5ObjectToVisualize *selectedObject;
};

#endif // H5FILE_H
