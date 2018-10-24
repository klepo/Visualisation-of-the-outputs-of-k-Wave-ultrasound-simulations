/**
 * @file        dataset.h
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        30 July      2014 (created) <br>
 *              23 October   2018 (updated)
 *
 * @brief       The header file with H5Helper::Dataset class declaration.
 *
 * @license     This file is part of the k-wave-h5-helper library for processing the HDF5 data
 *              created by the k-Wave toolbox - http://www.k-wave.org. This file may be used,
 *              distributed and modified under the terms of the LGPL version 3 open source
 *              license. A copy of the LGPL license should have been received with this file.
 *              Otherwise, it can be found at: http://www.gnu.org/copyleft/lesser.html.
 *
 * @copyright   Copyright © 2018, Petr Kleparnik, VUT FIT Brno. All Rights Reserved.
 *
 */

#ifndef DATASET_H
#define DATASET_H

#include <vector>
#include <map>
#include <assert.h>
#include <iomanip>
#include <complex>

#include "object.h"
#include "vector3d.h"
#include "vector4d.h"

namespace H5Helper
{
/// k-Wave dataset types
enum class DatasetType
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
    RESHAPED_3D,
    BASIC_3D_DWNSMPL,
    RESHAPED_3D_DWNSMPL,
    BASIC_INDEX,
    TIME_STEPS_INDEX,
    TIME_STEPS_C_INDEX,
    TIME_STEPS_D_INDEX,
    TIME_STEPS_S_INDEX,
    CUBOID,
    CUBOID_C,
    CUBOID_D,
    CUBOID_S,
    CUBOID_DWNSMPL,
    CUBOID_DWNSMPL_C,
    CUBOID_DWNSMPL_D,
    CUBOID_DWNSMPL_S,
    CUBOID_ATTR,
    CUBOID_ATTR_C,
    CUBOID_ATTR_D,
    CUBOID_ATTR_S,
    CUBOID_ATTR_DWNSMPL,
    CUBOID_ATTR_DWNSMPL_C,
    CUBOID_ATTR_DWNSMPL_D,
    CUBOID_ATTR_DWNSMPL_S,
};

/**
 * @brief The Dataset class represents wrapper for the HDF5 datasets
 */
class Dataset : public Object
{
public:
    Dataset(hid_t dataset, std::string name, File *file);
    ~Dataset();

    hid_t getId() const;
    hsize_t getRank() const;
    Vector getDims() const;
    Vector getChunkDims() const;
    hsize_t getSize() const;
    H5T_class_t getDataTypeClass() const;
    hid_t getDataType() const;
    DatasetType getType(hsize_t sensorMaskSize = 0) const;
    std::string getTypeString() const;
    std::string getTypeString(DatasetType type) const;

    void getGlobalMaxValue(float &value, hsize_t &maxVFIndex, bool reset = false);
    void getGlobalMinValue(float &value, hsize_t &minVFIndex, bool reset = false);
    void getGlobalMaxValue(hsize_t &value, hsize_t &maxVIIndex, bool reset = false);
    void getGlobalMinValue(hsize_t &value, hsize_t &minVIIndex, bool reset = false);

    void findAndSetGlobalMinAndMaxValue(bool reset = false, bool log = true);

    // Block reading
    hsize_t getRealNumberOfElmsToLoad() const;
    hsize_t getNumberOfBlocks() const;
    Vector getNumberOfBlocksInDims() const;
    Vector getGeneralBlockDims() const;
    hsize_t getNumberOfElmsToLoad() const;
    void setNumberOfElmsToLoad(hsize_t count);
    void setMaxNumberOfElmsToLoad(hsize_t count);

    void setMPIOAccess(H5FD_mpio_xfer_t type, bool log = true);

    void readDataset(Vector offset, Vector count, float *&data, float &min, float &max, hsize_t &minIndex, hsize_t &maxIndex, bool log = true, hsize_t block = 0);
    void readDataset(Vector offset, Vector count, hsize_t *&data, hsize_t &min, hsize_t &max, hsize_t &minIndex, hsize_t &maxIndex, bool log = true, hsize_t block = 0);
    void readDataset(Vector offset, Vector count, float *&data, bool log = true, hsize_t block = 0);
    void readDataset(Vector offset, Vector count, hsize_t *&data, bool log = true, hsize_t block = 0);
    void readDataset(float *&data, bool log = true);
    void readDataset(hsize_t *&data, bool log = true);
    void readDataset(float *&data, float &min, float &max, hsize_t &minIndex, hsize_t &maxIndex, bool log = true);
    void readDataset(hsize_t *&data, hsize_t &min, hsize_t &max, hsize_t &minIndex, hsize_t &maxIndex, bool log = true);
    void readDataset(float &data, bool log = true);
    void readDataset(hsize_t &data, bool log = true);

