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
    void testOfReading();

    void static xcorr(float *dataSrc1, float *dataSrc2, float *dataDst, const int length1, const int length2);
    void static conv(float *dataSrc1, float *dataSrc2, float *dataDst, const int length1, const int length2);
    void static findPeaks(float *dataSrc, int *dataDst, const int length, int &lengthDst);
    void static diff(float *dataSrc, float *dataDst, const int length);
    void static diff(int *dataSrc, int *dataDst, const int length);
    float static mean(float *dataSrc, const int length);
    int static mean(int *dataSrc, const int length);

private:
    void findMinAndMaxPositionFromSensorMask(HDF5Helper::HDF5Vector3D &min, HDF5Helper::HDF5Vector3D &max);
    void computeDstDims(HDF5Helper::HDF5Vector3D dimsSrc, double ratio, HDF5Helper::HDF5Vector3D &dimsDst, HDF5Helper::HDF5Vector3D &chunkSize, Settings *settings);
    void changeChunksOfDataset(HDF5Helper::HDF5Dataset *srcDataset);
    void resamplingOfDataset(HDF5Helper::HDF5Dataset *srcDataset, HDF5Helper::HDF5Vector dimsSrc, HDF5Helper::HDF5Vector dimsDst, HDF5Helper::HDF5Dataset *dstDataset);
    void compressDataset(HDF5Helper::HDF5Dataset *srcDataset);
    void copyAttributes(HDF5Helper::HDF5Dataset *srcDataset, HDF5Helper::HDF5Dataset *dstDataset);
    hsize_t getPeriod(float *dataSrc, const hsize_t length);
    void resize2D(float *dataSrc, float *dataDst, unsigned int srcWidth, unsigned int srcHeight, unsigned int dstWidth, unsigned int dstHeight);
    void resize2D(float *dataSrc, float *dataDst, hsize_t srcWidth, hsize_t srcHeight, hsize_t dstWidth, hsize_t dstHeight);
    void resize3D(float *dataSrc, float *dataDst, unsigned int srcWidth, unsigned int srcHeight, unsigned int srcDepth, unsigned int dstWidth, unsigned int dstHeight, unsigned int dstDepth);
    void resize3D(float *dataSrc, float *dataDst, hsize_t srcWidth, hsize_t srcHeight, hsize_t srcDepth, hsize_t dstWidth, hsize_t dstHeight, hsize_t dstDepth);


    HDF5Helper::File *hDF5OutputFile;
    DtsForPcs *dtsForPcs;
    Settings *settings;
};

#endif // PROCESSING_H
