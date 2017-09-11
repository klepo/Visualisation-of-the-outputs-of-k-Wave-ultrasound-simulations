/**
 * @file        openedh5file.h
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        30 July      2014 (created) \n
 *              11 September 2017 (updated)
 *
 * @brief       The header file with OpenedH5File class declaration.
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

#ifndef H5FILE_H
#define H5FILE_H

#include <QtCore>

#include <hdf5helper.h>

/**
 * @brief The OpenedH5File class represents wrapper for opened HDF5 file
 */
class OpenedH5File : public QObject
{
    Q_OBJECT
public:

    /// 3D 4D object type
    typedef enum ObjectType {
        dataset3D_t,
        dataset4D_t
    } ObjectType;

    explicit OpenedH5File(QString fileName, QObject *parent = 0);
    ~OpenedH5File();

    class H5ObjectToVisualize;
    class H5SubobjectToVisualize;

    QMap<QString, H5ObjectToVisualize *> getObjects();

    H5ObjectToVisualize *getObject(QString name);
    H5ObjectToVisualize *getObjectBySubobjectName(QString name);

    QMap<QString, QString> getInfo();

    HDF5Helper::File *getFile();

    QString getFilename() const;
    QString getRawFilename() const;

    HDF5Helper::Vector4D getNDims() const;
    hsize_t getNT() const;
    hsize_t getNX() const;
    hsize_t getNY() const;
    hsize_t getNZ() const;

signals:

public slots:
    void setSelectedSubobject(QString name);
    void setObjectSelected(QString name, bool value);
    void toogleObjectSelected(QString mainName);

private:
    HDF5Helper::File *file = 0;
    HDF5Helper::Vector4D nDims;
    void findDatasetsForVisualization(HDF5Helper::Group *group);

    QMap<QString, QString> info;
    QMap<QString, H5ObjectToVisualize *> objects;

    H5ObjectToVisualize *selectedObject = 0;

    void setObject(QString name, HDF5Helper::Dataset *dataset, ObjectType type);
};

#endif // H5FILE_H
