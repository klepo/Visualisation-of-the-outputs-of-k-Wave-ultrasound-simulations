/**
 * @file        dataset.cpp
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        30 July      2014 (created) \n
 *              28 August    2017 (updated)
 *
 * @brief       The implementation file containing HDF5Helper::Dataset class definition. This class
 *              is used for better work with k-Wave HDF5 datasets (e.g read, write).
 *
 * @license     This file is part of the hdf5helper library for processing the HDF5 data
 *              created by the k-Wave toolbox - http://www.k-wave.org. This file may be used,
 *              distributed and modified under the terms of the LGPL version 3 open source
 *              license. A copy of the LGPL license should have been recieved with this file.
 *              Otherwise, it can be found at: http://www.gnu.org/copyleft/lesser.html.
 *
 * @copyright   Copyright © 2017, Petr Kleparnik, VUT FIT Brno. All Rights Reserved.
 *
 */

#include <dataset.h>
#include <file.h>

namespace HDF5Helper {

/**
 * @brief Creates Dataset object with given file, name and dataset
 * @param[in] dataset Dataset id
 * @param[in] name Name of dataset
 * @param[in] file HDF5 File
 * @throw std::runtime_error
 */
Dataset::Dataset(hid_t dataset, std::string name, File *file) : Object(dataset, name, file)
{
    // Save dataset
    this->dataset = dataset;

    // Init space
    dataspace = H5Dget_space(dataset);
    if (dataspace < 0) {
        throw std::runtime_error("H5Dget_space error");
    }

    // Get type
    datatype = H5Dget_type(dataset);
    if (datatype < 0) {
        throw std::runtime_error("H5Dget_type error");
    }

    // Check data type of dataset
    if (!H5Tequal(datatype, H5T_NATIVE_FLOAT) && !H5Tequal(datatype, H5T_NATIVE_UINT64)) {
        throw std::runtime_error("Wrong data type of dataset");
    }

    // Get rank, dims and chunk dims
    rank = H5Sget_simple_extent_ndims(dataspace);
    if (rank < 0) {
        throw std::runtime_error("H5Sget_simple_extent_ndims error");
    }

    dims = Vector(rank);
    chunkDims = Vector(rank);

    int dimsCount = H5Sget_simple_extent_dims(dataspace, dims.getVectorPtr(), 0);
    if (dimsCount < 0) {
        throw std::runtime_error("H5Sget_simple_extent_dims error");
    }

    plist = H5Dget_create_plist(dataset);
    if (plist < 0) {
        throw std::runtime_error("H5Dget_create_plist error");
    }

    if (H5D_CHUNKED == H5Pget_layout(plist)) {
        int chunkCount = H5Pget_chunk(plist, int(dims.getLength()), chunkDims.getVectorPtr());
        if (chunkCount < 0) {
            throw std::runtime_error("H5Pget_chunk error");
        }
    }

    err = H5Pclose(plist);
    if (err < 0) {
        throw std::runtime_error("H5Pclose error");
    }

    plist_DATASET_XFER = H5Pcreate(H5P_DATASET_XFER);
    if (plist_DATASET_XFER < 0) {
        throw std::runtime_error("H5Pcreate error");
    }

    // Init min/max
    maxVF = 0;
    minVF = 0;
    maxVI = 0;
    minVI = 0;

    // Init some flags for block reading
    offsets = 0;
    counts = 0;
    //numberOfElementsToLoad = ile->getNumberOfElmsToLoad();
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
    if (deleteLog)
        std::cout << "Closing dataset \"" << getName() << "\"";
    err = H5Pclose(plist_DATASET_XFER);
    if (err < 0) {
        //throw std::runtime_error("H5Pclose error");
    }
    err = H5Sclose(dataspace);
    if (err < 0) {
        //throw std::runtime_error("H5Sclose error");
    }
    err = H5Tclose(datatype);
    if (err < 0) {
        //throw std::runtime_error("H5Tclose error");
    }
    err = H5Dclose(dataset);
    if (err < 0) {
        //throw std::runtime_error("H5Dclose error");
    }
    if (deleteLog)
        std::cout << " ... OK" << std::endl;
}

/**
 * @brief Returns dataset id
 * @return Dataset id
 */
hid_t Dataset::getId() const
{
    return dataset;
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
    return H5Tget_class(datatype);
}

/**
 * @brief Returns dataset datatype
 * @return Dataset datatype
 */
hid_t Dataset::getDataType() const
{
    return datatype;
}

/**
 * @brief Returns dataset type
 * @param[in] sensorMaskSize Sensor mask size (optional)
 * @return Dataset type
 */
DatasetType Dataset::getType(hsize_t sensorMaskSize) const
{
    Vector4D nDims = getFile()->getNdims();
    if (getDims().getLength() == 3) { // 3D type
        Vector3D dims = getDims();
        if (H5Tequal(datatype, H5T_NATIVE_UINT64)) {
            if (dims == Vector3D(1, 1, 1)) {
                if (getOnlyName() == NT_DATASET) {
                    return DatasetType::N_DIM_T;
                }
                if (getOnlyName() == NX_DATASET) {
                    return DatasetType::N_DIM_X;
                }
                if (getOnlyName() == NY_DATASET) {
                    return DatasetType::N_DIM_Y;
                }
                if (getOnlyName() == NZ_DATASET) {
                    return DatasetType::N_DIM_Z;
                }
            }
            if (getOnlyName() == SENSOR_MASK_INDEX_DATASET) {
                return DatasetType::MASK_INDEX;
            }
            if (getOnlyName() == SENSOR_MASK_CORNERS_DATASET) {
                return DatasetType::MASK_CORNERS;
            }
            if (getOnlyName() == P_SOURCE_INPUT_DATASET) {
                return DatasetType::P_SOURCE_INPUT;
            }
        }
        if (H5Tequal(datatype, H5T_FLOAT)) {
            if (dims.z() == nDims.z()
                    && dims.y() == nDims.y()
                    && dims.x() == nDims.x()
                    ) {
                return DatasetType::BASIC_3D;
            }
            if (dims.z() < nDims.z()
                    && dims.y() < nDims.y()
                    && dims.x() < nDims.x()
                    && this->hasAttribute(SRC_SIZE_X_ATTR)
                    && hasAttribute(SRC_SIZE_Y_ATTR)
                    && hasAttribute(SRC_SIZE_Z_ATTR)
                    && hasAttribute(SRC_DATASET_NAME_ATTR)
                    ) {
                return DatasetType::DWNSMPL_3D;
            }
            if (hasAttribute(POSITION_X_ATTR)
                    && hasAttribute(POSITION_Y_ATTR)
                    && hasAttribute(POSITION_Z_ATTR)
                    ) {
                return DatasetType::MASK_3D;
            }
            if (dims.z() == 1
                    && dims.y() == 1
                    && dims.x() == sensorMaskSize
                    && !hasAttribute(SRC_DATASET_NAME_ATTR)
                    ) {
                return DatasetType::BASIC_MASK;
            }
            if (dims.z() == 1
                    && dims.y() <= nDims.w()
                    && dims.x() == sensorMaskSize
                    && !hasAttribute(SRC_DATASET_NAME_ATTR)
                    ) {
                return DatasetType::TIME_STEPS_MASK;
            }
            if (dims.z() == 1
                    //&& dims.y() <= nDims.w()
                    && dims.x() == sensorMaskSize
                    && hasAttribute(SRC_DATASET_NAME_ATTR)
                    && hasAttribute(HDF5Helper::C_PERIOD_ATTR)
                    && hasAttribute(C_TYPE_ATTR)
                    && readAttributeS(C_TYPE_ATTR, false) == "fi"
                    ) {
                return DatasetType::TIME_STEPS_FI_MASK;
            }
            if (dims.z() == 1
                    //&& dims.y() <= nDims.w()
                    && dims.x() == sensorMaskSize
                    && hasAttribute(SRC_DATASET_NAME_ATTR)
                    && hasAttribute(HDF5Helper::C_PERIOD_ATTR)
                    && hasAttribute(C_TYPE_ATTR)
                    && readAttributeS(C_TYPE_ATTR, false) == "k"
                    ) {
                return DatasetType::TIME_STEPS_K_MASK;
            }
            if (dims.z() == 1
                    //&& dims.y() <= nDims.w()
                    && dims.x() == sensorMaskSize
                    && hasAttribute(SRC_DATASET_NAME_ATTR)
                    && hasAttribute(C_TYPE_ATTR)
                    && readAttributeS(C_TYPE_ATTR, false) == "d"
                    ) {
                return DatasetType::TIME_STEPS_D_MASK;
            }
            if (dims.z() == 1
                    && dims.y() <= nDims.w()
                    && dims.x() == sensorMaskSize
                    && hasAttribute(SRC_DATASET_NAME_ATTR)
                    && hasAttribute(C_TYPE_ATTR)
                    && readAttributeS(C_TYPE_ATTR, false) == "s"
                    ) {
                return DatasetType::TIME_STEPS_S_MASK;
            }
        }
    }
    if (getDims().getLength() == 4) { // 4D type (cuboids)
        if (H5Tequal(datatype, H5T_FLOAT)) {
            // Downsampled
            if (hasAttribute(SRC_SIZE_X_ATTR)
                    && hasAttribute(SRC_SIZE_Y_ATTR)
                    && hasAttribute(SRC_SIZE_Z_ATTR)
                    && hasAttribute(SRC_DATASET_NAME_ATTR)
                    ) {
                // With position attributtes
                if (hasAttribute(POSITION_X_ATTR)
                        && hasAttribute(POSITION_Y_ATTR)
                        && hasAttribute(POSITION_Z_ATTR)
                        ) {
                    if (hasAttribute(HDF5Helper::C_TYPE_ATTR)
                            && hasAttribute(HDF5Helper::C_PERIOD_ATTR)
                            && readAttributeS(HDF5Helper::C_TYPE_ATTR, false) == "fi"
                            ) {
                        return DatasetType::CUBOID_ATTR_DWNSMPL_FI;
                    } else if (hasAttribute(HDF5Helper::C_TYPE_ATTR)
                               && hasAttribute(HDF5Helper::C_PERIOD_ATTR)
                               && readAttributeS(HDF5Helper::C_TYPE_ATTR, false) == "k"
                            ) {
                        return DatasetType::CUBOID_ATTR_DWNSMPL_K;
                    } else if (hasAttribute(HDF5Helper::C_TYPE_ATTR)
                               && readAttributeS(HDF5Helper::C_TYPE_ATTR, false) == "d"
                            ) {
                        return DatasetType::CUBOID_ATTR_DWNSMPL_D;
                    } else if (hasAttribute(HDF5Helper::C_TYPE_ATTR)
                               && readAttributeS(HDF5Helper::C_TYPE_ATTR, false) == "s"
                            ) {
                        return DatasetType::CUBOID_ATTR_DWNSMPL_S;
                    } else {
                        return DatasetType::CUBOID_ATTR_DWNSMPL;
                    }
                } else { // Without position attributes
                    if (hasAttribute(HDF5Helper::C_TYPE_ATTR)
                            && hasAttribute(HDF5Helper::C_PERIOD_ATTR)
                            && readAttributeS(HDF5Helper::C_TYPE_ATTR, false) == "fi"
                            ) {
                        return DatasetType::CUBOID_DWNSMPL_FI;
                    } else if (hasAttribute(HDF5Helper::C_TYPE_ATTR)
                               && hasAttribute(HDF5Helper::C_PERIOD_ATTR)
                               && readAttributeS(HDF5Helper::C_TYPE_ATTR, false) == "k"
                            ) {
                        return DatasetType::CUBOID_DWNSMPL_K;
                    } else if (hasAttribute(HDF5Helper::C_TYPE_ATTR)
                               && readAttributeS(HDF5Helper::C_TYPE_ATTR, false) == "d"
                            ) {
                        return DatasetType::CUBOID_DWNSMPL_D;
                    } else if (hasAttribute(HDF5Helper::C_TYPE_ATTR)
                               && readAttributeS(HDF5Helper::C_TYPE_ATTR, false) == "s"
                            ) {
                        return DatasetType::CUBOID_DWNSMPL_S;
                    } else {
                        return DatasetType::CUBOID_DWNSMPL;
                    }
                }
            } else { // Original
                // With position attributtes
                if (hasAttribute(POSITION_X_ATTR)
                        && hasAttribute(POSITION_Y_ATTR)
                        && hasAttribute(POSITION_Z_ATTR)
                        ) {
                    if (hasAttribute(HDF5Helper::C_TYPE_ATTR)
                            && hasAttribute(HDF5Helper::C_PERIOD_ATTR)
                            && readAttributeS(HDF5Helper::C_TYPE_ATTR, false) == "fi"
                            && hasAttribute(SRC_DATASET_NAME_ATTR)
                            ) {
                        return DatasetType::CUBOID_ATTR_FI;
                    } else if (hasAttribute(HDF5Helper::C_TYPE_ATTR)
                               && hasAttribute(HDF5Helper::C_PERIOD_ATTR)
                               && readAttributeS(HDF5Helper::C_TYPE_ATTR, false) == "k"
                               && hasAttribute(SRC_DATASET_NAME_ATTR)
                            ) {
                        return DatasetType::CUBOID_ATTR_K;
                    } else if (hasAttribute(HDF5Helper::C_TYPE_ATTR)
                               && readAttributeS(HDF5Helper::C_TYPE_ATTR, false) == "d"
                               && hasAttribute(SRC_DATASET_NAME_ATTR)
                            ) {
                        return DatasetType::CUBOID_ATTR_D;
                    } else if (hasAttribute(HDF5Helper::C_TYPE_ATTR)
                               && readAttributeS(HDF5Helper::C_TYPE_ATTR, false) == "s"
                               && hasAttribute(SRC_DATASET_NAME_ATTR)
                            ) {
                        return DatasetType::CUBOID_ATTR_S;
                    } else {
                        return DatasetType::CUBOID_ATTR;
                    }
                } else { // Without position attributes
                    if (hasAttribute(HDF5Helper::C_TYPE_ATTR)
                            && hasAttribute(HDF5Helper::C_PERIOD_ATTR)
                            && readAttributeS(HDF5Helper::C_TYPE_ATTR, false) == "fi"
                            && hasAttribute(SRC_DATASET_NAME_ATTR)
                            ) {
                        return DatasetType::CUBOID_FI;
                    } else if (hasAttribute(HDF5Helper::C_TYPE_ATTR)
                               && hasAttribute(HDF5Helper::C_PERIOD_ATTR)
                               && readAttributeS(HDF5Helper::C_TYPE_ATTR, false) == "k"
                               && hasAttribute(SRC_DATASET_NAME_ATTR)
                            ) {
                        return DatasetType::CUBOID_K;
                    } else if (hasAttribute(HDF5Helper::C_TYPE_ATTR)
                               && readAttributeS(HDF5Helper::C_TYPE_ATTR, false) == "d"
                               && hasAttribute(SRC_DATASET_NAME_ATTR)
                            ) {
                        return DatasetType::CUBOID_D;
                    } else if (hasAttribute(HDF5Helper::C_TYPE_ATTR)
                               && readAttributeS(HDF5Helper::C_TYPE_ATTR, false) == "s"
                               && hasAttribute(SRC_DATASET_NAME_ATTR)
                            ) {
                        return DatasetType::CUBOID_S;
                    } else {
                        return DatasetType::CUBOID;
                    }
                }
            }
        }
    }

    return DatasetType::UNKNOWN;
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
            return "General k-Wave dataset type";
        case DatasetType::UNKNOWN:
            return "Unknown type";
        case DatasetType::N_DIM_X:
            return "Nx type";
        case DatasetType::N_DIM_Y:
            return "Ny type";
        case DatasetType::N_DIM_Z:
            return "Nz type";
        case DatasetType::N_DIM_T:
            return "Nt type";
        case DatasetType::MASK_INDEX:
            return "Sensor mask index type";
        case DatasetType::MASK_CORNERS:
            return "Sensor mask corners type";
        case DatasetType::P_SOURCE_INPUT:
            return "P source input type";
        case DatasetType::BASIC_3D:
            return "3D type";
        case DatasetType::DWNSMPL_3D:
            return "3D type (donwsampled)";
        case DatasetType::MASK_3D:
            return "3D type (reshaped mask)";
        case DatasetType::BASIC_MASK:
            return "Sensor mask type";
        case DatasetType::TIME_STEPS_MASK:
            return "Sensor mask type (time steps)";
        case DatasetType::TIME_STEPS_FI_MASK:
            return "Sensor mask type (compressed fi)";
        case DatasetType::TIME_STEPS_K_MASK:
            return "Sensor mask type (compressed k)";
        case DatasetType::TIME_STEPS_D_MASK:
            return "Sensor mask type (decompressed)";
        case DatasetType::TIME_STEPS_S_MASK:
            return "Sensor mask type (difference)";
        case DatasetType::CUBOID:
            return "Cuboid type";
        case DatasetType::CUBOID_FI:
            return "Cuboid type (compressed fi)";
        case DatasetType::CUBOID_K:
            return "Cuboid type (compressed k)";
        case DatasetType::CUBOID_D:
            return "Cuboid type (decompressed)";
        case DatasetType::CUBOID_S:
            return "Cuboid type (difference)";
        case DatasetType::CUBOID_DWNSMPL:
            return "Cuboid type (donwsampled";
        case DatasetType::CUBOID_DWNSMPL_FI:
            return "Cuboid type (donwsampled compressed fi)";
        case DatasetType::CUBOID_DWNSMPL_K:
            return "Cuboid type (donwsampled compressed k)";
        case DatasetType::CUBOID_DWNSMPL_D:
            return "Cuboid type (donwsampled decompressed)";
        case DatasetType::CUBOID_DWNSMPL_S:
            return "Cuboid type (donwsampled difference)";
        case DatasetType::CUBOID_ATTR:
            return "Cuboid type with attributes";
        case DatasetType::CUBOID_ATTR_FI:
            return "Cuboid type with attributes (compressed fi)";
        case DatasetType::CUBOID_ATTR_K:
            return "Cuboid type with attributes (compressed k)";
        case DatasetType::CUBOID_ATTR_D:
            return "Cuboid type with attributes (decompressed)";
        case DatasetType::CUBOID_ATTR_S:
            return "Cuboid type with attributes (difference)";
        case DatasetType::CUBOID_ATTR_DWNSMPL:
            return "Cuboid type with attributes (donwsampled)";
        case DatasetType::CUBOID_ATTR_DWNSMPL_FI:
            return "Cuboid type with attributes (donwsampled compressed fi)";
        case DatasetType::CUBOID_ATTR_DWNSMPL_K:
            return "Cuboid type with attributes (donwsampled compressed k)";
        case DatasetType::CUBOID_ATTR_DWNSMPL_D:
            return "Cuboid type with attributes (donwsampled decompressed)";
        case DatasetType::CUBOID_ATTR_DWNSMPL_S:
            return "Cuboid type with attributes (donwsampled difference)";
    };
    return "Unknow type";
}

