/**
 * @file        reshape.h
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        8  September 2016 (created) \n
 *              28 August    2017 (updated)
 *
 * @brief       The header file with Reshape class declaration.
 *
 * @license     This file is part of the k-Wave-h5-processing tool for processing the HDF5 data
 *              created by the k-Wave toolbox - http://www.k-wave.org. This file may be used,
 *              distributed and modified under the terms of the LGPL version 3 open source
 *              license. A copy of the LGPL license should have been recieved with this file.
 *              Otherwise, it can be found at: http://www.gnu.org/copyleft/lesser.html.
 *
 * @copyright   Copyright © 2017, Petr Kleparnik, VUT FIT Brno. All Rights Reserved.
 *
 */


#ifndef RESHAPE_H
#define RESHAPE_H

#include "changechunks.h"

class Reshape : public ChangeChunks
{
public:
    Reshape(HDF5Helper::File *outputFile, DtsForPcs *dtsForPcs, Settings *settings);
    virtual void execute();

private:
    virtual void findMinAndMaxPositionFromSensorMask(HDF5Helper::Dataset *sensorMaskIndexDataset, HDF5Helper::Vector3D &min, HDF5Helper::Vector3D &max) final;
};

#endif // RESHAPE_H