    void writeDataset(Vector offset, Vector count, float *data, bool log = false);
    void writeDataset(Vector offset, Vector count, hsize_t *data, bool log = false);
    void writeDataset(float *data, bool log = false);
    void writeDataset(hsize_t *data, bool log = false);

    void readBlock(hsize_t index, Vector &offset, Vector &count, float *&data, float &min, float &max, hsize_t &minIndex, hsize_t &maxIndex, bool log = true);
    void readBlock(hsize_t index, Vector &offset, Vector &count, hsize_t *&data, hsize_t &min, hsize_t &max, hsize_t &minIndex, hsize_t &maxIndex, bool log = true);
    void readBlock(hsize_t index, Vector &offset, Vector &count, float *&data, bool log = true);
    void readBlock(hsize_t index, Vector &offset, Vector &count, hsize_t *&data, bool log = true);

    void readEmptyBlock(bool log = true);

private:
    Dataset(const Dataset &);
    Dataset &operator=(const Dataset &);
    void readDatasetGeneral(Vector offset, Vector count, void *data, bool log = true);
    void writeDatasetGeneral(Vector offset, Vector count, void *data, bool log = false);

    void checkOffsetAndCountParams(Vector offset, Vector count) const;

    void findMinAndMaxValue(const float *data, hsize_t size, float &minVF, float &maxVF, hsize_t &minVFIndex, hsize_t &maxVFIndex) const;
    void findMinAndMaxValue(const hsize_t *data, hsize_t size, hsize_t &minVI, hsize_t &maxVI, hsize_t &minVIIndex, hsize_t &maxVIIndex) const;

    void findGlobalMinAndMaxValue(bool reset = false, bool log = true);
    void findGlobalMinAndMaxValueF(bool log = true);
    void findGlobalMinAndMaxValueI(bool log = true);

    void initBlockReading();
    Vector getBlockCount(hsize_t index) const;
    Vector getBlockOffset(hsize_t index) const;

    void checkDataTypeAndAllocation(float *&data, int type, hsize_t size) const;
    void checkDataTypeAndAllocation(hsize_t *&data, int type, hsize_t size) const;

    void checkType(int type) const;
    void checkFloatType() const;
    void checkIntegerType() const;
    bool isFloatType() const;
    bool isIntegerType() const;

    std::string dataTypeString(int type) const;
    std::string memoryErrorMessage(hsize_t size, int type) const;
    std::string readErrorMessage(hsize_t size, int type) const;

    void printsReadingMessage(hsize_t block = 0) const;
    void printsReadingTimeMessage(double t0, double t1) const;
    void printsReadingTimeMessage(double t0, double t1, Vector offset, Vector count) const;
    void printsWritingTimeMessage(double t0, double t1, Vector offset, Vector count) const;

    hid_t plist = 0;
    hid_t plist_DATASET_XFER = 0;

    // Block reading
    hsize_t numberOfBlocks = 0;
    Vector numberOfBlocksInDims;
    hsize_t lastBlockCount = 0;
    Vector blockDims;
    Vector lastBlockDims;

    //Vector *offsets;
    //Vector *counts;
    hsize_t numberOfElementsToLoad = 0;
    hsize_t realNumberOfElementsToLoad = 0;

    hid_t dataset = 0;
    hid_t dataspace = 0;
    hid_t datatype = 0;

    int rank = 0;
    Vector dims;
    Vector chunkDims;

    hsize_t minVI = 0;
    hsize_t maxVI = 0;

    float minVF = 0;
    float maxVF = 0;

    hsize_t minVIndex = 0;
    hsize_t maxVIndex = 0;

    bool issetGlobalMinAndMaxValue = false;
};

/// Vector of datasets datatype
typedef std::vector<Dataset *> VectorOfDatasets;
/// Map of datasets datatype
typedef std::map<std::string, Dataset *> MapOfDatasets;
/// Iterator for map of datasets datatype
typedef MapOfDatasets::iterator MapOfDatasetsIt;
/// Pair of datasets datatype
typedef std::pair<std::string, Dataset *> PairOfDatasets;
}

#endif // DATASET_H