/**
 * @brief Returns global maximal integer value
 * @param[out] value Global maximal integer value
 * @param[out] maxVIndex Index of maximal value
 * @param[in] reset Reset flag for finding the value in dataset (optional)
 * @throw std::runtime_error
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
 * @brief Returns global minimal integer value
 * @param[out] value Global minimal integer value
 * @param[out] minVIndex Index of minimal value
 * @param[in] reset Reset flag for finding the value in dataset (optional)
 * @throw std::runtime_error
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
 * @throw std::runtime_error
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
 * @throw std::runtime_error
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
 * @param[in] log Logging flag (optional)
 *
 * The values are loaded from dataset attributes or searched from dataset data
 * and are stored to this object.
 */
void Dataset::findAndSetGlobalMinAndMaxValue(bool reset, bool log)
{
    if (isFloatType()) {
        if (reset) {
            std::cout << "Finding min/max value ..." << std::endl;
            Dataset::findGlobalMinAndMaxValueF();
            Dataset::setAttribute(MIN_ATTR, minVF, log);
            Dataset::setAttribute(MAX_ATTR, maxVF, log);
            Dataset::setAttribute(MIN_INDEX_ATTR, minVIndex, log);
            Dataset::setAttribute(MAX_INDEX_ATTR, maxVIndex, log);
            std::cout << "Finding min/max value ... OK" << std::endl;
        } else {
            if (this->hasAttribute(MIN_ATTR) && this->hasAttribute(MAX_ATTR) && this->hasAttribute(MIN_INDEX_ATTR) && this->hasAttribute(MAX_INDEX_ATTR)) {
                std::cout << "Reading min/max value ..." << std::endl;
                minVF = Dataset::readAttributeF(MIN_ATTR, log);
                maxVF = Dataset::readAttributeF(MAX_ATTR, log);
                minVIndex = Dataset::readAttributeI(MIN_INDEX_ATTR, log);
                maxVIndex = Dataset::readAttributeI(MAX_INDEX_ATTR, log);
                issetGlobalMinAndMaxValue = true;
                std::cout << "Reading min/max value ... OK" << std::endl;
            } else {
                std::cout << "Finding min/max value ..." << std::endl;
                Dataset::findGlobalMinAndMaxValueF();
                Dataset::setAttribute(MIN_ATTR, minVF, log);
                Dataset::setAttribute(MAX_ATTR, maxVF, log);
                Dataset::setAttribute(MIN_INDEX_ATTR, minVIndex, log);
                Dataset::setAttribute(MAX_INDEX_ATTR, maxVIndex, log);
                std::cout << "Finding min/max value ... OK" << std::endl;
            }
        }
    } else {
        if (reset) {
            std::cout << "Finding min/max value ..." << std::endl;
            Dataset::findGlobalMinAndMaxValueI();
            Dataset::setAttribute(MIN_ATTR, minVI, log);
            Dataset::setAttribute(MAX_ATTR, maxVI, log);
            Dataset::setAttribute(MIN_INDEX_ATTR, minVIndex, log);
            Dataset::setAttribute(MAX_INDEX_ATTR, maxVIndex, log);
            std::cout << "Finding min/max value ... OK" << std::endl;
        } else {
            if (this->hasAttribute(MIN_ATTR) && this->hasAttribute(MAX_ATTR) && this->hasAttribute(MIN_INDEX_ATTR) && this->hasAttribute(MAX_INDEX_ATTR)) {
                std::cout << "Reading min/max value ..." << std::endl;
                minVI = Dataset::readAttributeI(MIN_ATTR, log);
                maxVI = Dataset::readAttributeI(MAX_ATTR, log);
                minVIndex = Dataset::readAttributeI(MIN_INDEX_ATTR, log);
                maxVIndex = Dataset::readAttributeI(MAX_INDEX_ATTR, log);
                issetGlobalMinAndMaxValue = true;
                std::cout << "Reading min/max value ... OK" << std::endl;
            } else {
                std::cout << "Finding min/max value ..." << std::endl;
                Dataset::findGlobalMinAndMaxValueI();
                Dataset::setAttribute(MIN_ATTR, minVI, log);
                Dataset::setAttribute(MAX_ATTR, maxVI, log);
                Dataset::setAttribute(MIN_INDEX_ATTR, minVIndex, log);
                Dataset::setAttribute(MAX_INDEX_ATTR, maxVIndex, log);
                std::cout << "Finding min/max value ... OK" << std::endl;
            }
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
    return counts[0];
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
    numberOfElementsToLoad = count;
    if (dims.getSize() <= numberOfElementsToLoad) {
        numberOfElementsToLoad = dims.getSize();
    }
    initBlockReading();
}

/**
 * @brief Sets maximal number of elements to load for block reading
 * @param[in] count Number of elements to load for block reading
 */
void Dataset::setMaxNumberOfElmsToLoad(hsize_t count)
{
    if (count < getNumberOfElmsToLoad()) {
        setNumberOfElmsToLoad(count);
    }
}

/**
 * @brief Sets MPIO access type
 * @param[in] type MPIO access type
 * @throw std::runtime_error
 */
void Dataset::setMPIOAccess(H5FD_mpio_xfer_t type)
{
    if (type == H5FD_MPIO_COLLECTIVE) {
        std::cout << "Setting H5FD_MPIO_COLLECTIVE access (" << getName() << ")" << std::endl;
    } else if (type == H5FD_MPIO_INDEPENDENT) {
        std::cout << "Setting H5FD_MPIO_INDEPENDENT access (" << getName() << ")" << std::endl;
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
 * @param[in] log Logging flag (optional)
 * @param[in] block Index of block for block reading (optional)
 */
void Dataset::readDataset(Vector offset, Vector count, float *&data, float &min, float &max, hsize_t &minIndex, hsize_t &maxIndex, bool log, hsize_t block)
{
    if (log)
        printsReadingMessage(block);
    checkDataTypeAndAllocation(data, H5T_NATIVE_FLOAT, count.getSize());
    readDatasetGeneral(offset, count, data, log);
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
 * @param[in] log Logging flag (optional)
 * @param[in] block Index of block for block reading (optional)
 */
void Dataset::readDataset(Vector offset, Vector count, hsize_t *&data, hsize_t &min, hsize_t &max, hsize_t &minIndex, hsize_t &maxIndex, bool log, hsize_t block)
{
    if (log)
        printsReadingMessage(block);
    checkDataTypeAndAllocation(data, H5T_NATIVE_UINT64, count.getSize());
    readDatasetGeneral(offset, count, data, log);
    Dataset::findMinAndMaxValue(data, count.getSize(), min, max, minIndex, maxIndex);
}

/**
 * @brief Reads dataset
 * @param[in] offset Data offset
 * @param[in] count Data count
 * @param[out] data Output data
 * @param[in] log Logging flag (optional)
 * @param[in] block Index of block for block reading (optional)
 */
void Dataset::readDataset(Vector offset, Vector count, float *&data, bool log, hsize_t block)
{
    if (log)
        printsReadingMessage(block);
    checkDataTypeAndAllocation(data, H5T_NATIVE_FLOAT, count.getSize());
    readDatasetGeneral(offset, count, data, log);
}

/**
 * @brief Reads dataset
 * @param[in] offset Data offset
 * @param[in] count Data count
 * @param[out] data Output data
 * @param[in] log Logging flag (optional)
 * @param[in] block Index of block for block reading (optional)
 */
void Dataset::readDataset(Vector offset, Vector count, hsize_t *&data, bool log, hsize_t block)
{
    if (log)
        printsReadingMessage(block);
    checkDataTypeAndAllocation(data, H5T_NATIVE_UINT64, count.getSize());
    readDatasetGeneral(offset, count, data, log);
}

/**
 * @brief Reads dataset
 * @param[out] data Output data
 * @param[in] log Logging flag (optional)
 */
void Dataset::readDataset(float *&data, bool log)
{
    readDataset(Vector(dims.getLength(), 0), dims, data, log);
}

/**
 * @brief Reads dataset
 * @param[out] data Output data
 * @param[in] log Logging flag (optional)
 */
void Dataset::readDataset(hsize_t *&data, bool log)
{
    readDataset(Vector(dims.getLength(), 0), dims, data, log);
}

/**
 * @brief Reads dataset
 * @param[out] data Output data
 * @param[out] min Minimal value
 * @param[out] max Maximal value
 * @param[out] minIndex Index of minimal value
 * @param[out] maxIndex Index of maximal value
 * @param[in] log Logging flag (optional)
 */
void Dataset::readDataset(float *&data, float &min, float &max, hsize_t &minIndex, hsize_t &maxIndex, bool log)
{
    readDataset(Vector(dims.getLength(), 0), dims, data, min, max, minIndex, maxIndex, log);
}

/**
 * @brief Reads dataset
 * @param[out] data Output data
 * @param[out] min Minimal value
 * @param[out] max Maximal value
 * @param[out] minIndex Index of minimal value
 * @param[out] maxIndex Index of maximal value
 * @param[in] log Logging flag (optional)
 */
void Dataset::readDataset(hsize_t *&data, hsize_t &min, hsize_t &max, hsize_t &minIndex, hsize_t &maxIndex, bool log)
{
    readDataset(Vector(dims.getLength(), 0), dims, data, min, max, minIndex, maxIndex, log);
}

/**
 * @brief Reads dataset of size 1
 * @param[out] data Output data
 * @param[in] log Logging flag (optional)
 * @throw std::runtime_error
 */
void Dataset::readDataset(float &data, bool log)
{
    if (dims.getSize() == 1) {
        float *dataTmp;
        readDataset(Vector(dims.getLength(), 0), dims, dataTmp, log);
        data = dataTmp[0];
        delete[] dataTmp;
    } else {
        throw std::runtime_error("Dataset has not size 1");
    }
}

/**
 * @brief Reads dataset of size 1
 * @param[out] data Output data
 * @param[in] log Logging flag (optional)
 * @throw std::runtime_error
 */
void Dataset::readDataset(hsize_t &data, bool log)
{
    if (dims.getSize() == 1) {
        hsize_t *dataTmp;
        readDataset(Vector(dims.getLength(), 0), dims, dataTmp, log);
        data = dataTmp[0];
        delete[] dataTmp;
    } else {
        throw std::runtime_error("Dataset has not size 1");
    }
}

/**
 * @brief Writes data to the dataset
 * @param[in] offset Data offset
 * @param[in] count Data count
 * @param[in] data Data to write
 * @param[in] log Logging flag (optional)
 */
void Dataset::writeDataset(Vector offset, Vector count, float *data, bool log)
{
    checkFloatType();
    writeDatasetGeneral(offset, count, static_cast<void *>(data), log);
}

/**
 * @brief Writes data to the dataset
 * @param[in] offset Data offset
 * @param[in] count Data count
 * @param[in] data Data to write
 * @param[in] log Logging flag (optional)
 */
void Dataset::writeDataset(Vector offset, Vector count, hsize_t *data, bool log)
{
    checkIntegerType();
    writeDatasetGeneral(offset, count, static_cast<void *>(data), log);
}

/**
 * @brief Writes data to the dataset
 * @param[in] data Data to write
 * @param[in] log Logging flag (optional)
 */
void Dataset::writeDataset(float *data, bool log)
{
    writeDataset(Vector(dims.getLength(), 0), dims, data, log);
}

/**
 * @brief Writes data to the dataset
 * @param[in] data Data to write
 * @param[in] log Logging flag (optional)
 */
void Dataset::writeDataset(hsize_t *data, bool log)
{
    writeDataset(Vector(dims.getLength(), 0), dims, data, log);
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
 * @param[in] log Logging flag (optional)
 */
void Dataset::readBlock(const hsize_t index, Vector &offset, Vector &count, float *&data, float &min, float &max, hsize_t &minIndex, hsize_t &maxIndex, bool log)
{
    readDataset(offsets[index], counts[index], data, min, max, minIndex, maxIndex, log, index + 1);
    offset = offsets[index];
    count = counts[index];
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
 * @param[in] log Logging flag (optional)
 */
void Dataset::readBlock(const hsize_t index, Vector &offset, Vector &count, hsize_t *&data, hsize_t &min, hsize_t &max, hsize_t &minIndex, hsize_t &maxIndex, bool log)
{
    readDataset(offsets[index], counts[index], data, min, max, minIndex, maxIndex, log, index + 1);
    offset = offsets[index];
    count = counts[index];
}

/**
 * @brief Reads block of dataset
 * @param[in] index Block index
 * @param[out] offset Data offset
 * @param[out] count Data count
 * @param[out] data Output data
 * @param[in] log Logging flag (optional)
 */
void Dataset::readBlock(const hsize_t index, Vector &offset, Vector &count, float *&data, bool log)
{
    readDataset(offsets[index], counts[index], data, log, index + 1);
    offset = offsets[index];
    count = counts[index];
}

/**
 * @brief Reads block of dataset
 * @param[in] index Block index
 * @param[out] offset Data offset
 * @param[out] count Data count
 * @param[out] data Output data
 * @param[in] log Logging flag (optional)
 */
void Dataset::readBlock(const hsize_t index, Vector &offset, Vector &count, hsize_t *&data, bool log)
{
    readDataset(offsets[index], counts[index], data, log, index + 1);
    offset = offsets[index];
    count = counts[index];
}

/**
 * @brief Reads empty block
 * @throw std::runtime_error
 */
void Dataset::readEmptyBlock()
{
    hid_t dataspace = H5Dget_space(dataset);
    H5Sselect_none(dataspace);
    Vector3D count;
    hid_t memspace = H5Screate_simple(3, count.getVectorPtr(), 0);
    H5Sselect_none(memspace);
    double t0 = 0, t1 = 0;
    t0 = getTime();
    std::cout << "Reading dataset " << getName() << " ..." << std::endl;
    err = H5Dread(dataset, datatype, memspace, dataspace, plist_DATASET_XFER, 0);
    t1 = getTime();
    if (err < 0) {
        throw std::runtime_error("H5Dread error");
    }
    std::cout << getName() << " \tread time:  \t" << (t1 - t0) << " ms;\tempty block" << std::endl;
}

/**
 * @brief Reads general dataset
 * @param[in] offset Data offset
 * @param[in] count Data count
 * @param[out] data Output data
 * @param[in] log Logging flag (optional)
 * @throw std::runtime_error
 */
void Dataset::readDatasetGeneral(Vector offset, Vector count, void *data, bool log)
{
    Dataset::checkOffsetAndCountParams(offset, count);
    Vector mem_offset(offset.getLength());

    hid_t dataspace = H5Dget_space(dataset);
    err = H5Sselect_hyperslab(dataspace, H5S_SELECT_SET, offset.getVectorPtr(), 0, count.getVectorPtr(), 0);
    if (err < 0) {
        throw std::runtime_error("H5Sselect_hyperslab error");
    }
    hid_t memspace = H5Screate_simple(int(count.getLength()), count.getVectorPtr(), 0);
    if (memspace < 0) {
        throw std::runtime_error("H5Screate_simple error");
    }

    err = H5Sselect_hyperslab(memspace, H5S_SELECT_SET, mem_offset.getVectorPtr(), 0, count.getVectorPtr(), 0);
    if (err < 0) {
        throw std::runtime_error("H5Sselect_hyperslab error");
    }

    double t0 = 0, t1 = 0;

    if (log)
        t0 = getTime();

    // Reading
    err = H5Dread(dataset, datatype, memspace, dataspace, plist_DATASET_XFER, data);
    if (err < 0) {
        throw std::runtime_error("H5Dread error");
    }

    if (log)
        t1 = getTime();

    err = H5Sclose(dataspace);
    if (err < 0) {
        throw std::runtime_error("H5Sclose error");
    }
    err = H5Sclose(memspace);
    if (err < 0) {
        throw std::runtime_error("H5Sclose error");
    }

    if (log)
        std::cout << getName() << " \tread time: \t" << (t1 - t0) << " ms;\toffset: " << offset << ";\tcount: " << count << std::endl;

    // Debug output
    if ((*getFile()->getLogFileStream()).is_open()) {
        int r = 0;
        if (count[0] == 1) r = 0;
        if (count[1] == 1) r = 1;
        if (count[2] == 1) r = 2;
        *getFile()->getLogFileStream() << (t1 - t0) << ";" << offset << ";" << r << std::endl;
    }
}

/**
 * @brief Writes general dataset
 * @param[in] offset Data offset
 * @param[in] count Data count
 * @param[in] data Output data
 * @param[in] log Logging flag (optional)
 * @throw std::runtime_error
 */
void Dataset::writeDatasetGeneral(Vector offset, Vector count, void *data, bool log)
{
    Dataset::checkOffsetAndCountParams(offset, count);
    Vector mem_offset(offset.getLength());

    hid_t dataspace = H5Dget_space(dataset);
    err = H5Sselect_hyperslab(dataspace, H5S_SELECT_SET, offset.getVectorPtr(), 0, count.getVectorPtr(), 0);
    if (err < 0) {
        throw std::runtime_error("H5Sselect_hyperslab error");
    }

    hid_t memspace = H5Screate_simple(int(count.getLength()), count.getVectorPtr(), 0);
    if (memspace < 0) {
        throw std::runtime_error("H5Screate_simple error");
    }

    err = H5Sselect_hyperslab(memspace, H5S_SELECT_SET, mem_offset.getVectorPtr(), 0, count.getVectorPtr(), 0);
    if (err < 0) {
        throw std::runtime_error("H5Sselect_hyperslab error");
    }

    double t0 = 0, t1 = 0;

    if (log)
        t0 = getTime();

    // Writing
    err = H5Dwrite(dataset, datatype, memspace, dataspace, plist_DATASET_XFER, data);
    if (err < 0) {
        throw std::runtime_error("H5Dwrite error");
    }

    if (log)
        t1 = getTime();

    err = H5Sclose(dataspace);
    if (err < 0) {
        throw std::runtime_error("H5Sclose error");
    }
    err = H5Sclose(memspace);
    if (err < 0) {
        throw std::runtime_error("H5Sclose error");
    }

    if (log)
        std::cout << getName() << " \twrite time:  \t" << (t1 - t0) << " ms;\toffset: " << offset << ";\tcount: " << count << std::endl;
}

/**
 * @brief Checks offset and count params
 * @param[in] offset Data offset
 * @param[in] count Data count
 * @throw std::runtime_error
 */
void Dataset::checkOffsetAndCountParams(Vector offset, Vector count)
{
    if ((dims.getLength() != offset.getLength()) || (dims.getLength() != count.getLength()) || count.getLength() != getRank()) {
        std::cout << dims << " " << offset << " " << count;
        throw std::runtime_error("Wrong offset or count");
    }

    for (unsigned int i = 0; i < offset.getLength(); i++) {
        if (offset[i] >= dims[i])
            throw std::runtime_error("Wrong offset - too big offset of dimension " + std::to_string(i));
        if (count[i] <= 0)
            throw std::runtime_error("Wrong count - too small count of dimension " + std::to_string(i));
        if (offset[i] + count[i] > dims[i])
            throw std::runtime_error("Wrong count - sum of offset and count of dimension " + std::to_string(i) + " is too big");
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
void Dataset::findMinAndMaxValue(const float *data, const hsize_t size, float &minVF, float &maxVF, hsize_t &minVFIndex, hsize_t &maxVFIndex)
{
    bool first = true;
    for (hsize_t i = 0; i < size; i++) {
        HDF5Helper::checkOrSetMinMaxValue(first, minVF, maxVF, data[i]);
        if (data[i] == minVF)
            minVFIndex = i;
        if (data[i] == maxVF)
            maxVFIndex = i;
    }
}

/**
 * @brief Finds minimal and maximal integer value
 * @param[in] data Input data
 * @param[in] size Size of data
 * @param[out] minVI Minimal value
 * @param[out] maxVI Maximal value
 * @param[out] minVIIndex Index of minimal value
 * @param[out] maxVIIndex Index of maximal value
 */
void Dataset::findMinAndMaxValue(const hsize_t *data, const hsize_t size, hsize_t &minVI, hsize_t &maxVI, hsize_t &minVIIndex, hsize_t &maxVIIndex)
{
    bool first = true;
    for (hsize_t i = 0; i < size; i++) {
        HDF5Helper::checkOrSetMinMaxValue(first, minVI, maxVI, data[i]);
        if (data[i] == minVI)
            minVIIndex = i;
        if (data[i] == maxVI)
            maxVIIndex = i;
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
                minVF = Dataset::readAttributeF(MIN_ATTR, false);
                maxVF = Dataset::readAttributeF(MAX_ATTR, false);
                minVIndex = Dataset::readAttributeI(MIN_INDEX_ATTR, false);
                maxVIndex = Dataset::readAttributeI(MAX_INDEX_ATTR, false);
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
                minVI = Dataset::readAttributeI(MIN_ATTR, false);
                maxVI = Dataset::readAttributeI(MAX_ATTR, false);
                minVIndex = Dataset::readAttributeI(MIN_INDEX_ATTR, false);
                maxVIndex = Dataset::readAttributeI(MAX_INDEX_ATTR, false);
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
    float minVFTmp;
    float maxVFTmp;
    hsize_t minVIndexTmp;
    hsize_t maxVIndexTmp;
    hsize_t linearOffset = 0;
    bool first = true;
    for (hsize_t i = 0; i < numberOfBlocks; i++) {
        float *data;
        readBlock(i, offset, count, data, minVFTmp, maxVFTmp, minVIndexTmp, maxVIndexTmp);
        if (first)
            minVF = maxVF = data[0];
        first = false;
        convertMultiDimToLinear(offset, linearOffset, dims);
        if (minVFTmp < minVF) {
            minVF = minVFTmp;
            minVIndex = linearOffset + minVIndexTmp;
        }
        if (maxVFTmp > maxVF) {
            maxVF = maxVFTmp;
            maxVIndex = linearOffset + maxVIndexTmp;
        }
        delete [] data; // !!!
    }
    issetGlobalMinAndMaxValue = true;
}

/**
 * @brief Finds global minimal and maximal integer value
 */
void Dataset::findGlobalMinAndMaxValueI()
{
    Vector offset;
    Vector count;
    hsize_t minVITmp;
    hsize_t maxVITmp;
    hsize_t minVIndexTmp;
    hsize_t maxVIndexTmp;
    hsize_t linearOffset = 0;
    bool first = true;
    for (hsize_t i = 0; i < numberOfBlocks; i++) {
        hsize_t *data;
        readBlock(i, offset, count, data, minVITmp, maxVITmp, minVIndexTmp, maxVIndexTmp);
        if (first)
            minVI = maxVI = data[0];
        first = false;
        convertMultiDimToLinear(offset, linearOffset, dims);
        if (minVITmp < minVI) {
            minVI = minVITmp;
            minVIndex = linearOffset + minVIndexTmp;
        }
        if (maxVITmp > maxVI) {
            maxVI = maxVITmp;
            maxVIndex = linearOffset + maxVIndexTmp;
        }
        delete [] data; // !!!
    }
    issetGlobalMinAndMaxValue = true;
}

/**
 * @brief Inits block reading
 */
void Dataset::initBlockReading()
{
    hsize_t prod = 1;
    Vector blockDims(dims.getLength(), 1);
    Vector blockDimsLast(dims.getLength(), 1);
    numberOfBlocksInDims = dims;

    hsize_t c = 0;
    bool diffSizeFlag = false;
    realNumberOfElementsToLoad = getNumberOfElmsToLoad();

    for (hsize_t i = dims.getLength(); i > 0; i--) {
        hsize_t j = i - 1;
        hsize_t newProd = prod * dims[j];
        if (newProd > getNumberOfElmsToLoad()) {
            blockDims[j] = getNumberOfElmsToLoad() / prod;
            blockDimsLast[j] = blockDims[j];
            numberOfBlocksInDims[j] = dims[j] / blockDims[j];
            c = numberOfBlocksInDims[j];

            if (dims[j] % blockDims[j]) {
                blockDimsLast[j] = dims[j] % blockDims[j];
                diffSizeFlag = true;
                c++;
            }
            realNumberOfElementsToLoad = prod * blockDims[j];
            numberOfBlocksInDims[j] = c;

            break;
        }
        prod = newProd;
        blockDims[j] = dims[j];
        blockDimsLast[j] = dims[j];
        numberOfBlocksInDims[j] = 1;
    }

    numberOfBlocks = 1;
    for (hsize_t i = 0; i < dims.getLength(); i++) {
        hsize_t p = dims[i] / blockDims[i];
        if (dims[i] % blockDims[i])
            p++;
        numberOfBlocks *= p;
    }

    delete[] offsets;
    delete[] counts;

    offsets = new Vector[numberOfBlocks];
    counts = new Vector[numberOfBlocks];

    //std::cout << std::endl;

    hsize_t sum = 0;
    for (hsize_t i = 0; i < numberOfBlocks; i++) {
        counts[i] = blockDims;
        if (diffSizeFlag && (i + 1) % c == 0) {
            counts[i] = blockDimsLast;
        }
        HDF5Helper::convertlinearToMultiDim(sum, offsets[i], dims);
        sum += counts[i].getSize();

        //std::cout << "count " << counts[i] << " \toffset " << offsets[i] << std::endl;
    }

    //std::cout << std::endl << numberOfElementsToLoad << std::endl;
    //std::cout << realNumberOfElementsToLoad << std::endl;
    //std::cout << "sS " << dims << std::endl;
    //std::cout << "cC " << numberOfBlocks << std::endl;
    //std::cout << "mB " << blockDims << std::endl;
    //std::cout << "mC " << numberOfBlocksInDims << std::endl;
    //std::cout << "mL " << blockDimsLast << std::endl << std::endl;
}

/**
 * @brief Checks data type and allocates memory
 * @param[out] data
 * @param[in] type
 * @param[in] size
 * @throw std::runtime_error
 */
void Dataset::checkDataTypeAndAllocation(hsize_t *&data, int type, hsize_t size)
{
    checkType(type);

    if (size > getNumberOfElmsToLoad())
        throw std::runtime_error(readErrorMessage(size, H5T_NATIVE_UINT64));

    try {
        data = new hsize_t[size](); // TODO check available memory?
        assert(data != 0 && "Bad memory allocation");
        if (data == 0)
            throw std::runtime_error("Bad memory allocation");
    } catch (std::bad_alloc) {
        throw std::runtime_error(memoryErrorMessage(size, H5T_NATIVE_UINT64));
    }
}

/**
 * @brief Checks data type and allocates memory
 * @param[out] data
 * @param[in] type
 * @param[in] size
 * @throw std::runtime_error
 */
void Dataset::checkDataTypeAndAllocation(float *&data, int type, hsize_t size)
{
    checkType(type);

    if (size > getNumberOfElmsToLoad())
        throw std::runtime_error(readErrorMessage(size, H5T_NATIVE_FLOAT));

    try {
        data = new float[size](); // TODO check available memory?
        assert(data != 0 && "Bad memory allocation");
        if (data == 0)
            throw std::runtime_error("Bad memory allocation");
    } catch (std::bad_alloc) {
        throw std::runtime_error(memoryErrorMessage(size, H5T_NATIVE_FLOAT));
    }
}

/**
 * @brief Checks datatype (H5T_NATIVE_FLOAT or H5T_NATIVE_UINT64)
 * @param[in] type Datatype
 */
void Dataset::checkType(int type)
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
void Dataset::checkFloatType()
{
    checkType(H5T_NATIVE_FLOAT);
}

/**
 * @brief Checks integer type
 */
void Dataset::checkIntegerType()
{
    checkType(H5T_NATIVE_UINT64);
}

/**
 * @brief Is float type?
 * @return True/False
 */
bool Dataset::isFloatType() const
{
    return H5Tequal(datatype, H5T_NATIVE_FLOAT) != 0;
}

/**
 * @brief Is integer type?
 * @return True/False
 */
bool Dataset::isIntegerType() const
{
    return H5Tequal(datatype, H5T_NATIVE_UINT64) != 0;
}

/**
 * @brief Returns datatype as string (H5T_NATIVE_FLOAT or H5T_NATIVE_UINT64)
 * @param[in] type Datatype
 * @return Datatype as string
 */
std::string Dataset::dataTypeString(int type) const
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
std::string Dataset::memoryErrorMessage(hsize_t size, int type) const
{
    return "There is not enough memory to allocate dataset (dataset size: " + std::to_string(size) + " " + dataTypeString(type) + ")";
}

/**
 * @brief Returns read error message
 * @param[in] size Dataset size
 * @param[in] type Datatype
 * @return Read error message
 */
std::string Dataset::readErrorMessage(hsize_t size, int type) const
{
    return "Can not read the entire dataset, size: " + std::to_string(size) + " " + dataTypeString(type) + " (max size: " + std::to_string(getNumberOfElmsToLoad()) + " " + dataTypeString(type) + ")";
}

/**
 * @brief Prints reading dataset and block message
 * @param[in] block Block index
 */
void Dataset::printsReadingMessage(hsize_t block) const
{
    std::cout << "Reading dataset " << getName();
    if (block)
        std::cout << ", block " << block << "/" << numberOfBlocks;
    std::cout << " ..." << std::endl;
}
}
