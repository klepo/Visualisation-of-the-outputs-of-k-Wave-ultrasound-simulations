/**
 * @file        openedh5file.h
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        30 July      2014 (created) \n
 *              27 February  2017 (updated)
 *
 * @brief       The header file with OpenedH5File class declaration.
 *
 *
 * @license     This file is part of k-Wave visualiser application
 * for visualizing HDF5 data created by the k-Wave toolbox - http://www.k-wave.org.
 *
 * @copyright   Copyright © 2017, Petr Kleparnik, VUT FIT Brno. All Rights Reserved.
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

    typedef enum ObjectType {
        dataset3D_t,
        dataset4D_t
    } ObjectType;

    explicit OpenedH5File(QString fileName, QObject *parent = 0);
    ~OpenedH5File();

    class H5ObjectToVisualize;
    class H5SubobjectToVisualize;

    QMap<QString, H5ObjectToVisualize *> getObjects();

    H5ObjectToVisualize *getObject(QString mainName);
    H5ObjectToVisualize *getObjectBySubobjectName(QString name);

    QMap<QString, QString> getInfo();

    HDF5Helper::File *getFile();

    QString getFilename() const;
    QString getRawFilename() const;

    HDF5Helper::HDF5Vector4D getNDims() const;
    hsize_t getNT() const;
    hsize_t getNX() const;
    hsize_t getNY() const;
    hsize_t getNZ() const;

signals:

public slots:
    void setSelectedSubobject(QString name);
    void setObjectSelected(QString mainName, bool value);
    void toogleObjectSelected(QString mainName);

private:
    HDF5Helper::File *file = 0;
    HDF5Helper::HDF5Vector4D nDims;
    void findDatasetsForVisualization(HDF5Helper::HDF5Group *group);

    QMap<QString, QString> info;
    QMap<QString, H5ObjectToVisualize *> objects;

    H5ObjectToVisualize *selectedObject = 0;

    void setObject(QString name, HDF5Helper::HDF5Dataset *dataset, ObjectType type);
};

#endif // H5FILE_H
