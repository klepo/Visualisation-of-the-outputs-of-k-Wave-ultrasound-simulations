/**
 * @file        dataset.cpp
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        30 July      2014 (created) <br>
 *              27 March     2019 (updated)
 *
 * @brief       The implementation file containing H5Helper::Dataset class definition.
 *
 * This class is used for better work with k-Wave HDF5 datasets (e.g read, write).
 *
 * @license     This file is part of the k-wave-h5-helper library for processing the HDF5 data
 *              created by the k-Wave toolbox - http://www.k-wave.org. This file may be used,
 *              distributed and modified under the terms of the LGPL version 3 open source
 *              license. A copy of the LGPL license should have been received with this file.
 *              Otherwise, it can be found at: http://www.gnu.org/copyleft/lesser.html.
 *
 * @copyright   Copyright © 2019, Petr Kleparnik, VUT FIT Brno. All Rights Reserved.
 *
 */

#include "dataset.h"

namespace H5Helper {

/**
 * @brief Creates Dataset object with given file, name and dataset
 * @param[in] datasetId Dataset id
 * @param[in] name Name of dataset
 * @param[in] file HDF5 File
 * @throw std::runtime_error
 */
Dataset::Dataset(hid_t datasetId, std::string name, File *file)
    : Object(datasetId, name, file)
      , datasetId(datasetId)
{
    // Init space
    dataspaceId = H5Dget_space(datasetId);
    if (dataspaceId < 0) {
        throw std::runtime_error("H5Dget_space error");
    }

    // Get type
    datatypeId = H5Dget_type(datasetId);
    if (datatypeId < 0) {
        throw std::runtime_error("H5Dget_type error");
    }

    // Check data type of dataset
    if (!H5Tequal(datatypeId, H5T_NATIVE_FLOAT) && !H5Tequal(datatypeId, H5T_NATIVE_UINT64)) {
        throw std::runtime_error("Wrong data type of dataset");
    }

    // Get rank, dims and chunk dims
    rank = H5Sget_simple_extent_ndims(dataspaceId);
    if (rank < 0) {
        throw std::runtime_error("H5Sget_simple_extent_ndims error");
    }

    dims = Vector(rank);
    chunkDims = Vector(rank);

    int dimsCount = H5Sget_simple_extent_dims(dataspaceId, dims.getVectorPtr(), nullptr);
    if (dimsCount < 0) {
        throw std::runtime_error("H5Sget_simple_extent_dims error");
    }

    pListId = H5Dget_create_plist(datasetId);
    if (pListId < 0) {
        throw std::runtime_error("H5Dget_create_plist error");
    }

    if (H5D_CHUNKED == H5Pget_layout(pListId)) {
        int chunkCount = H5Pget_chunk(pListId, int(dims.getLength()), chunkDims.getVectorPtr());
        if (chunkCount < 0) {
            throw std::runtime_error("H5Pget_chunk error");
        }
    }

    err = H5Pclose(pListId);
    if (err < 0) {
        throw std::runtime_error("H5Pclose error");
    }

    pListDatasetXferId = H5Pcreate(H5P_DATASET_XFER);
    if (pListDatasetXferId < 0) {
        throw std::runtime_error("H5Pcreate error");
    }

    // Init min/max
    maxVF = std::numeric_limits<float>::min();
    minVF = std::numeric_limits<float>::max();
    maxVI = std::numeric_limits<hsize_t>::min();
    minVI = std::numeric_limits<hsize_t>::max();

    // Init some flags for block reading
    //offsets = 0;
    //counts = 0;
    setNumberOfElmsToLoad(file->getNumberOfElmsToLoad());

    // Min/max flag
    issetGlobalMinAndMaxValue = false;
}

/**
 * @brief Destructor of Dataset object
 *
 * Closes all dataset related HDF5 objects.
 */
Dataset::~Dataset()
{
    Helper::printDebugMsgStart("Closing dataset \"" + getName() + "\"");
    err = H5Pclose(pListDatasetXferId);
    if (err < 0) {
        //throw std::runtime_error("H5Pclose error");
    }
    err = H5Sclose(dataspaceId);
    if (err < 0) {
        //throw std::runtime_error("H5Sclose error");
    }
    err = H5Tclose(datatypeId);
    if (err < 0) {
        //throw std::runtime_error("H5Tclose error");
    }
    err = H5Dclose(datasetId);
    if (err < 0) {
        //throw std::runtime_error("H5Dclose error");
    }
    Helper::printDebugMsgEnd("OK");
}

/**
 * @brief Returns dataset id
 * @return Dataset id
 */
hid_t Dataset::getId() const
{
    return datasetId;
}

/**
 * @brief Returns dataset rank
 * @return Dataset rank
 */
hsize_t Dataset::getRank() const
{
    return dims.getLength();
}

/**
 * @brief Returns dataset dimensions
 * @return Dataset dimensions
 */
Vector Dataset::getDims() const
{
    return dims;
}

/**
 * @brief Returns dataset chunk dimensions
 * @return Dataset chunk dimensions
 */
Vector Dataset::getChunkDims() const
{
    return chunkDims;
}

/**
 * @brief Returns dataset size
 * @return Dataset size
 */
hsize_t Dataset::getSize() const
{
    return dims.getSize();
}

/**
 * @brief Returns dataset datatype class
 * @return Dataset datatype class
 */
H5T_class_t Dataset::getDataTypeClass() const
{
    return H5Tget_class(datatypeId);
}

/**
 * @brief Returns dataset datatype id
 * @return Dataset datatype id
 */
hid_t Dataset::getDataType() const
{
    return datatypeId;
}

/**
 * @brief Returns dataset type
 * @param[in] sensorMaskSize Sensor mask size (optional)
 * @return Dataset type
 */
DatasetType Dataset::getType(hsize_t sensorMaskSize) const
{
    Helper::setDebugFlagAndStoreLast(false);
    Vector4D nDims = getFile()->getNDims();
    std::string name = getName();
    DatasetType type = DatasetType::UNKNOWN;
    if (getDims().getLength() == 3) { // 3D type
        Vector3D dims = getDims();
        if (H5Tequal(datatypeId, H5T_NATIVE_UINT64)) {
            if (dims == Vector3D(1, 1, 1)) {
                if (getOnlyName() == NT_DATASET) {
                    type = DatasetType::N_DIM_T;
                } else if (getOnlyName() == NX_DATASET) {
                    type = DatasetType::N_DIM_X;
                } else if (getOnlyName() == NY_DATASET) {
                    type = DatasetType::N_DIM_Y;
                } else if (getOnlyName() == NZ_DATASET) {
                    type = DatasetType::N_DIM_Z;
                }
            } else if (getOnlyName() == SENSOR_MASK_INDEX_DATASET) {
                type = DatasetType::MASK_INDEX;
            } else if (getOnlyName() == SENSOR_MASK_CORNERS_DATASET) {
                type = DatasetType::MASK_CORNERS;
            }
        } else if (H5Tequal(datatypeId, H5T_NATIVE_FLOAT)) {
            if (dims == Vector3D(1, 1, 1)) {
                if (getOnlyName() == DT_DATASET) {
                    type = DatasetType::DT;
                } else if (getOnlyName() == DX_DATASET) {
                    type = DatasetType::DX;
                } else if (getOnlyName() == DY_DATASET) {
                    type = DatasetType::DY;
                } else if (getOnlyName() == DZ_DATASET) {
                    type = DatasetType::DZ;
                }
            } else if (getOnlyName() == P_SOURCE_INPUT_DATASET) {
                type = DatasetType::P_SOURCE_INPUT;
            } else if (hasAttribute(POSITION_X_ATTR)
                && hasAttribute(POSITION_Y_ATTR)
                && hasAttribute(POSITION_Z_ATTR)
                && hasAttribute(SRC_POSITION_X_ATTR)
                && hasAttribute(SRC_POSITION_Y_ATTR)
                && hasAttribute(SRC_POSITION_Z_ATTR)
                && hasAttribute(SRC_SIZE_X_ATTR)
                && hasAttribute(SRC_SIZE_Y_ATTR)
                && hasAttribute(SRC_SIZE_Z_ATTR)
                && hasAttribute(SRC_DATASET_NAME_ATTR)
                ) {
                type = DatasetType::RESHAPED_3D_DWNSMPL;
            } else if (hasAttribute(POSITION_X_ATTR)
                && hasAttribute(POSITION_Y_ATTR)
                && hasAttribute(POSITION_Z_ATTR)
                && std::count(name.begin(), name.end(), '/') == 1
                ) {
                type = DatasetType::RESHAPED_3D;
            } else if (dims.z() < nDims.z()
                && dims.y() < nDims.y()
                && dims.x() < nDims.x()
                && hasAttribute(SRC_SIZE_X_ATTR)
                && hasAttribute(SRC_SIZE_Y_ATTR)
                && hasAttribute(SRC_SIZE_Z_ATTR)
                && hasAttribute(SRC_DATASET_NAME_ATTR)
                ) {
                type = DatasetType::BASIC_3D_DWNSMPL;
            } else if (dims.z() == nDims.z()
                && dims.y() == nDims.y()
                && dims.x() == nDims.x()
                ) {
                type = DatasetType::BASIC_3D;
            } else if (dims.z() <= nDims.z()
                && dims.y() <= nDims.y()
                && dims.x() <= nDims.x()
                && std::count(name.begin(), name.end(), '/') > 1
                ) {
                type = DatasetType::BASIC_CUBOID;
            } else if (dims.z() == 1
                && dims.y() == 1
                && dims.x() == sensorMaskSize
                && !hasAttribute(SRC_DATASET_NAME_ATTR)
                ) {
                type = DatasetType::BASIC_INDEX;
            } else if (dims.z() == 1
                && dims.y() <= nDims.w()
                && dims.x() == sensorMaskSize
                && !hasAttribute(SRC_DATASET_NAME_ATTR)
                ) {
                type = DatasetType::TIME_STEPS_INDEX;
            } else if (dims.z() == 1
                //&& dims.y() <= nDims.w()
                //&& dims.x() == sensorMaskSize
                && hasAttribute(C_PERIOD_ATTR)
                && hasAttribute(C_TYPE_ATTR)
                && readAttributeS(C_TYPE_ATTR) == "c"
                ) {
                type = DatasetType::TIME_STEPS_C_INDEX;
            } else if (dims.z() == 1
                //&& dims.y() <= nDims.w()
                //&& dims.x() == sensorMaskSize
                && hasAttribute(C_TYPE_ATTR)
                && readAttributeS(C_TYPE_ATTR) == "d"
                ) {
                type = DatasetType::TIME_STEPS_D_INDEX;
            } else if (dims.z() == 1
                && dims.y() <= nDims.w()
                && dims.x() == sensorMaskSize
                && hasAttribute(C_TYPE_ATTR)
                && readAttributeS(C_TYPE_ATTR) == "s"
                ) {
                type = DatasetType::TIME_STEPS_S_INDEX;
            }
        }
    } else if (getDims().getLength() == 4) { // 4D type (cuboids)
        if (H5Tequal(datatypeId, H5T_NATIVE_FLOAT)) {
            // Downsampled
            if (hasAttribute(SRC_SIZE_X_ATTR)
                && hasAttribute(SRC_SIZE_Y_ATTR)
                && hasAttribute(SRC_SIZE_Z_ATTR)
                && hasAttribute(SRC_DATASET_NAME_ATTR)
                ) {
                // With position attributes
                if (hasAttribute(POSITION_X_ATTR)
                    && hasAttribute(POSITION_Y_ATTR)
                    && hasAttribute(POSITION_Z_ATTR)
                    && hasAttribute(SRC_POSITION_X_ATTR)
                    && hasAttribute(SRC_POSITION_Y_ATTR)
                    && hasAttribute(SRC_POSITION_Z_ATTR)
                    ) {
                    if (hasAttribute(C_TYPE_ATTR)
                        && hasAttribute(C_PERIOD_ATTR)
                        && readAttributeS(C_TYPE_ATTR) == "c"
                        ) {
                        type = DatasetType::CUBOID_ATTR_DWNSMPL_C;
                    } else if (hasAttribute(C_TYPE_ATTR)
                               && readAttributeS(C_TYPE_ATTR) == "d"
                               ) {
                        type = DatasetType::CUBOID_ATTR_DWNSMPL_D;
                    } else if (hasAttribute(C_TYPE_ATTR)
                               && readAttributeS(C_TYPE_ATTR) == "s"
                               ) {
                        type = DatasetType::CUBOID_ATTR_DWNSMPL_S;
                    } else {
                        type = DatasetType::CUBOID_ATTR_DWNSMPL;
                    }
                } else { // Without position attributes
                    if (hasAttribute(C_TYPE_ATTR)
                        && hasAttribute(C_PERIOD_ATTR)
                        && readAttributeS(C_TYPE_ATTR) == "c"
                        ) {
                        type = DatasetType::CUBOID_DWNSMPL_C;
                    } else if (hasAttribute(C_TYPE_ATTR)
                               && readAttributeS(C_TYPE_ATTR) == "d"
                               ) {
                        type = DatasetType::CUBOID_DWNSMPL_D;
                    } else if (hasAttribute(C_TYPE_ATTR)
                               && readAttributeS(C_TYPE_ATTR) == "s"
                               ) {
                        type = DatasetType::CUBOID_DWNSMPL_S;
                    } else {
                        type = DatasetType::CUBOID_DWNSMPL;
                    }
                }
            } else { // Original
                // With position attributes
                if (hasAttribute(POSITION_X_ATTR)
                    && hasAttribute(POSITION_Y_ATTR)
                    && hasAttribute(POSITION_Z_ATTR)
                    ) {
                    if (hasAttribute(C_TYPE_ATTR)
                        && hasAttribute(C_PERIOD_ATTR)
                        && readAttributeS(C_TYPE_ATTR) == "c"
                        ) {
                        type = DatasetType::CUBOID_ATTR_C;
                    } else if (hasAttribute(C_TYPE_ATTR)
                               && readAttributeS(C_TYPE_ATTR) == "d"
                               ) {
                        type = DatasetType::CUBOID_ATTR_D;
                    } else if (hasAttribute(C_TYPE_ATTR)
                               && readAttributeS(C_TYPE_ATTR) == "s"
                               ) {
                        type = DatasetType::CUBOID_ATTR_S;
                    } else {
                        type = DatasetType::CUBOID_ATTR;
                    }
                } else { // Without position attributes
                    if (hasAttribute(C_TYPE_ATTR)
                        && hasAttribute(C_PERIOD_ATTR)
                        && readAttributeS(C_TYPE_ATTR) == "c"
                        ) {
                        type = DatasetType::CUBOID_C;
                    } else if (hasAttribute(C_TYPE_ATTR)
                               && readAttributeS(C_TYPE_ATTR) == "d"
                               ) {
                        type = DatasetType::CUBOID_D;
                    } else if (hasAttribute(C_TYPE_ATTR)
                               && readAttributeS(C_TYPE_ATTR) == "s"
                               ) {
                        type = DatasetType::CUBOID_S;
                    } else {
                        type = DatasetType::CUBOID;
                    }
                }
            }
        }
    }
    Helper::recoverLastDebugFlag();
    return type;
}

/**
 * @brief Returns this dataset type as string
 * @return This dataset type as string
 */
std::string Dataset::getTypeString() const
{
    return getTypeString(this->getType());
}

/**
 * @brief Returns dataset type as string
 * @param[in] type Dataset type
 * @return Dataset type as string
 */
std::string Dataset::getTypeString(DatasetType type) const
{
    switch (type) {
        case DatasetType::ALL:
            return "General k-Wave dataset";
        case DatasetType::UNKNOWN:
            return "Unknown";
        case DatasetType::N_DIM_X:
            return "Nx";
        case DatasetType::N_DIM_Y:
            return "Ny";
        case DatasetType::N_DIM_Z:
            return "Nz";
        case DatasetType::N_DIM_T:
            return "Nt";
        case DatasetType::DX:
            return "dx";
        case DatasetType::DY:
            return "dy";
        case DatasetType::DZ:
            return "dz";
        case DatasetType::DT:
            return "dt";
        case DatasetType::MASK_INDEX:
            return "Sensor mask index";
        case DatasetType::MASK_CORNERS:
            return "Sensor mask corners";
        case DatasetType::P_SOURCE_INPUT:
            return "P source input";
        case DatasetType::BASIC_3D:
            return "3D";
        case DatasetType::BASIC_CUBOID:
            return "3D cuboid";
        case DatasetType::RESHAPED_3D:
            return "3D (reshaped index)";
        case DatasetType::BASIC_3D_DWNSMPL:
            return "3D (downsampled)";
        case DatasetType::RESHAPED_3D_DWNSMPL:
            return "3D (downsampled reshaped index)";
        case DatasetType::BASIC_INDEX:
            return "Basic index";
        case DatasetType::TIME_STEPS_INDEX:
            return "Time steps index (time steps)";
        case DatasetType::TIME_STEPS_C_INDEX:
            return "Time steps index (compressed)";
        case DatasetType::TIME_STEPS_D_INDEX:
            return "Time steps index (decompressed)";
        case DatasetType::TIME_STEPS_S_INDEX:
            return "Time steps index (difference)";
        case DatasetType::CUBOID:
            return "Cuboid";
        case DatasetType::CUBOID_C:
            return "Cuboid (compressed)";
        case DatasetType::CUBOID_D:
            return "Cuboid (decompressed)";
        case DatasetType::CUBOID_S:
            return "Cuboid (difference)";
        case DatasetType::CUBOID_DWNSMPL:
            return "Cuboid (downsampled";
        case DatasetType::CUBOID_DWNSMPL_C:
            return "Cuboid (downsampled compressed)";
        case DatasetType::CUBOID_DWNSMPL_D:
            return "Cuboid (downsampled decompressed)";
        case DatasetType::CUBOID_DWNSMPL_S:
            return "Cuboid (downsampled difference)";
        case DatasetType::CUBOID_ATTR:
            return "Cuboid with attributes";
        case DatasetType::CUBOID_ATTR_C:
            return "Cuboid with attributes (compressed)";
        case DatasetType::CUBOID_ATTR_D:
            return "Cuboid with attributes (decompressed)";
        case DatasetType::CUBOID_ATTR_S:
            return "Cuboid with attributes (difference)";
        case DatasetType::CUBOID_ATTR_DWNSMPL:
            return "Cuboid with attributes (downsampled)";
        case DatasetType::CUBOID_ATTR_DWNSMPL_C:
            return "Cuboid with attributes (downsampled compressed)";
        case DatasetType::CUBOID_ATTR_DWNSMPL_D:
            return "Cuboid with attributes (downsampled decompressed)";
        case DatasetType::CUBOID_ATTR_DWNSMPL_S:
            return "Cuboid with attributes (downsampled difference)";
    }
    return "Unknown";
}

/**
 * @brief Is float type?
 * @return True/False
 */
bool Dataset::isFloatType() const
{
    return H5Tequal(datatypeId, H5T_NATIVE_FLOAT) != 0;
}

/**
 * @brief Is 64-bit unsigned integer type?
 * @return True/False
 */
bool Dataset::isIntegerType() const
{
    return H5Tequal(datatypeId, H5T_NATIVE_UINT64) != 0;
}

/**
 * @brief Returns global maximal 64-bit unsigned integer value
 * @param[out] value Global maximal 64-bit unsigned integer value
 * @param[out] maxVIndex Index of maximal value
 * @param[in] reset Reset flag for finding the value in dataset (optional)
 */
void Dataset::getGlobalMaxValue(hsize_t &value, hsize_t &maxVIndex, bool reset)
{
    checkIntegerType();
    if (issetGlobalMinAndMaxValue != true)
        findGlobalMinAndMaxValue(reset);
    value = maxVI;
    maxVIndex = this->maxVIndex;
}

/**
 * @brief Returns global minimal 64-bit unsigned integer value
 * @param[out] value Global minimal 64-bit unsigned integer value
 * @param[out] minVIndex Index of minimal value
 * @param[in] reset Reset flag for finding the value in dataset (optional)
 */
void Dataset::getGlobalMinValue(hsize_t &value, hsize_t &minVIndex, bool reset)
{
    checkIntegerType();
    if (issetGlobalMinAndMaxValue != true)
        findGlobalMinAndMaxValue(reset);
    value = minVI;
    minVIndex = this->minVIndex;
}

/**
 * @brief Returns global maximal float value
 * @param[out] value Global maximal float value
 * @param[out] maxVIndex Index of maximal value
 * @param[in] reset Reset flag for finding the value in dataset (optional)
 */
void Dataset::getGlobalMaxValue(float &value, hsize_t &maxVIndex, bool reset)
{
    checkFloatType();
    if (issetGlobalMinAndMaxValue != true)
        findGlobalMinAndMaxValue(reset);
    value = maxVF;
    maxVIndex = this->maxVIndex;
}

/**
 * @brief Returns global minimal float value
 * @param[out] value Global minimal float value
 * @param[out] minVIndex Index of minimal value
 * @param[in] reset Reset flag for finding the value in dataset (optional)
 */
void Dataset::getGlobalMinValue(float &value, hsize_t &minVIndex, bool reset)
{
    checkFloatType();
    if (issetGlobalMinAndMaxValue != true)
        findGlobalMinAndMaxValue(reset);
    value = minVF;
    minVIndex = this->minVIndex;
}

/**
 * @brief Finds and sets global minimal and maximal value
 * @param[in] reset Reset flag for searching the values in dataset (optional)
 *
 * The values are loaded from dataset attributes or searched from dataset data
 * and are stored to this object.
 */
void Dataset::findAndSetGlobalMinAndMaxValue(bool reset)
{
    if (isFloatType()) {
        if (reset || (!this->hasAttribute(MIN_ATTR)
                      || !this->hasAttribute(MAX_ATTR)
                      || !this->hasAttribute(MIN_INDEX_ATTR)
                      || !this->hasAttribute(MAX_INDEX_ATTR))
            ) {
            Helper::printDebugMsg("Finding min/max value");
            Dataset::findGlobalMinAndMaxValueF();
            Dataset::setAttribute(MIN_ATTR, minVF);
            Dataset::setAttribute(MAX_ATTR, maxVF);
            Dataset::setAttribute(MIN_INDEX_ATTR, minVIndex);
            Dataset::setAttribute(MAX_INDEX_ATTR, maxVIndex);
            Helper::printDebugMsgStart("Finding min/max value");
            Helper::printDebugMsgEnd("OK");
        } else {
            Helper::printDebugMsg("Reading min/max value");
            minVF = Dataset::readAttributeF(MIN_ATTR);
            maxVF = Dataset::readAttributeF(MAX_ATTR);
            minVIndex = Dataset::readAttributeI(MIN_INDEX_ATTR);
            maxVIndex = Dataset::readAttributeI(MAX_INDEX_ATTR);
            issetGlobalMinAndMaxValue = true;
            Helper::printDebugMsgStart("Finding min/max value");
            Helper::printDebugMsgEnd("OK");
        }
    } else {
        if (reset || (!this->hasAttribute(MIN_ATTR)
                      || !this->hasAttribute(MAX_ATTR)
                      || !this->hasAttribute(MIN_INDEX_ATTR)
                      || !this->hasAttribute(MAX_INDEX_ATTR))
            ) {
            Helper::printDebugMsg("Finding min/max value");
            Dataset::findGlobalMinAndMaxValueI();
            Dataset::setAttribute(MIN_ATTR, minVI);
            Dataset::setAttribute(MAX_ATTR, maxVI);
            Dataset::setAttribute(MIN_INDEX_ATTR, minVIndex);
            Dataset::setAttribute(MAX_INDEX_ATTR, maxVIndex);
            Helper::printDebugMsgStart("Finding min/max value");
            Helper::printDebugMsgEnd("OK");
        } else {
            Helper::printDebugMsg("Reading min/max value");
            minVI = Dataset::readAttributeI(MIN_ATTR);
            maxVI = Dataset::readAttributeI(MAX_ATTR);
            minVIndex = Dataset::readAttributeI(MIN_INDEX_ATTR);
            maxVIndex = Dataset::readAttributeI(MAX_INDEX_ATTR);
            issetGlobalMinAndMaxValue = true;
            Helper::printDebugMsgStart("Finding min/max value");
            Helper::printDebugMsgEnd("OK");
        }
    }
}

/**
 * @brief Returns real number of elements to load from dataset in block reading
 * @return Real number of elements to load from dataset
 */
hsize_t Dataset::getRealNumberOfElmsToLoad() const
{
    return realNumberOfElementsToLoad;
}

/**
 * @brief Returns number of blocks for block reading
 * @return Number of blocks for block reading
 */
hsize_t Dataset::getNumberOfBlocks() const
{
    return numberOfBlocks;
}

/**
 * @brief Returns number of blocks in dims for block reading
 * @return Number of blocks in dims for block reading
 */
Vector Dataset::getNumberOfBlocksInDims() const
{
    return numberOfBlocksInDims;
}

/**
 * @brief Returns general block dims of block reading
 * @return General block dims of block reading
 */
Vector Dataset::getGeneralBlockDims() const
{
    return getBlockDims(0);
}

/**
 * @brief Returns number of elements to load for block reading
 * @return Number of elements to load for block reading
 */
hsize_t Dataset::getNumberOfElmsToLoad() const
{
    return numberOfElementsToLoad;
}

/**
 * @brief Sets number of elements to load for block reading
 * @param[in] count Number of elements to load for block reading
 */
void Dataset::setNumberOfElmsToLoad(hsize_t count)
{
#ifdef PARALLEL_HDF5
    if (file->getMPISize() > 1 && size > std::numeric_limits<int>::max())
        throw std::runtime_error("setNumberOfElmsToLoad error");
#endif
    if (count < 1)
        throw std::runtime_error("setNumberOfElmsToLoad error");
    numberOfElementsToLoad = count;
    // numberOfElementsToLoad of dataset is limited by dataset size
    if (dims.getSize() <= numberOfElementsToLoad) {
        numberOfElementsToLoad = dims.getSize();
    }
    initBlockReading();
}

/**
 * @brief Sets MPIO access type
 * @param[in] type MPIO access type
 * @throw std::runtime_error
 */
void Dataset::setMPIOAccess(H5FD_mpio_xfer_t type)
{
    if (type == H5FD_MPIO_COLLECTIVE) {
        Helper::printDebugMsg("Setting H5FD_MPIO_COLLECTIVE access (" + getName() + ")");
    } else if (type == H5FD_MPIO_INDEPENDENT) {
        Helper::printDebugMsg("Setting H5FD_MPIO_INDEPENDENT access (" + getName() + ")");
    } else {
        throw std::runtime_error("H5Pset_dxpl_mpio error - Wrong MPIO type");
    }
#ifdef PARALLEL_HDF5
    err = H5Pset_dxpl_mpio(plist_DATASET_XFER, type);
    if (err < 0) {
        throw std::runtime_error("H5Pset_dxpl_mpio error");
    }
#endif
}

/**
 * @brief Reads dataset
 * @param[in] offset Data offset
 * @param[in] count Data count
 * @param[out] data Output data
 * @param[out] min Minimal value
 * @param[out] max Maximal value
 * @param[out] minIndex Index of minimal value
 * @param[out] maxIndex Index of maximal value
 * @param[in] block Index of block for block reading (optional)
 */
void Dataset::readDataset(Vector offset, Vector count, float *&data, float &min, float &max, hsize_t &minIndex, hsize_t &maxIndex, hsize_t block)
{
    printsReadingMessage(block);
    checkDataTypeAndAllocation(data, H5T_NATIVE_FLOAT, count.getSize());
    readDatasetGeneral(offset, count, data);
    Dataset::findMinAndMaxValue(data, count.getSize(), min, max, minIndex, maxIndex);
}

/**
 * @brief Reads dataset
 * @param[in] offset Data offset
 * @param[in] count Data count
 * @param[out] data Output data
 * @param[out] min Minimal value
 * @param[out] max Maximal value
 * @param[out] minIndex Index of minimal value
 * @param[out] maxIndex Index of maximal value
 * @param[in] block Index of block for block reading (optional)
 */
void Dataset::readDataset(Vector offset, Vector count, hsize_t *&data, hsize_t &min, hsize_t &max, hsize_t &minIndex, hsize_t &maxIndex, hsize_t block)
{
    printsReadingMessage(block);
    checkDataTypeAndAllocation(data, H5T_NATIVE_UINT64, count.getSize());
    readDatasetGeneral(offset, count, data);
    Dataset::findMinAndMaxValue(data, count.getSize(), min, max, minIndex, maxIndex);
}

/**
 * @brief Reads dataset
 * @param[in] offset Data offset
 * @param[in] count Data count
 * @param[out] data Output data
 * @param[in] block Index of block for block reading (optional)
 */
void Dataset::readDataset(Vector offset, Vector count, float *&data, hsize_t block)
{
    printsReadingMessage(block);
    checkDataTypeAndAllocation(data, H5T_NATIVE_FLOAT, count.getSize());
    readDatasetGeneral(offset, count, data);
}

/**
 * @brief Reads dataset
 * @param[in] offset Data offset
 * @param[in] count Data count
 * @param[out] data Output data
 * @param[in] block Index of block for block reading (optional)
 */
void Dataset::readDataset(Vector offset, Vector count, hsize_t *&data, hsize_t block)
{
    printsReadingMessage(block);
    checkDataTypeAndAllocation(data, H5T_NATIVE_UINT64, count.getSize());
    readDatasetGeneral(offset, count, data);
}

/**
 * @brief Reads dataset
 * @param[out] data Output data
 */
void Dataset::readDataset(float *&data)
{
    readDataset(Vector(dims.getLength(), 0), dims, data);
}

/**
 * @brief Reads dataset
 * @param[out] data Output data
 */
void Dataset::readDataset(hsize_t *&data)
{
    readDataset(Vector(dims.getLength(), 0), dims, data);
}

/**
 * @brief Reads dataset
 * @param[out] data Output data
 * @param[out] min Minimal value
 * @param[out] max Maximal value
 * @param[out] minIndex Index of minimal value
 * @param[out] maxIndex Index of maximal value
 */
void Dataset::readDataset(float *&data, float &min, float &max, hsize_t &minIndex, hsize_t &maxIndex)
{
    readDataset(Vector(dims.getLength(), 0), dims, data, min, max, minIndex, maxIndex);
}

/**
 * @brief Reads dataset
 * @param[out] data Output data
 * @param[out] min Minimal value
 * @param[out] max Maximal value
 * @param[out] minIndex Index of minimal value
 * @param[out] maxIndex Index of maximal value
 */
void Dataset::readDataset(hsize_t *&data, hsize_t &min, hsize_t &max, hsize_t &minIndex, hsize_t &maxIndex)
{
    readDataset(Vector(dims.getLength(), 0), dims, data, min, max, minIndex, maxIndex);
}

/**
 * @brief Reads dataset of size 1
 * @param[out] data Output data
 * @throw std::runtime_error
 */
void Dataset::readDataset(float &data)
{
    if (dims.getSize() == 1) {
        float *dataTmp = nullptr;
        readDataset(Vector(dims.getLength(), 0), dims, dataTmp);
        data = dataTmp[0];
        if (dataTmp) {
            delete[] dataTmp;
            dataTmp = nullptr;
        }
    } else {
        throw std::runtime_error("Dataset has not size 1");
    }
}

/**
 * @brief Reads dataset of size 1
 * @param[out] data Output data
 * @throw std::runtime_error
 */
void Dataset::readDataset(hsize_t &data)
{
    if (dims.getSize() == 1) {
        hsize_t *dataTmp = nullptr;
        readDataset(Vector(dims.getLength(), 0), dims, dataTmp);
        data = dataTmp[0];
        if (dataTmp) {
            delete[] dataTmp;
            dataTmp = nullptr;
        }
    } else {
        throw std::runtime_error("Dataset has not size 1");
    }
}

/**
 * @brief Writes data to the dataset
 * @param[in] offset Data offset
 * @param[in] count Data count
 * @param[in] data Data to write
 */
void Dataset::writeDataset(Vector offset, Vector count, const float *data, hsize_t block)
{
    printsWritingMessage(block);
    checkFloatType();
    writeDatasetGeneral(offset, count, static_cast<const void *>(data));
}

/**
 * @brief Writes data to the dataset
 * @param[in] offset Data offset
 * @param[in] count Data count
 * @param[in] data Data to write
 */
void Dataset::writeDataset(Vector offset, Vector count, const hsize_t *data, hsize_t block)
{
    printsWritingMessage(block);
    checkIntegerType();
    writeDatasetGeneral(offset, count, static_cast<const void *>(data));
}

/**
 * @brief Writes data to the dataset
 * @param[in] data Data to write
 */
void Dataset::writeDataset(const float *data)
{
    writeDataset(Vector(dims.getLength(), 0), dims, data);
}

/**
 * @brief Writes data to the dataset
 * @param[in] data Data to write
 */
void Dataset::writeDataset(const hsize_t *data)
{
    writeDataset(Vector(dims.getLength(), 0), dims, data);
}

/**
 * @brief Reads block of dataset
 * @param[in] index Block index
 * @param[out] offset Data offset
 * @param[out] count Data count
 * @param[out] data Output data
 * @param[out] min Minimal value
 * @param[out] max Maximal value
 * @param[out] minIndex Index of minimal value
 * @param[out] maxIndex Index of maximal value
 */
void Dataset::readBlock(hsize_t index, Vector &offset, Vector &count, float *&data, float &min, float &max, hsize_t &minIndex, hsize_t &maxIndex)
{
    readDataset(getBlockOffset(index), getBlockDims(index), data, min, max, minIndex, maxIndex, index + 1);
    offset = getBlockOffset(index);
    count = getBlockDims(index);
}

/**
 * @brief Reads block of dataset
 * @param[in] index Block index
 * @param[out] offset Data offset
 * @param[out] count Data count
 * @param[out] data Output data
 * @param[out] min Minimal value
 * @param[out] max Maximal value
 * @param[out] minIndex Index of minimal value
 * @param[out] maxIndex Index of maximal value
 */
void Dataset::readBlock(hsize_t index, Vector &offset, Vector &count, hsize_t *&data, hsize_t &min, hsize_t &max, hsize_t &minIndex, hsize_t &maxIndex)
{
    readDataset(getBlockOffset(index), getBlockDims(index), data, min, max, minIndex, maxIndex, index + 1);
    offset = getBlockOffset(index);
    count = getBlockDims(index);
}

/**
 * @brief Reads block of dataset
 * @param[in] index Block index
 * @param[out] offset Data offset
 * @param[out] count Data count
 * @param[out] data Output data
 */
void Dataset::readBlock(hsize_t index, Vector &offset, Vector &count, float *&data)
{
    readDataset(getBlockOffset(index), getBlockDims(index), data, index + 1);
    offset = getBlockOffset(index);
    count = getBlockDims(index);
}

/**
 * @brief Reads block of dataset
 * @param[in] index Block index
 * @param[out] offset Data offset
 * @param[out] count Data count
 * @param[out] data Output data
 */
void Dataset::readBlock(hsize_t index, Vector &offset, Vector &count, hsize_t *&data)
{
    readDataset(getBlockOffset(index), getBlockDims(index), data, index + 1);
    offset = getBlockOffset(index);
    count = getBlockDims(index);
}

/**
 * @brief Reads empty block
 * @throw std::runtime_error
 */
void Dataset::readEmptyBlock()
{
    hid_t dataspaceId = H5Dget_space(datasetId);
    H5Sselect_none(dataspaceId);
    Vector3D count;
    hid_t memspaceId = H5Screate_simple(3, count.getVectorPtr(), nullptr);
    H5Sselect_none(memspaceId);
    double t0 = 0, t1 = 0;
    t0 = getTime();
    Helper::printDebugMsg("Reading dataset \"" + getName() + "\"");
    err = H5Dread(datasetId, datatypeId, memspaceId, dataspaceId, pListDatasetXferId, nullptr);
    t1 = getTime();
    if (err < 0) {
        throw std::runtime_error("H5Dread error");
    }
    printsReadingTimeMessage(t0, t1);
}

/**
 * @brief Reads general dataset
 * @param[in] offset Data offset
 * @param[in] count Data count
 * @param[out] data Output data
 * @throw std::runtime_error
 */
void Dataset::readDatasetGeneral(Vector offset, Vector count, void *data)
{
    Dataset::checkOffsetAndCountParams(offset, count);
    Vector mem_offset(offset.getLength());

    hid_t dataspaceId = H5Dget_space(datasetId);
    err = H5Sselect_hyperslab(dataspaceId, H5S_SELECT_SET, offset.getVectorPtr(), nullptr, count.getVectorPtr(), nullptr);
    if (err < 0) {
        throw std::runtime_error("H5Sselect_hyperslab error");
    }
    hid_t memspaceId = H5Screate_simple(int(count.getLength()), count.getVectorPtr(), nullptr);
    if (memspaceId < 0) {
        throw std::runtime_error("H5Screate_simple error");
    }

    err = H5Sselect_hyperslab(memspaceId, H5S_SELECT_SET, mem_offset.getVectorPtr(), nullptr, count.getVectorPtr(), nullptr);
    if (err < 0) {
        throw std::runtime_error("H5Sselect_hyperslab error");
    }

    double t0 = 0, t1 = 0;

    t0 = getTime();

    // Reading
    err = H5Dread(datasetId, datatypeId, memspaceId, dataspaceId, pListDatasetXferId, data);
    if (err < 0) {
        throw std::runtime_error("H5Dread error");
    }

    t1 = getTime();

    err = H5Sclose(dataspaceId);
    if (err < 0) {
        throw std::runtime_error("H5Sclose error");
    }
    err = H5Sclose(memspaceId);
    if (err < 0) {
        throw std::runtime_error("H5Sclose error");
    }

    printsReadingTimeMessage(t0, t1, offset, count);
}

/**
 * @brief Writes general dataset
 * @param[in] offset Data offset
 * @param[in] count Data count
 * @param[in] data Output data
 * @throw std::runtime_error
 */
void Dataset::writeDatasetGeneral(Vector offset, Vector count, const void *data)
{
    Dataset::checkOffsetAndCountParams(offset, count);
    Vector mem_offset(offset.getLength());

    hid_t dataspaceId = H5Dget_space(datasetId);
    err = H5Sselect_hyperslab(dataspaceId, H5S_SELECT_SET, offset.getVectorPtr(), nullptr, count.getVectorPtr(), nullptr);
    if (err < 0) {
        throw std::runtime_error("H5Sselect_hyperslab error");
    }

    hid_t memspaceId = H5Screate_simple(int(count.getLength()), count.getVectorPtr(), nullptr);
    if (memspaceId < 0) {
        throw std::runtime_error("H5Screate_simple error");
    }

    err = H5Sselect_hyperslab(memspaceId, H5S_SELECT_SET, mem_offset.getVectorPtr(), nullptr, count.getVectorPtr(), nullptr);
    if (err < 0) {
        throw std::runtime_error("H5Sselect_hyperslab error");
    }

    double t0 = 0, t1 = 0;

    t0 = getTime();

    // Writing
    err = H5Dwrite(datasetId, datatypeId, memspaceId, dataspaceId, pListDatasetXferId, data);
    if (err < 0) {
        throw std::runtime_error("H5Dwrite error");
    }

    t1 = getTime();

    err = H5Sclose(dataspaceId);
    if (err < 0) {
        throw std::runtime_error("H5Sclose error");
    }
    err = H5Sclose(memspaceId);
    if (err < 0) {
        throw std::runtime_error("H5Sclose error");
    }

    printsWritingTimeMessage(t0, t1, offset, count);
}

/**
 * @brief Checks offset and count parameters
 * @param[in] offset Data offset
 * @param[in] count Data count
 * @throw std::runtime_error
 */
void Dataset::checkOffsetAndCountParams(Vector offset, Vector count) const
{
    if ((dims.getLength() != offset.getLength()) || (dims.getLength() != count.getLength()) || count.getLength() != getRank()) {
        //std::cout << dims << " " << offset << " " << count;
        throw std::runtime_error("Wrong offset or count");
    }

    for (unsigned int i = 0; i < offset.getLength(); i++) {
        if (offset.at(i) >= dims.at(i))
            throw std::runtime_error("Wrong offset - too big offset of dimension " + std::to_string(i) + " (" + std::to_string(offset.at(i)) + " >= " + std::to_string(dims.at(i)) + ")");
        if (count.at(i) <= 0)
            throw std::runtime_error("Wrong count - too small count of dimension " + std::to_string(i) + " (" + std::to_string(count.at(i)) + " <= 0)");
        if (offset.at(i) + count.at(i) > dims.at(i))
            throw std::runtime_error("Wrong count - sum of offset and count of dimension " + std::to_string(i) + " is too big (" + std::to_string(offset.at(i)) + " + " + std::to_string(count.at(i)) + " > " + std::to_string(dims.at(i)) + ")");
    }
}

/**
 * @brief Finds minimal and maximal float value
 * @param[in] data Input data
 * @param[in] size Size of data
 * @param[out] minVF Minimal value
 * @param[out] maxVF Maximal value
 * @param[out] minVFIndex Index of minimal value
 * @param[out] maxVFIndex Index of maximal value
 */
void Dataset::findMinAndMaxValue(const float *data, hsize_t size, float &minVF, float &maxVF, hsize_t &minVFIndex, hsize_t &maxVFIndex) const
{
    minVF = std::numeric_limits<float>::max();
    maxVF = std::numeric_limits<float>::min();
    for (hsize_t i = 0; i < size; i++) {
        checkOrSetMinMaxValue(minVF, maxVF, data[i], minVFIndex, maxVFIndex, i);
    }
}

/**
 * @brief Finds minimal and maximal 64-bit unsigned integer value
 * @param[in] data Input data
 * @param[in] size Size of data
 * @param[out] minVI Minimal value
 * @param[out] maxVI Maximal value
 * @param[out] minVIIndex Index of minimal value
 * @param[out] maxVIIndex Index of maximal value
 */
void Dataset::findMinAndMaxValue(const hsize_t *data, hsize_t size, hsize_t &minVI, hsize_t &maxVI, hsize_t &minVIIndex, hsize_t &maxVIIndex) const
{
    minVI = std::numeric_limits<hsize_t>::max();
    maxVI = std::numeric_limits<hsize_t>::min();
    for (hsize_t i = 0; i < size; i++) {
        checkOrSetMinMaxValue(minVI, maxVI, data[i], minVIIndex, maxVIIndex, i);
    }
}

/**
 * @brief Finds global minimal and maximal value
 * @param[in] reset Reset flag for searching the values in dataset (optional)
 */
void Dataset::findGlobalMinAndMaxValue(bool reset)
{
    if (isFloatType()) {
        if (reset) {
            Dataset::findGlobalMinAndMaxValueF();
        } else {
            if (this->hasAttribute(MIN_ATTR) && this->hasAttribute(MAX_ATTR) && this->hasAttribute(MIN_INDEX_ATTR) && this->hasAttribute(MAX_INDEX_ATTR)) {
                minVF = Dataset::readAttributeF(MIN_ATTR);
                maxVF = Dataset::readAttributeF(MAX_ATTR);
                minVIndex = Dataset::readAttributeI(MIN_INDEX_ATTR);
                maxVIndex = Dataset::readAttributeI(MAX_INDEX_ATTR);
                issetGlobalMinAndMaxValue = true;
            } else {
                Dataset::findGlobalMinAndMaxValueF();
            }
        }
    } else {
        if (reset) {
            Dataset::findGlobalMinAndMaxValueI();
        } else {
            if (this->hasAttribute(MIN_ATTR) && this->hasAttribute(MAX_ATTR) && this->hasAttribute(MIN_INDEX_ATTR) && this->hasAttribute(MAX_INDEX_ATTR)) {
                minVI = Dataset::readAttributeI(MIN_ATTR);
                maxVI = Dataset::readAttributeI(MAX_ATTR);
                minVIndex = Dataset::readAttributeI(MIN_INDEX_ATTR);
                maxVIndex = Dataset::readAttributeI(MAX_INDEX_ATTR);
                issetGlobalMinAndMaxValue = true;
            } else {
                Dataset::findGlobalMinAndMaxValueI();
            }
        }
    }
}

/**
 * @brief Finds global minimal and maximal float value
 */
void Dataset::findGlobalMinAndMaxValueF()
{
    Vector offset;
    Vector count;
    float minVFTmp = std::numeric_limits<float>::max();
    float maxVFTmp = std::numeric_limits<float>::min();
    hsize_t minVIndexTmp = 0;
    hsize_t maxVIndexTmp = 0;
    hsize_t linearOffset = 0;
    float *data = new float[blockDims.getSize()]();
    for (hsize_t i = 0; i < numberOfBlocks; i++) {
        readBlock(i, offset, count, data, minVFTmp, maxVFTmp, minVIndexTmp, maxVIndexTmp);
        convertMultiDimToLinear(offset, linearOffset, dims);
        H5Helper::checkOrSetMinMaxValue(minVF, maxVF, minVFTmp, maxVFTmp, minVIndex, maxVIndex, linearOffset + minVIndexTmp, linearOffset + maxVIndexTmp);
    }
    issetGlobalMinAndMaxValue = true;
    if (data) {
        delete[] data; // !!!
        data = nullptr;
    }
}

/**
 * @brief Finds global minimal and maximal 64-bit unsigned integer value
 */
void Dataset::findGlobalMinAndMaxValueI()
{
    Vector offset;
    Vector count;
    hsize_t minVITmp = std::numeric_limits<hsize_t>::min();
    hsize_t maxVITmp = std::numeric_limits<hsize_t>::max();
    hsize_t minVIndexTmp = 0;
    hsize_t maxVIndexTmp = 0;
    hsize_t linearOffset = 0;
    hsize_t *data = new hsize_t[blockDims.getSize()]();
    for (hsize_t i = 0; i < numberOfBlocks; i++) {
        readBlock(i, offset, count, data, minVITmp, maxVITmp, minVIndexTmp, maxVIndexTmp);
        convertMultiDimToLinear(offset, linearOffset, dims);
        H5Helper::checkOrSetMinMaxValue(minVI, maxVI, minVITmp, maxVITmp, minVIndex, maxVIndex, linearOffset + minVIndexTmp, linearOffset + maxVIndexTmp);
    }
    issetGlobalMinAndMaxValue = true;
    if (data) {
        delete[] data; // !!!
        data = nullptr;
    }
}

/**
 * @brief Initializes block reading
 */
void Dataset::initBlockReading()
{
    hsize_t prod = 1;
    blockDims = Vector(dims.getLength(), 1);
    lastBlockDims = Vector(dims.getLength(), 1);
    numberOfBlocksInDims = dims;

    lastBlockCount = 1;
    bool diffSizeFlag = false;
    realNumberOfElementsToLoad = getNumberOfElmsToLoad();

    for (hsize_t i = dims.getLength(); i > 0; i--) {
        hsize_t j = i - 1;
        hsize_t newProd = prod * dims[j];
        if (newProd > getNumberOfElmsToLoad()) {
            blockDims[j] = getNumberOfElmsToLoad() / prod;
            lastBlockDims[j] = blockDims[j];
            numberOfBlocksInDims[j] = dims[j] / blockDims[j];
            lastBlockCount = numberOfBlocksInDims[j];

            if (dims[j] % blockDims[j]) {
                lastBlockDims[j] = dims[j] % blockDims[j];
                diffSizeFlag = true;
                lastBlockCount++;
            }
            realNumberOfElementsToLoad = prod * blockDims[j];
            numberOfBlocksInDims[j] = lastBlockCount;

            break;
        }
        prod = newProd;
        blockDims[j] = dims[j];
        lastBlockDims[j] = dims[j];
        numberOfBlocksInDims[j] = 1;
    }

    numberOfBlocks = 1;
    for (hsize_t i = 0; i < dims.getLength(); i++) {
        hsize_t p = dims[i] / blockDims[i];
        if (dims[i] % blockDims[i])
            p++;
        numberOfBlocks *= p;
    }
}

/**
 * @brief Returns block dimensions by index
 * @param[in] index Index
 * @return Block dimensions
 */
Vector Dataset::getBlockDims(hsize_t index) const
{
    if ((index + 1) % lastBlockCount == 0) {
        return lastBlockDims;
    } else {
        return blockDims;
    }
}

/**
 * @brief Returns block offset by index
 * @param[in] index Index
 * @return Block offset
 */
Vector Dataset::getBlockOffset(hsize_t index) const
{
    Vector offset2;
    hsize_t lastBlockDimsCount = (index) / lastBlockCount;
    hsize_t lastBlockDimsSize = lastBlockDimsCount * lastBlockDims.getSize();
    hsize_t sum2 = lastBlockDimsSize + (index - lastBlockDimsCount) * blockDims.getSize();
    convertlinearToMultiDim(sum2, offset2, dims);
    return offset2;
}

/**
 * @brief Checks data type and allocates memory
 * @param[out] data Output data - must be nullptr or allocated
 * @param[in] type
 * @param[in] size
 * @throw std::runtime_error
 */
void Dataset::checkDataTypeAndAllocation(hsize_t *&data, hid_t type, hsize_t size) const
{
    checkType(type);

    if (size > getNumberOfElmsToLoad())
        throw std::runtime_error(readErrorMessage(size, H5T_NATIVE_UINT64));

    try {
        if (data == nullptr)
            data = new hsize_t[size](); // TODO check available memory?
        if (data == nullptr)
            throw std::runtime_error("Bad memory allocation");
    } catch (std::bad_alloc) {
        throw std::runtime_error(memoryErrorMessage(size, H5T_NATIVE_UINT64));
    }
}

/**
 * @brief Checks data type and allocates memory
 * @param[out] data Output data - must be nullptr or allocated
 * @param[in] type
 * @param[in] size
 * @throw std::runtime_error
 */
void Dataset::checkDataTypeAndAllocation(float *&data, hid_t type, hsize_t size) const
{
    checkType(type);

    if (size > getNumberOfElmsToLoad())
        throw std::runtime_error(readErrorMessage(size, H5T_NATIVE_FLOAT));

    try {
        if (data == nullptr)
            data = new float[size](); // TODO check available memory?
        if (data == nullptr)
            throw std::runtime_error("Bad memory allocation");
    } catch (std::bad_alloc) {
        throw std::runtime_error(memoryErrorMessage(size, H5T_NATIVE_FLOAT));
    }
}

/**
 * @brief Checks datatype (H5T_NATIVE_FLOAT or H5T_NATIVE_UINT64)
 * @param[in] type Datatype
 */
void Dataset::checkType(hid_t type) const
{
    if (type == H5T_NATIVE_FLOAT) {
        if (!isFloatType())
            throw std::runtime_error("Wrong data type of dataset (not " + dataTypeString(type) + ")");
    } else if (type == H5T_NATIVE_UINT64) {
        if (!isIntegerType())
            throw std::runtime_error("Wrong data type of dataset (not " + dataTypeString(type) + ")");
    } else {
        throw std::runtime_error("Wrong data type of dataset (not support of " + dataTypeString(type) + ")");
    }
}

/**
 * @brief Checks float type
 */
void Dataset::checkFloatType() const
{
    checkType(H5T_NATIVE_FLOAT);
}

/**
 * @brief Checks 64-bit unsigned integer type
 */
void Dataset::checkIntegerType() const
{
    checkType(H5T_NATIVE_UINT64);
}

/**
 * @brief Returns datatype as string (H5T_NATIVE_FLOAT or H5T_NATIVE_UINT64)
 * @param[in] type Datatype
 * @return Datatype as string
 */
std::string Dataset::dataTypeString(hid_t type) const
{
    std::string typeStr = "unknown type";
    if (type == H5T_NATIVE_UINT64)
        typeStr = "unsigned 64-bit integers";
    if (type == H5T_NATIVE_FLOAT)
        typeStr = "floats";
    return typeStr;
}

/**
 * @brief Returns memory error message
 * @param[in] size Data size
 * @param[in] type Datatype
 * @return Memory error message
 */
std::string Dataset::memoryErrorMessage(hsize_t size, hid_t type) const
{
    return "There is not enough memory to allocate dataset (dataset size: " + std::to_string(size) + " " + dataTypeString(type) + ")";
}

/**
 * @brief Returns read error message
 * @param[in] size Dataset size
 * @param[in] type Datatype
 * @return Read error message
 */
std::string Dataset::readErrorMessage(hsize_t size, hid_t type) const
{
    return "Can not read the entire dataset, size: " + std::to_string(size) + " " + dataTypeString(type) + " (max size: " + std::to_string(getNumberOfElmsToLoad()) + " " + dataTypeString(type) + ")";
}

/**
 * @brief Prints reading dataset and block message
 * @param[in] block Block index (optional)
 */
void Dataset::printsReadingMessage(hsize_t block) const
{
    if (block) {
        Helper::printDebugMsg("Reading dataset \"" + getName() + "\", block " + std::to_string(block) + "/" +  std::to_string(numberOfBlocks));
    } else {
        Helper::printDebugMsg("Reading dataset \"" + getName() + "\"");
    }
}

/**
 * @brief Prints reading time message
 * @param[in] t0 Star time
 * @param[in] t1 End time
 */
void Dataset::printsReadingTimeMessage(double t0, double t1) const
{
    std::stringstream ss;
    ss << std::setw(11) << std::right << "empty block";
    Helper::printDebugMsg(ss.str());
    Helper::printDebugMsgEnd(std::to_string(int(t1 - t0)) + " ms");
}

/**
 * @brief Prints reading time message with offset and count
 * @param[in] t0 Star time
 * @param[in] t1 End time
 * @param[in] offset Offset
 * @param[in] count Count
 */
void Dataset::printsReadingTimeMessage(double t0, double t1, Vector offset, Vector count) const
{
    std::stringstream ss;
    ss << std::setw(8) << std::right << "  offset: ";
    ss << std::setw(23) << std::left << offset;
    ss << std::setw(7) << std::right << "count: ";
    ss << std::setw(24) << std::left << count;
    Helper::printDebugMsg(ss.str());
    Helper::printDebugMsgEnd(std::to_string(int(t1 - t0)) + " ms");
}

/**
 * @brief Prints writing dataset and block message
 * @param[in] block Block index (optional)
 */
void Dataset::printsWritingMessage(hsize_t block) const
{
    if (block) {
        Helper::printDebugMsg("Writing dataset \"" + getName() + "\", block " + std::to_string(block) + "/" +  std::to_string(numberOfBlocks));
    } else {
        Helper::printDebugMsg("Writing dataset \"" + getName() + "\"");
    }
}

/**
 * @brief Prints writing time message with offset and count
 * @param[in] t0 Star time
 * @param[in] t1 End time
 * @param[in] offset Offset
 * @param[in] count Count
 */
void Dataset::printsWritingTimeMessage(double t0, double t1, Vector offset, Vector count) const
{
    std::stringstream ss;
    ss << std::setw(8) << std::right << "  offset: ";
    ss << std::setw(23) << std::left << offset;
    ss << std::setw(7) << std::right << "count: ";
    ss << std::setw(24) << std::left << count;
    Helper::printDebugMsg(ss.str());
    Helper::printDebugMsgEnd(std::to_string(int(t1 - t0)) + " ms");
}
}
