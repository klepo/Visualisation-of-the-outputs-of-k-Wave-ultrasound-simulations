/**
 * @file        reshape.h
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        8  September 2016 (created) \n
 *              11 September 2017 (updated)
 *
 * @brief       The header file with Reshape class declaration.
 *
 * @license     This file is part of the k-Wave-h5-processing tool for processing the HDF5 data
 *              created by the k-Wave toolbox - http://www.k-wave.org. This file may be used,
 *              distributed and modified under the terms of the LGPL version 3 open source
 *              license. A copy of the LGPL license should have been received with this file.
 *              Otherwise, it can be found at: http://www.gnu.org/copyleft/lesser.html.
 *
 * @copyright   Copyright © 2017, Petr Kleparnik, VUT FIT Brno. All Rights Reserved.
 *
 */

#ifndef RESHAPE_H
#define RESHAPE_H

#include <changechunks.h>

/**
 * @brief The Reshape class represents wrapper for reshaping sensor mask type datasets
 */
class Reshape : public ChangeChunks
{
public:
    Reshape(H5Helper::File *outputFile, DtsForPcs *dtsForPcs, Settings *settings);
    virtual void execute();

private:
    virtual void findMinAndMaxPositionFromSensorMask(H5Helper::Dataset *sensorMaskIndexDataset, H5Helper::Vector3D &min, H5Helper::Vector3D &max, bool log = true) final;
    void reshapeCuboid(H5Helper::Dataset *dataset, hsize_t *sensorMaskCornersData, bool log = true);
    void reshapeMaskTypeDataset(H5Helper::Dataset *dataset, H5Helper::Vector3D globalPos, H5Helper::Vector3D dims, H5Helper::Vector4D chunkDims, bool log = true);
};

#endif // RESHAPE_H