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
    void testOfReading();

private:
    void findMinAndMaxPositionFromSensorMask(HDF5Helper::HDF5Vector3D &min, HDF5Helper::HDF5Vector3D &max);
    void computeDstDims(HDF5Helper::HDF5Vector3D dimsSrc, double ratio, HDF5Helper::HDF5Vector3D &dimsDst, HDF5Helper::HDF5Vector3D &chunkSize, Settings *settings);
    void changeChunksOfDataset(HDF5Helper::HDF5Dataset *srcDataset);
    void resamplingOfDataset3D(HDF5Helper::HDF5Dataset *srcDataset, HDF5Helper::HDF5Vector3D nDims, HDF5Helper::HDF5Vector3D nDimsDst, HDF5Helper::HDF5Dataset *dstDatasetFinal);
    void resamplingOfDataset4D(HDF5Helper::HDF5Dataset *srcDataset, HDF5Helper::HDF5Vector4D nDims, HDF5Helper::HDF5Vector4D nDimsDst, HDF5Helper::HDF5Dataset *dstDatasetFinal);
    void copyAttributes(HDF5Helper::HDF5Dataset *srcDataset, HDF5Helper::HDF5Dataset *dstDataset);
    void resize(float *dataSrc, float *dataDst, unsigned int w, unsigned int h, unsigned int wDst, unsigned int hDst);
    void resize(float *dataSrc, float *dataDst, hsize_t w, hsize_t h, hsize_t wDst, hsize_t hDst);

    HDF5Helper::File *hDF5OutputFile;
    DtsForPcs *dtsForPcs;
    Settings *settings;
};

#endif // PROCESSING_H
