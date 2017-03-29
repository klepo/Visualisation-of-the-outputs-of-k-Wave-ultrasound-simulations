/**
 * @file        HDF5Dataset.h
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        30 July      2014 (created) \n
 *              27 February  2017 (updated)
 *
 * @brief       The header file with HDF5Dataset class declaration.
 *
 * @license     This file is part of the hdf5helper library for k-Wave h5 processing
 *              for preprocessing the HDF5 data created by the k-Wave toolbox - http://www.k-wave.org.
 *              The hdf5helper library is free software.
 *
 * @copyright   Copyright © 2017, Petr Kleparnik, VUT FIT Brno. All Rights Reserved.
 *
 */

#ifndef HDF5DATASET_H
#define HDF5DATASET_H

#include <map>
#include <assert.h>

#include <HDF5Object.h>
#include <HDF5Vector3D.h>
#include <HDF5Vector4D.h>

namespace HDF5Helper
{

/// k-Wave dataset types
enum class HDF5DatasetType
{
    ALL,
    UNKNOWN,
    N_DIM_X,
    N_DIM_Y,
    N_DIM_Z,
    N_DIM_T,
    MASK_INDEX,
    MASK_CORNERS,
    P_SOURCE_INPUT,
    BASIC_3D,
    DWNSMPL_3D,
    MASK_3D,
    BASIC_MASK,
    TIME_STEPS_MASK,
    TIME_STEPS_FI_MASK,
    TIME_STEPS_K_MASK,
    TIME_STEPS_D_MASK,
    TIME_STEPS_S_MASK,
    CUBOID,
    CUBOID_FI,
    CUBOID_K,
    CUBOID_D,
    CUBOID_S,
    CUBOID_DWNSMPL,
    CUBOID_DWNSMPL_FI,
    CUBOID_DWNSMPL_K,
    CUBOID_DWNSMPL_D,
    CUBOID_DWNSMPL_S,
    CUBOID_ATTR,
    CUBOID_ATTR_FI,
    CUBOID_ATTR_K,
    CUBOID_ATTR_D,
    CUBOID_ATTR_S,
    CUBOID_ATTR_DWNSMPL,
    CUBOID_ATTR_DWNSMPL_FI,
    CUBOID_ATTR_DWNSMPL_K,
    CUBOID_ATTR_DWNSMPL_D,
    CUBOID_ATTR_DWNSMPL_S,
};

/**
 * @brief The HDF5Dataset class represents wrapper for HDF5 datasets
 */
class HDF5Dataset : public HDF5Object
{
public:
    HDF5Dataset(const hid_t dataset, const std::string name, File *hDF5File);
    ~HDF5Dataset();

    std::string getOnlyName() const;
    hid_t getId() const;
    hsize_t getRank() const;
    HDF5Vector getDims() const;
    HDF5Vector getChunkDims() const;
    hsize_t getSize() const;
    H5T_class_t getDataTypeClass() const;
    hid_t getDataType() const;
    HDF5DatasetType getType(hsize_t sensorMaskSize = 0) const;
    std::string getTypeString(HDF5DatasetType type) const;

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
    hsize_t getNumberOfElmsToLoad() const;
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
    void readDataset(float &data, bool log = true);
    void readDataset(hsize_t &data, bool log = true);

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

    void checkDataTypeAndAllocation(float *&data, int type, hsize_t size);
    void checkDataTypeAndAllocation(hsize_t *&data, int type, hsize_t size);

    void checkType();

    std::string memoryErrorMessage(hsize_t size, int type) const;
    std::string readErrorMessage(hsize_t size, int type) const;
    std::string dataTypeString(int type) const;

    hid_t plist = 0;
    hid_t plist_DATASET_XFER = 0;

    // Block reading
    hsize_t numberOfBlocks = 0;
    HDF5Vector numberOfBlocksInDims;
    HDF5Vector *offsets;
    HDF5Vector *counts;
    hsize_t numberOfElementsToLoad = 0;
    hsize_t realNumberOfElementsToLoad = 0;

    hid_t dataset = 0;
    hid_t dataspace = 0;
    hid_t datatype = 0;

    int rank = 0;
    HDF5Vector dims;
    HDF5Vector chunkDims;

    hsize_t maxVI = 0;
    hsize_t minVI = 0;

    float maxVF = 0;
    float minVF = 0;

    bool issetGlobalMinAndMaxValue = false;
};

typedef std::map<const std::string, HDF5Dataset *> MapOfDatasets;
typedef HDF5Helper::MapOfDatasets::iterator MapOfDatasetsIt;
typedef std::pair<const std::string, HDF5Dataset *> PairOfDatasets;
}

#endif // HDF5DATASET_H
