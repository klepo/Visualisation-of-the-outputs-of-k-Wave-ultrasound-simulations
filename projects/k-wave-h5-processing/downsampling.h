/**
 * @file        downsampling.h
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        8  September 2016 (created) \n
 *              11 September 2017 (updated)
 *
 * @brief       The header file with Downsampling class declaration.
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


#ifndef DOWNSAMPLING_H
#define DOWNSAMPLING_H

#include <processing.h>

/**
 * @brief The Downsampling class represents wrapper for downsampling
 */
class Downsampling : public Processing
{
public:
    Downsampling(HDF5Helper::File *outputFile, DtsForPcs *dtsForPcs, Settings *settings);
    virtual void execute();

private:
    void resampleDataset(HDF5Helper::Dataset *srcDataset);
    void computeDstDims(HDF5Helper::Vector3D dimsSrc, float ratio, HDF5Helper::Vector3D &dimsDst, HDF5Helper::Vector3D &chunkSize, hsize_t maxChunkSize);
    void resize2D(float *dataSrc, float *dataDst, unsigned int srcWidth, unsigned int srcHeight, unsigned int dstWidth, unsigned int dstHeight);
    void resize2D(float *dataSrc, float *dataDst, hsize_t srcWidth, hsize_t srcHeight, hsize_t dstWidth, hsize_t dstHeight);
    void resize3D(float *dataSrc, float *dataDst, unsigned int srcWidth, unsigned int srcHeight, unsigned int srcDepth, unsigned int dstWidth, unsigned int dstHeight, unsigned int dstDepth);
    void resize3D(float *dataSrc, float *dataDst, hsize_t srcWidth, hsize_t srcHeight, hsize_t srcDepth, hsize_t dstWidth, hsize_t dstHeight, hsize_t dstDepth);
};

#endif // DOWNSAMPLING_H
