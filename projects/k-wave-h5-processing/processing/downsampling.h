/**
 * @file        downsampling.h
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        8  September 2016 (created) <br>
 *              25 October   2018 (updated)
 *
 * @brief       The header file with Downsampling class declaration.
 *
 * @license     This file is part of the k-Wave-h5-processing tool for processing the HDF5 data
 *              created by the k-Wave toolbox - http://www.k-wave.org. This file may be used,
 *              distributed and modified under the terms of the LGPL version 3 open source
 *              license. A copy of the LGPL license should have been received with this file.
 *              Otherwise, it can be found at: http://www.gnu.org/copyleft/lesser.html.
 *
 * @copyright   Copyright © 2018, Petr Kleparnik, VUT FIT Brno. All Rights Reserved.
 *
 */


#ifndef DOWNSAMPLING_H
#define DOWNSAMPLING_H

#include "processing.h"

/**
 * @brief The Downsampling class represents wrapper for downsampling
 */
class Downsampling : public Processing
{
public:
    Downsampling(H5Helper::File *outputFile, DtsForPcs *dtsForPcs, const Settings *settings);
    virtual void execute();

private:
    void resampleDataset(H5Helper::Dataset *srcDataset, bool log = true);
    void computeDstDims(H5Helper::Vector3D dimsSrc, H5Helper::Vector3D maxChunkDims, float ratio, H5Helper::Vector3D &dimsDst, H5Helper::Vector3D &chunkSize);
    void resize2D(const float *dataSrc, float *dataDst, hsize_t srcWidth, hsize_t srcHeight, hsize_t dstWidth, hsize_t dstHeight);
    void resize3D(const float *dataSrc, float *dataDst, hsize_t srcWidth, hsize_t srcHeight, hsize_t srcDepth, hsize_t dstWidth, hsize_t dstHeight, hsize_t dstDepth);
};

#endif // DOWNSAMPLING_H
