/**
 * @file        HDF5Dataset.h
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        30 July      2014 (created)
 *              8  September 2016 (updated)
 *              3  November  2016 (updated)
 *
 * @brief       The header file with HDF5Dataset class declaration.
 *
 * @license     This file is part of the hdf5helper library for k-Wave h5 processing
 *              for preprocessing the HDF5 data created by the k-Wave toolbox - http://www.k-wave.org.
 *              The hdf5helper library is free software.
 *
 * @copyright   Copyright © 2016, Petr Kleparnik, VUT FIT Brno. All Rights Reserved.
 *
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

    std::string getName() const;
    std::string getOnlyName() const;
    hid_t getId() const;
    hsize_t getRank() const;
    HDF5Vector getDims() const;
    HDF5Vector getChunkDims() const;
    hsize_t getSize() const;
    H5T_class_t getDataTypeClass() const;

    void getGlobalMaxValue(float &value, bool reset = false);
    void getGlobalMinValue(float &value, bool reset = false);
    void getGlobalMaxValue(hsize_t &value, bool reset = false);
    void getGlobalMinValue(hsize_t &value, bool reset = false);

    void getMinAndMaxValue(const float *data, const hsize_t size, float &minVF, float &maxVF);
    void getMinAndMaxValue(const hsize_t *data, const hsize_t size, hsize_t &minVI, hsize_t &maxVI);

    void findAndSetGlobalMinAndMaxValue(bool reset = false);

    // Block reading
    hsize_t getRealNumberOfElmsToLoad() const;
    hsize_t getNumberOfBlocks() const;
    HDF5Vector getNumberOfBlocksInDims() const;

    void setNumberOfElmsToLoad(hsize_t size);
    void setMaxNumberOfElmsToLoad(hsize_t size);
    hsize_t getNumberOfElmsToLoad();
    HDF5Vector getGeneralBlockDims() const;

    void setMPIOAccess(H5FD_mpio_xfer_t type);

    void readDataset(HDF5Vector offset, HDF5Vector count, float *&data, float &min, float &max, bool log = true);
    void readDataset(HDF5Vector offset, HDF5Vector count, hsize_t *&data, hsize_t &min, hsize_t &max, bool log = true);
    void readDataset(HDF5Vector offset, HDF5Vector count, float *&data, bool log = true);
    void readDataset(HDF5Vector offset, HDF5Vector count, hsize_t *&data, bool log = true);
    void readDataset(float *&data, bool log = true);
    void readDataset(hsize_t *&data, bool log = true);
    void readDataset(float *&data, float &min, float &max, bool log = true);
    void readDataset(hsize_t *&data, hsize_t &min, hsize_t &max, bool log = true);

    void writeDataset(HDF5Vector offset, HDF5Vector count, float *data, bool log = false);
    void writeDataset(HDF5Vector offset, HDF5Vector count, hsize_t *data, bool log = false);
    void writeDataset(float *data, bool log = false);
    void writeDataset(hsize_t *data, bool log = false);

    void readBlock(const hsize_t index, HDF5Vector &offset, HDF5Vector &count, float *&data, float &min, float &max, bool log = true);
    void readBlock(const hsize_t index, HDF5Vector &offset, HDF5Vector &count, hsize_t *&data, hsize_t &min, hsize_t &max, bool log = true);
    void readBlock(const hsize_t index, HDF5Vector &offset, HDF5Vector &count, float *&data, bool log = true);
    void readBlock(const hsize_t index, HDF5Vector &offset, HDF5Vector &count, hsize_t *&data, bool log = true);

    void readEmptyBlock();

private:
    void readDatasetGeneral(HDF5Vector offset, HDF5Vector count, void *data, bool log = true);
    void writeDatasetGeneral(HDF5Vector offset, HDF5Vector count, void *data, bool log = false);

    void checkOffsetAndCountParams(HDF5Vector offset, HDF5Vector count);

    void findGlobalMinAndMaxValue(bool reset = false);
    void findGlobalMinAndMaxValueF();
    void findGlobalMinAndMaxValueI();

    void initBlockReading();

    void checkTypeAndAllocation(float *&data, int type, hsize_t size);
    void checkTypeAndAllocation(hsize_t *&data, int type, hsize_t size);

    std::string memoryErrorMessage(hsize_t size, int type);
    std::string readErrorMessage(hsize_t size, int type);
    std::string typeString(int type);

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
