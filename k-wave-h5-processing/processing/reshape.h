/**
 * @file        reshape.h
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        8  September 2016 (created) <br>
 *              27 March     2019 (updated)
 *
 * @brief       The header file with Reshape class declaration.
 *
 * @license     This file is part of the k-Wave-h5-processing tool for processing the HDF5 data
 *              created by the k-Wave toolbox - http://www.k-wave.org. This file may be used,
 *              distributed and modified under the terms of the LGPL version 3 open source
 *              license. A copy of the LGPL license should have been received with this file.
 *              Otherwise, it can be found at: http://www.gnu.org/copyleft/lesser.html.
 *
 * @copyright   Copyright © 2019, Petr Kleparnik, VUT FIT Brno. All Rights Reserved.
 *
 */

#ifndef RESHAPE_H
#define RESHAPE_H

#include "changechunks.h"

/**
 * @brief The Reshape class represents wrapper for reshaping sensor mask type datasets
 */
class Reshape : public ChangeChunks
{
public:
    Reshape(H5Helper::File *outputFile, DtsForPcs *dtsForPcs, const Settings *settings);
    virtual void execute();

private:
    virtual void findMinAndMaxPositionFromSensorMask(H5Helper::Dataset *sensorMaskIndexDataset, H5Helper::Vector3D &min, H5Helper::Vector3D &max, bool log = true) final;
    void reshapeCuboid(H5Helper::Dataset *dataset, const hsize_t *sensorMaskCornersData, bool log = true);
    void reshapeMaskTypeDataset(H5Helper::Dataset *dataset, H5Helper::Vector3D globalPosTmp, H5Helper::Vector3D dimsTmp, H5Helper::Vector4D chunkDimsTmp, bool log = true);
};

#endif // RESHAPE_H
