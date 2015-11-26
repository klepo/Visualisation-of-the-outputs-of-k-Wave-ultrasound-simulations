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

class HDF5Helper::File::HDF5Dataset : public HDF5Helper::File::HDF5Object
{
public:
    HDF5Dataset(const hid_t dataset, const std::string name, File *hDF5File);
    ~HDF5Dataset();

    void readFullDataset(float *&data);
    void readFullDataset(hsize_t *&data);

    void read3DDataset(File::HDF5Vector3D offset, File::HDF5Vector3D count, float *&data, float &minVF, float &maxVF);
    void read3DDataset(File::HDF5Vector3D offset, File::HDF5Vector3D count, hsize_t *&data, hsize_t &minVI, hsize_t &maxVI);

    void write3DDataset(File::HDF5Vector3D offset, File::HDF5Vector3D count, float *data, bool log = false);
    void write3DDataset(File::HDF5Vector3D offset, File::HDF5Vector3D count, hsize_t *data, bool log = false);

    void readBlock(const hsize_t index, File::HDF5Vector3D &offset, File::HDF5Vector3D &count, float *&data, float &minVFTmp, float &maxVFTmp);
    void readBlock(const hsize_t index, File::HDF5Vector3D &offset, File::HDF5Vector3D &count, hsize_t *&data, hsize_t &minVFTmp, hsize_t &maxVFTmp);

    void readEmptyBlock();

    void getMinAndMaxValue(const float *data, const hsize_t size, float &minVF, float &maxVF);
    void getMinAndMaxValue(const hsize_t *data, const hsize_t size, hsize_t &minVI, hsize_t &maxVI);

    #ifdef PARALLEL_HDF5
        void setMPIOAccess(H5FD_mpio_xfer_t type);
    #endif

    hsize_t getRealNumberOfElmsToLoad();
    hsize_t getNumberOfBlocks();

    void setNumberOfElmsToLoad(hsize_t size);
    hsize_t getNumberOfElmsToLoad();
    File::HDF5Vector3D getGeneralBlockDims();

    hsize_t getRank();
    hsize_t getSize();
    File::HDF5Vector3D getDims();
    File::HDF5Vector3D getChunkDims();

    hsize_t getId();
    std::string getName();

    void findAndSetGlobalMinAndMaxValue(bool reset = false);
    void findGlobalMinAndMaxValue(bool reset = false);

    hsize_t getGlobalMaxValueI(bool reset = false);
    hsize_t getGlobalMinValueI(bool reset = false);

    float getGlobalMaxValueF(bool reset = false);
    float getGlobalMinValueF(bool reset = false);

    H5T_class_t getDataTypeClass();

private:
    void findGlobalMinAndMaxValueF();
    void findGlobalMinAndMaxValueI();

    void checkOffsetAndCountParams(File::HDF5Vector3D offset, File::HDF5Vector3D count);

    void initBlockReading();
    void recomputeBlock();
    void computeNumberOfBlocks();
    void iterateToBlock(const hsize_t index);

    hid_t plist;
    hid_t plist_DATASET_XFER;
    void *convBuffer;
    void *bkgBuffer;

    File::HDF5Vector3D blockDims;
    File::HDF5Vector3D offset;
    File::HDF5Vector3D count;
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
    File::HDF5Vector3D dims;
    File::HDF5Vector3D chunk_dims;

    std::string name;

    hsize_t maxVI;
    hsize_t minVI;

    float maxVF;
    float minVF;

    bool issetGlobalMinAndMaxValue;
};

#endif // HDF5DATASET_H
