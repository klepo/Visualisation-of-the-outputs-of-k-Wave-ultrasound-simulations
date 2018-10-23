/**
 * @file        h5openedfile.h
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        30 July      2014 (created) <br>
 *              10 October   2018 (updated)
 *
 * @brief       The header file with OpenedH5File class declaration.
 *
 * @license     This file is part of the k-Wave-h5-visualizer tool for processing the HDF5 data
 *              created by the k-Wave toolbox - http://www.k-wave.org. This file may be used,
 *              distributed and modified under the terms of the LGPL version 3 open source
 *              license. A copy of the LGPL license should have been received with this file.
 *              Otherwise, it can be found at: http://www.gnu.org/copyleft/lesser.html.
 *
 * @copyright   Copyright © 2018, Petr Kleparnik, VUT FIT Brno. All Rights Reserved.
 *
 */

#ifndef H5OPENEDFILE_H
#define H5OPENEDFILE_H

#include <QtCore>

#include <k-wave-h5-helper.h>

class H5ObjectToVisualize;

/**
 * @brief The OpenedH5File class represents wrapper for opened HDF5 file
 */
class H5OpenedFile : public QObject
{
    Q_OBJECT
public:

    /// 3D and 4D object type
    typedef enum ObjectType {
        DATASET_3D,
        DATASET_4D
    } ObjectType;

    explicit H5OpenedFile(QString filename, QObject *parent = nullptr);
    ~H5OpenedFile();

    QVector<H5ObjectToVisualize *> getObjects();
    QVector<H5ObjectToVisualize *> getObjectsSelected();
    QMap<QString, QString> getInfo();
    H5Helper::File *getFile();
    QString getFilename() const;
    QString getRawFilename() const;
    H5Helper::Vector4D getNDims() const;

signals:

public slots:

private:
    Q_DISABLE_COPY(H5OpenedFile)

    H5Helper::File *file = nullptr;
    H5Helper::Vector4D nDims;
    QMap<QString, QString> info;
    QVector<H5ObjectToVisualize *> objects;

    void findDatasetsForVisualization(H5Helper::Group *group);
    void setObject(H5Helper::Dataset *dataset, ObjectType type);
};

#endif // H5OPENEDFILE_H
