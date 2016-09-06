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
    std::string getOnlyName();
    hid_t getId();
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
    HDF5Vector getNumberOfBlocksInDims() const;

    HDF5Vector getGeneralBlockDims();
    void setNumberOfElmsToLoad(hsize_t size);
    void setMaxNumberOfElmsToLoad(hsize_t size);
    hsize_t getNumberOfElmsToLoad();

    void setMPIOAccess(H5FD_mpio_xfer_t type);

    void readFullDataset(float *&data);
    void readFullDataset(hsize_t *&data);

    void readDataset(HDF5Vector offset, HDF5Vector count, float *&data, float &min, float &max);
    void readDataset(HDF5Vector offset, HDF5Vector count, hsize_t *&data, hsize_t &min, hsize_t &max);

    void writeDataset(HDF5Vector offset, HDF5Vector count, float *data, bool log = false);
    void writeDataset(HDF5Vector offset, HDF5Vector count, hsize_t *data, bool log = false);

    void readBlock(const hsize_t index, HDF5Vector &offset, HDF5Vector &count, float *&data, float &min, float &max);
    void readBlock(const hsize_t index, HDF5Vector &offset, HDF5Vector &count, hsize_t *&data, hsize_t &min, hsize_t &max);

    void readEmptyBlock();

private:
    void readFullDataset(void *data);

    void readDataset(HDF5Vector offset, HDF5Vector count, void *data);
    void writeDataset(HDF5Vector offset, HDF5Vector count, void *data, bool log);

    void checkOffsetAndCountParams(HDF5Vector offset, HDF5Vector count);

    void findGlobalMinAndMaxValueF();
    void findGlobalMinAndMaxValueI();

    void initBlockReading();

    hid_t plist;
    hid_t plist_DATASET_XFER;
    void *convBuffer;
    void *bkgBuffer;

    // Block reading
    hsize_t numberOfBlocks;
    HDF5Vector numberOfBlocksInDims;
    HDF5Vector *offsets;
    HDF5Vector *counts;
    hsize_t numberOfElementsToLoad;
    hsize_t realNumberOfElementsToLoad;

    hid_t dataset;
    hid_t dataspace;
    hid_t datatype;

    int rank;
    HDF5Vector dims;
    HDF5Vector chunkDims;

    std::string name;

    hsize_t maxVI;
    hsize_t minVI;

    float maxVF;
    float minVF;

    bool issetGlobalMinAndMaxValue;
};
}

#endif // HDF5DATASET_H
