/**
 * @file        processing.h
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        8  September 2016 (created)
 *              3  November  2016 (updated)
 *
 * @brief       The header file with Processing class declaration.
 *
 * @license     This application is for preprocessing the HDF5 data created by the k-Wave toolbox - http://www.k-wave.org.
 *              k-Wave h5 processing is free software.
 *
 * @copyright   Copyright © 2016, Petr Kleparnik, VUT FIT Brno. All Rights Reserved.
 *
 */

#ifndef PROCESSING_H
#define PROCESSING_H

#include <dtsforpcs.h>

#define MAX_NUMBER_OF_FRAMES 0 // TODO set 0 -> all steps (frames)

class Processing
{
public:
    Processing(HDF5Helper::File *hDF5OutputFile, DtsForPcs *dtsForPcs, Settings *settings);

    void reshape();
    void changeChunks();
    void donwsampling();
    void compress();
    void decompress();
    void difference();
    void testOfReading();

private:
    void findMinAndMaxPositionFromSensorMask(HDF5Helper::HDF5Vector3D &min, HDF5Helper::HDF5Vector3D &max);
    void computeDstDims(HDF5Helper::HDF5Vector3D dimsSrc, float ratio, HDF5Helper::HDF5Vector3D &dimsDst, HDF5Helper::HDF5Vector3D &chunkSize, Settings *settings);
    void changeChunksOfDataset(HDF5Helper::HDF5Dataset *srcDataset);
    void resampleDataset(HDF5Helper::HDF5Dataset *srcDataset);
    void compressDataset(HDF5Helper::HDF5Dataset *srcDataset);
    void decompressDatasets(HDF5Helper::HDF5Dataset *srcDatasetFi, HDF5Helper::HDF5Dataset *srcDatasetK);
    void substractDatasets(HDF5Helper::HDF5Dataset *datasetOriginal, HDF5Helper::HDF5Dataset *datasetDecoded);
    void copyAttributes(HDF5Helper::HDF5Dataset *srcDataset, HDF5Helper::HDF5Dataset *dstDataset);
    void resize2D(float *dataSrc, float *dataDst, unsigned int srcWidth, unsigned int srcHeight, unsigned int dstWidth, unsigned int dstHeight);
    void resize2D(float *dataSrc, float *dataDst, hsize_t srcWidth, hsize_t srcHeight, hsize_t dstWidth, hsize_t dstHeight);
    void resize3D(float *dataSrc, float *dataDst, unsigned int srcWidth, unsigned int srcHeight, unsigned int srcDepth, unsigned int dstWidth, unsigned int dstHeight, unsigned int dstDepth);
    void resize3D(float *dataSrc, float *dataDst, hsize_t srcWidth, hsize_t srcHeight, hsize_t srcDepth, hsize_t dstWidth, hsize_t dstHeight, hsize_t dstDepth);


    HDF5Helper::File *hDF5OutputFile;
    DtsForPcs *dtsForPcs;
    Settings *settings;
};

#endif // PROCESSING_H
