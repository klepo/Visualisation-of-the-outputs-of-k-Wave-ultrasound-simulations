/*
 * @file        HDF5Dataset.h
 * @author      Petr Kleparnik, VUT FIT Brno, xklepa01@stud.fit.vutbr.cz
 * @version     0.0
 * @date        30 July 2014
 *
 * @brief       The header file with HDF5Dataset class declaration.
 *
 * @section     Licence
 * This file is part of hdf5file library for k-Wave h5 processing
 * for preprocessing HDF5 data created by the k-Wave toolbox - http://www.k-wave.org.
 * Copyright © 2014, Petr Kleparnik, VUT FIT Brno.
 * hdf5file library is free software.
 */

#ifndef HDF5DATASET_H
#define HDF5DATASET_H

#include "HDF5Object.h"
#include "HDF5Vector3D.h"
#include "HDF5Vector4D.h"

namespace HDF5Helper
{
class HDF5Dataset : public HDF5Object
{
public:
    HDF5Dataset(const hid_t dataset, const std::string name, File *hDF5File);
    ~HDF5Dataset();

    std::string getName();
    hsize_t getId();
    hsize_t getRank();
    HDF5Vector getDims();
    HDF5Vector getChunkDims();
    hsize_t getSize();
    H5T_class_t getDataTypeClass();

    hsize_t getGlobalMaxValueI(bool reset = false);
    hsize_t getGlobalMinValueI(bool reset = false);

    float getGlobalMaxValueF(bool reset = false);
    float getGlobalMinValueF(bool reset = false);

    void getMinAndMaxValue(const float *data, const hsize_t size, float &minVF, float &maxVF);
    void getMinAndMaxValue(const hsize_t *data, const hsize_t size, hsize_t &minVI, hsize_t &maxVI);

    void findAndSetGlobalMinAndMaxValue(bool reset = false);
    void findGlobalMinAndMaxValue(bool reset = false);

    // Block reading
    hsize_t getRealNumberOfElmsToLoad();
    hsize_t getNumberOfBlocks();

    HDF5Vector3D getGeneralBlockDims();
    void setNumberOfElmsToLoad(hsize_t size);
    hsize_t getNumberOfElmsToLoad();

#ifdef PARALLEL_HDF5
    void setMPIOAccess(H5FD_mpio_xfer_t type);
#endif

    void readFullDataset(float *&data);
    void readFullDataset(hsize_t *&data);

    void read3DDataset(HDF5Vector3D offset, HDF5Vector3D count, float *&data, float &minVF, float &maxVF);
    void read3DDataset(HDF5Vector3D offset, HDF5Vector3D count, hsize_t *&data, hsize_t &minVI, hsize_t &maxVI);

    void write3DDataset(HDF5Vector3D offset, HDF5Vector3D count, float *data, bool log = false);
    void write3DDataset(HDF5Vector3D offset, HDF5Vector3D count, hsize_t *data, bool log = false);

    void read4DDataset(HDF5Vector4D offset, HDF5Vector4D count, float *&data, float &minVF, float &maxVF);

    void write4DDataset(HDF5Vector4D offset, HDF5Vector4D count, float *data, bool log = false);

    void read3DBlock(const hsize_t index, HDF5Vector3D &offset, HDF5Vector3D &count, float *&data, float &minVFTmp, float &maxVFTmp);
    void read3DBlock(const hsize_t index, HDF5Vector3D &offset, HDF5Vector3D &count, hsize_t *&data, hsize_t &minVFTmp, hsize_t &maxVFTmp);

    void readEmptyBlock();

private:
    void readFullDataset(void *data);
    void read3DDataset(HDF5Vector3D offset, HDF5Vector3D count, void *data);
    void write3DDataset(HDF5Vector3D offset, HDF5Vector3D count, void *data, bool log = false);
    void read4DDataset(HDF5Vector4D offset, HDF5Vector4D count, void *data);
    void write4DDataset(HDF5Vector4D offset, HDF5Vector4D count, void *data, bool log = false);

    void checkOffsetAndCountParams(HDF5Vector offset, HDF5Vector count);

    void findGlobalMinAndMaxValueF();
    void findGlobalMinAndMaxValueI();

    void initBlockReading();
    void iterateToBlock(const hsize_t index);
    void computeNumberOfBlocks();
    void recomputeBlock();

    hid_t plist;
    hid_t plist_DATASET_XFER;
    void *convBuffer;
    void *bkgBuffer;

    HDF5Vector3D blockDims;
    HDF5Vector3D offset;
    HDF5Vector3D count;
    bool blockInitialized;
    bool lastBlock;
    hsize_t blockSize;
    hsize_t numberOfBlocks;
    hsize_t actualBlock;

    hsize_t numberOfElementsToLoad;

    hid_t dataset;
    hid_t dataspace;
    hid_t datatype;

    int rank;
    HDF5Vector dims;
    HDF5Vector chunk_dims;

    std::string name;

    hsize_t maxVI;
    hsize_t minVI;

    float maxVF;
    float minVF;

    bool issetGlobalMinAndMaxValue;
};
}

#endif // HDF5DATASET_H
