/*
 * @file        HDF5Dataset.cpp
 * @author      Petr Kleparnik, VUT FIT Brno, xklepa01@stud.fit.vutbr.cz
 * @version     0.0
 * @date        30 July 2014
 *
 * @brief       The implementation file containing HDF5Dataset class definition.
 *              This class is for better work with HDF5 dataset (read, write, find min/max values, ...).
 *
 * @section     Licence
 * This file is part of hdf5file library for k-Wave h5 processing
 * for preprocessing HDF5 data created by the k-Wave toolbox - http://www.k-wave.org.
 * Copyright © 2014, Petr Kleparnik, VUT FIT Brno.
 * hdf5file library is free software.
 */

#include "HDF5Dataset.h"

namespace HDF5Helper {

/**
 * @brief HDF5::HDF5Dataset
 * @param dataset dataset (H5::DataSet type)
 * @param name name of dataset
 * @param hDF5File file
 * @throw std::runtime_error
 */
HDF5Dataset::HDF5Dataset(hid_t dataset, std::string name, File *hDF5File) : HDF5Object(dataset)
{
    // Save params
    this->hDF5File = hDF5File;
    this->name = name;
    this->dataset = dataset;
    object = this->dataset;

    // Init space
    dataspace = H5Dget_space(dataset);
    if (dataspace < 0){
        throw std::runtime_error("H5Dget_space error");
    }

    // Get type
    datatype = H5Dget_type(dataset);
    if (datatype < 0){
        throw std::runtime_error("H5Dget_type error");
    }

    if (!H5Tequal(datatype, H5T_NATIVE_FLOAT) && !H5Tequal(datatype, H5T_NATIVE_UINT64))
        throw std::runtime_error("Wrong data type of dataset");

    // Get rank, dims and chunk dims
    rank = H5Sget_simple_extent_ndims(dataspace);
    if (rank < 0){
        throw std::runtime_error("H5Sget_simple_extent_ndims error");
    }

    //if (rank > 3){
    //    throw std::runtime_error("Wrong dataset rank");
    //}
    dims = HDF5Vector(rank);
    chunk_dims = HDF5Vector(rank);

    int dimsCount = H5Sget_simple_extent_dims(dataspace, dims.getVectorPtr(), NULL);
    if (dimsCount < 0){
        throw std::runtime_error("H5Sget_simple_extent_dims error");
    }

    plist = H5Dget_create_plist(dataset);
    if (plist < 0){
        throw std::runtime_error("H5Dget_create_plist error");
    }
    if (H5D_CHUNKED == H5Pget_layout(plist)) {
        int chunkCount = H5Pget_chunk(plist, (int) dims.getLength(), chunk_dims.getVectorPtr());
        if (chunkCount < 0){
            throw std::runtime_error("H5Pget_chunk error");
        }
    }

    H5Pclose(plist);

    plist_DATASET_XFER = H5Pcreate(H5P_DATASET_XFER);
    if (plist_DATASET_XFER < 0){
        throw std::runtime_error("H5Pcreate error");
    }

    // Init min/max
    maxVF = 0;
    minVF = 0;
    maxVI = 0;
    minVI = 0;

    // Init some flags for block reading
    blockInitialized = false;
    lastBlock = false;
    blockSize = 0;
    numberOfElementsToLoad = hDF5File->getNumberOfElmsToLoad();
    setNumberOfElmsToLoad(numberOfElementsToLoad);

    // Min/max flag
    issetGlobalMinAndMaxValue = false;
}

/**
 * @brief HDF5Dataset::~HDF5Dataset
 */
HDF5Dataset::~HDF5Dataset()
{
    std::cout << "Closing dataset \"" << name << "\"";
    H5Pclose(plist_DATASET_XFER);
    //free(convBuffer);
    //free(bkgBuffer);
    H5Sclose(dataspace);
    H5Tclose(datatype);
    H5Dclose(dataset);
    std::cout << " ... OK" << std::endl;
}

/**
 * @brief HDF5Dataset::getName
 * @return name of dataset
 */
std::string HDF5Dataset::getName()
{
    return name;
}

/**
 * @brief HDF5Dataset::getId
 * @return id of dataset
 */
hsize_t HDF5Dataset::getId()
{
    return dataset;
}

/**
 * @brief HDF5Dataset::getRank
 * @return rank of dataset
 */
hsize_t HDF5Dataset::getRank()
{
    return dims.getLength();
}

/**
 * @brief HDF5Dataset::getDims
 * @return dimensions of dataset (array)
 */
HDF5Vector HDF5Dataset::getDims()
{
    return dims;
}

/**
 * @brief HDF5Dataset::getChunkDims
 * @return chunk dimensions of dataset
 */
HDF5Vector HDF5Dataset::getChunkDims()
{
    return chunk_dims;
}

/**
 * @brief HDF5Dataset::getSize
 * @return size of dataset (number of all elements)
 */
hsize_t HDF5Dataset::getSize()
{
    return dims.getSize();
}

/**
 * @brief HDF5Dataset::getDataType
 * @return data type of dataset (type)
 */
H5T_class_t HDF5Dataset::getDataTypeClass()
{
    return H5Tget_class(datatype);
}

/**
 * @brief HDF5Dataset::getGlobalMaxValueI Get global maximum integer value
 * @param reset (true - repeats search in dataset, false - read attribute)
 * @return max integer value
 */
hsize_t HDF5Dataset::getGlobalMaxValueI(bool reset)
{
    if (!H5Tequal(datatype, H5T_NATIVE_UINT64))
        throw std::runtime_error("Wrong data type of dataset (not integer)");
    if (issetGlobalMinAndMaxValue != true)
        findGlobalMinAndMaxValue(reset);
    return maxVI;
}

/**
 * @brief HDF5Dataset::getGlobalMinValueI Get global minimum integer value
 * @param reset (true - repeats search in dataset, false - read attribute)
 * @return min integer value
 */
hsize_t HDF5Dataset::getGlobalMinValueI(bool reset)
{
    if (!H5Tequal(datatype, H5T_NATIVE_UINT64))
        throw std::runtime_error("Wrong data type of dataset (not integer)");
    if (issetGlobalMinAndMaxValue != true)
        findGlobalMinAndMaxValue(reset);
    return minVI;
}

/**
 * @brief HDF5Dataset::getGlobalMaxValueF Get global minimum float value
 * @param reset (true - repeats search in dataset, false - read attribute)
 * @return max float value
 */
float HDF5Dataset::getGlobalMaxValueF(bool reset)
{
    if (!H5Tequal(datatype, H5T_NATIVE_FLOAT))
        throw std::runtime_error("Wrong data type of dataset (not float)");
    if (issetGlobalMinAndMaxValue != true)
        findGlobalMinAndMaxValue(reset);
    return maxVF;
}

/**
 * @brief HDF5Dataset::getGlobalMinValueF Get global minimum float value
 * @param reset (true - repeats search in dataset, false - read attribute)
 * @return min float value
 */
float HDF5Dataset::getGlobalMinValueF(bool reset)
{
    if (!H5Tequal(datatype, H5T_NATIVE_FLOAT))
        throw std::runtime_error("Wrong data type of dataset (not float)");
    if (issetGlobalMinAndMaxValue != true)
        findGlobalMinAndMaxValue(reset);
    return minVF;
}

/**
 * @brief HDF5Dataset::getMinAndMaxValue Find min/max value in float data
 * @param data
 * @param size
 * @param [out] minVF
 * @param [out] maxVF
 */
void HDF5Dataset::getMinAndMaxValue(const float *data, const hsize_t size, float &minVF, float &maxVF)
{
    maxVF = minVF = data[0];
    for (hsize_t i = 0; i < size; i++) {
        float value = data[i];
        if (value < minVF) minVF = value;
        if (value > maxVF) maxVF = value;
    }
}

/**
 * @brief HDF5Dataset::getMinAndMaxValue Find min/max value in hsize_t data
 * @param data
 * @param size
 * @param [out] minVI
 * @param [out] maxVI
 */
void HDF5Dataset::getMinAndMaxValue(const hsize_t *data, const hsize_t size, hsize_t &minVI, hsize_t &maxVI)
{
    maxVI = minVI = data[0];
    for (hsize_t i = 0; i < size; i++) {
        hsize_t value = data[i];
        if (value < minVI) minVI = value;
        if (value > maxVI) maxVI = value;
    }
}

/**
 * @brief HDF5Dataset::findAndSetGlobalMinAndMaxValue Find and set (setAttribute) global min and max value
 * @param reset (true - repeats search in dataset, false - read attribute)
 */
void HDF5Dataset::findAndSetGlobalMinAndMaxValue(bool reset)
{
    if (H5Tequal(datatype, H5T_NATIVE_FLOAT)) {
        if (reset) {
            HDF5Dataset::findGlobalMinAndMaxValueF();
            HDF5Dataset::setAttribute("min", minVF);
            HDF5Dataset::setAttribute("max", maxVF);
        } else {
            if (this->hasAttribute("min") && this->hasAttribute("max")) {
                minVF = HDF5Dataset::readAttributeF("min");
                maxVF = HDF5Dataset::readAttributeF("max");
                issetGlobalMinAndMaxValue = true;
            } else {
                HDF5Dataset::findGlobalMinAndMaxValueF();
                HDF5Dataset::setAttribute("min", minVF);
                HDF5Dataset::setAttribute("max", maxVF);
            }
        }
    } else {
        if (reset) {
            HDF5Dataset::findGlobalMinAndMaxValueI();
            HDF5Dataset::setAttribute("min", minVI);
            HDF5Dataset::setAttribute("max", maxVI);
        } else {
            if (this->hasAttribute("min") && this->hasAttribute("max")) {
                minVI = HDF5Dataset::readAttributeI("min");
                maxVI = HDF5Dataset::readAttributeI("max");
                issetGlobalMinAndMaxValue = true;
            } else {
                HDF5Dataset::findGlobalMinAndMaxValueI();
                HDF5Dataset::setAttribute("min", minVI);
                HDF5Dataset::setAttribute("max", maxVI);
            }
        }
    }
}

/**
 * @brief HDF5Dataset::findGlobalMinAndMaxValue Find global min and max value
 * @param reset (true - repeats search in dataset, false - read attribute)
 */
void HDF5Dataset::findGlobalMinAndMaxValue(bool reset)
{
    if (H5Tequal(datatype, H5T_NATIVE_FLOAT)) {
        if (reset) {
            HDF5Dataset::findGlobalMinAndMaxValueF();
        } else {
            if (this->hasAttribute("min") && this->hasAttribute("max")) {
                minVF = HDF5Dataset::readAttributeF("min");
                maxVF = HDF5Dataset::readAttributeF("max");
                issetGlobalMinAndMaxValue = true;
            } else {
                HDF5Dataset::findGlobalMinAndMaxValueF();
            }
        }
    } else {
        if (reset) {
            HDF5Dataset::findGlobalMinAndMaxValueI();
        } else {
            if (this->hasAttribute("min") && this->hasAttribute("max")) {
                minVI = HDF5Dataset::readAttributeI("min");
                maxVI = HDF5Dataset::readAttributeI("max");
                issetGlobalMinAndMaxValue = true;
            } else {
                HDF5Dataset::findGlobalMinAndMaxValueI();
            }
        }
    }
}

/**
 * @brief HDF5Dataset::getRealNumberOfElmsToLoad
 * @return real number of elements used in block reading
 */
hsize_t HDF5Dataset::getRealNumberOfElmsToLoad()
{
    return blockSize;
}

/**
 * @brief HDF5Dataset::getNumberOfBlocks
 * @return number of blocks for block reading
 */
hsize_t HDF5Dataset::getNumberOfBlocks()
{
    return numberOfBlocks;
}

/**
 * @brief HDF5Dataset::getGeneralBlockDims
 * @return general dims of block used for reading
 */
HDF5Vector3D HDF5Dataset::getGeneralBlockDims()
{
    return blockDims;
}

/**
 * @brief HDF5Dataset::setSizeOfDataPart Set maximal number of elements to read in one block
 * @param size
 */
void HDF5Dataset::setNumberOfElmsToLoad(hsize_t size)
{
#ifdef PARALLEL_HDF5
    if (hDF5File->getMPISize() > 1 && size > std::numeric_limits<int>::max())
        throw std::runtime_error("setNumberOfElmsToLoad error");
#endif
    numberOfElementsToLoad = size;
    initBlockReading();
    computeNumberOfBlocks();
}

/**
 * @brief HDF5Dataset::getSizeOfDataPart
 * @return maximal number of elements to read in one block
 */
hsize_t HDF5Dataset::getNumberOfElmsToLoad()
{
    return numberOfElementsToLoad;
}

#ifdef PARALLEL_HDF5
/**
 * @brief HDF5Dataset::setMPIOAccess
 * @param type H5FD_MPIO_COLLECTIVE/H5FD_MPIO_INDEPENDENT
 */
void HDF5Dataset::setMPIOAccess(H5FD_mpio_xfer_t type)
{
    if (type == H5FD_MPIO_COLLECTIVE) {
        std::cout << "Setting H5FD_MPIO_COLLECTIVE access (" << name << ")" << std::endl;
    } else if (type == H5FD_MPIO_INDEPENDENT) {
        std::cout << "Setting H5FD_MPIO_INDEPENDENT access (" << name << ")" << std::endl;
    } else
        throw std::runtime_error("H5Pset_dxpl_mpio error - Wrong MPIO type");

    err = H5Pset_dxpl_mpio(plist_DATASET_XFER, type);
    if (err < 0){
        throw std::runtime_error("H5Pset_dxpl_mpio error");

    }
}
#endif

/**
 * @brief HDF5Dataset::readFullDataset Read full float dataset.
 * @param [out] data (pointer) memory for data is alocated in this function and must be released somewhere in future
 * @throw std::runtime_error
 */
void HDF5Dataset::readFullDataset(float *&data)
{
    if (H5Tequal(datatype, H5T_NATIVE_FLOAT)) {
        if (dims.getSize() > numberOfElementsToLoad)
            throw std::runtime_error(std::string("Can not read the entire dataset, size: " + std::to_string(dims.getSize()) + " floats (max size: " + std::to_string(numberOfElementsToLoad) + " floats)"));
        try {
            data = new float[dims.getSize()](); // TODO kontrola dostupné paměti
            if (data == nullptr)
                throw std::runtime_error(std::string("Bad memory allocation"));
        } catch (std::bad_alloc) {
            throw std::runtime_error(std::string("There is not enough memory to allocate dataset (dataset size: " + std::to_string(dims.getSize()) + " floats)").c_str());
        }
        readFullDataset(data);
    } else
        throw std::runtime_error("Wrong data type of dataset (not float)");
}

/**
 * @brief HDF5Dataset::readFullDataset Read full unsigned integer dataset.
 * @param [out] data (pointer) memory for data is alocated in this function and must be released somewhere in future
 * @throw std::runtime_error
 */
void HDF5Dataset::readFullDataset(hsize_t *&data)
{
    if (H5Tequal(datatype, H5T_NATIVE_UINT64)) {
        if (dims.getSize() > numberOfElementsToLoad)
            throw std::runtime_error(std::string("Can not read the entire dataset, size: " + std::to_string(dims.getSize()) + " unsigned 64-bit integers (max size: " + std::to_string(numberOfElementsToLoad) + " unsigned 64-bit integers)"));
        try {
            data = new hsize_t[dims.getSize()](); // TODO kontrola dostupné paměti
            if (data == nullptr)
                throw std::runtime_error(std::string("Bad memory allocation"));
        } catch (std::bad_alloc) {
            throw std::runtime_error(std::string("There is not enough memory to allocate dataset (dataset size: " + std::to_string(dims.getSize()) + " unsigned 64-bit integers)").c_str());
        }
        readFullDataset((void *) data);
    } else
        throw std::runtime_error("Wrong data type of dataset (not integer)");
}

/**
 * @brief HDF5Dataset::read3DDataset Read 3D float dataset by offset (zO, yO, xO) and count (zC, xC, yC)
 * @param offset
 * @param count
 * @param [out] data (pointer) memory for data is alocated in this function and must be released somewhere in future
 * @param [out] minVF minimum float value from data read
 * @param [out] maxVF maximum float value from data read
 * @throw std::runtime_error
 */
void HDF5Dataset::read3DDataset(HDF5Vector3D offset, HDF5Vector3D count, float *&data, float &minVF, float &maxVF)
{
    if (!H5Tequal(datatype, H5T_NATIVE_FLOAT))
        throw std::runtime_error("Wrong data type of dataset (not float)");

    if (count.getSize() > numberOfElementsToLoad)
        throw std::runtime_error(std::string("Can not read dataset, size: " + std::to_string(count.getSize()) + " floats (max size: " + std::to_string(numberOfElementsToLoad) + " floats)"));

    try {
        data = new float[count.getSize()]();
        if (data == nullptr)
            throw std::runtime_error(std::string("Bad memory allocation"));
    } catch (std::bad_alloc) {
        throw std::runtime_error(std::string("There is not enough memory to allocate dataset (dataset size: " + std::to_string(count.getSize()) + " floats)").c_str());
    }

    read3DDataset(offset, count, data);

    // Find the miminum and maximum value
    HDF5Dataset::getMinAndMaxValue(data, count.getSize(), minVF, maxVF);
}

/**
 * @brief HDF5Dataset::read3DDataset Read 3D hsize_t dataset by offset (zO, yO, xO) and count (zC, xC, yC)
 * @param offset
 * @param count
 * @param [out] data (pointer) memory for data is alocated in this function and must be released somewhere in future
 * @param [out] minVI minimum hsize_t value from data read
 * @param [out] maxVI maximum hsize_t value from data read
 * @throw std::runtime_error
 */
void HDF5Dataset::read3DDataset(HDF5Vector3D offset, HDF5Vector3D count, hsize_t *&data, hsize_t &minVI, hsize_t &maxVI)
{
    if (!H5Tequal(datatype, H5T_NATIVE_UINT64))
        throw std::runtime_error("Wrong data type of dataset (not integer)");

    if (count.getSize() > numberOfElementsToLoad)
        throw std::runtime_error(std::string("Can not read dataset, size: " + std::to_string(count.getSize()) + " unsigned 64-bit integers (max size: " + std::to_string(numberOfElementsToLoad) + " unsigned 64-bit integers)"));

    try {
        data = new hsize_t[count.getSize()]();
        if (data == nullptr)
            throw std::runtime_error(std::string("Bad memory allocation"));
    } catch (std::bad_alloc) {
        throw std::runtime_error(std::string("There is not enough memory to allocate dataset (dataset size: " + std::to_string(count.getSize()) + " unsigned 64-bit integers)").c_str());
    }

    read3DDataset(offset, count, data);

    // Find the miminum and maximum value
    HDF5Dataset::getMinAndMaxValue(data, count.getSize(), minVI, maxVI);
}

void HDF5Dataset::read4DDataset(HDF5Vector4D offset, HDF5Vector4D count, float *&data, float &minVF, float &maxVF)
{
    if (!H5Tequal(datatype, H5T_NATIVE_FLOAT))
        throw std::runtime_error("Wrong data type of dataset (not float)");

    if (count.getSize() > numberOfElementsToLoad)
        throw std::runtime_error(std::string("Can not read dataset, size: " + std::to_string(count.getSize()) + " floats (max size: " + std::to_string(numberOfElementsToLoad) + " floats)"));

    try {
        data = new float[count.getSize()]();
        if (data == nullptr)
            throw std::runtime_error(std::string("Bad memory allocation"));
    } catch (std::bad_alloc) {
        throw std::runtime_error(std::string("There is not enough memory to allocate dataset (dataset size: " + std::to_string(count.getSize()) + " floats)").c_str());
    }

    read4DDataset(offset, count, data);

    // Find the miminum and maximum value
    HDF5Dataset::getMinAndMaxValue(data, count.getSize(), minVF, maxVF);
}

/**
 * @brief HDF5Dataset::write3DDataset Write 3D float dataset by offset (zO, yO, xO) and count (zC, xC, yC)
 * @param offset
 * @param count
 * @param data data to write
 * @param log (volatile) debug flag
 * @throw std::runtime_error
 */
void HDF5Dataset::write3DDataset(HDF5Vector3D offset, HDF5Vector3D count, float *data, bool log)
{
    if (!H5Tequal(datatype, H5T_NATIVE_FLOAT))
        throw std::runtime_error("Wrong data type of dataset (not float)");

    write3DDataset(offset, count, (void *) data, log);
}

/**
 * @brief HDF5Dataset::write3DDataset Write 3D hsize_t dataset by offset (zO, yO, xO) and count (zC, xC, yC)
 * @param offset
 * @param count
 * @param data data to write
 * @param log (volatile) debug flag
 * @throw std::runtime_error
 */
void HDF5Dataset::write3DDataset(HDF5Vector3D offset, HDF5Vector3D count, hsize_t *data, bool log)
{
    if (!H5Tequal(datatype, H5T_NATIVE_UINT64))
        throw std::runtime_error("Wrong data type of dataset (not integer)");

    write3DDataset(offset, count, (void *) data, log);
}

void HDF5Dataset::write4DDataset(HDF5Vector4D offset, HDF5Vector4D count, float *data, bool log)
{
    if (!H5Tequal(datatype, H5T_NATIVE_FLOAT))
        throw std::runtime_error("Wrong data type of dataset (not float)");

    write4DDataset(offset, count, (void *) data, log);
}

/**
 * @brief HDF5Dataset::readBlock Read float data block by index
 * @param index index of the loading block
 * @param [out] offset
 * @param [out] count
 * @param [out] data (pointer) memory for data is alocated in this function and must be released somewhere in future
 * @param [out] minVFTmp minimum float value from data read
 * @param [out] maxVFTmp maximum float value from data read
 */
void HDF5Dataset::read3DBlock(const hsize_t index, HDF5Vector3D &offset, HDF5Vector3D &count, float *&data, float &minVFTmp, float &maxVFTmp)
{
    iterateToBlock(index);
    read3DDataset(this->offset, this->count, data, minVFTmp, maxVFTmp);
    count = this->count;
    offset = this->offset;
}

/**
 * @brief HDF5Dataset::readBlock Read hsize_t data block by index
 * @param index index of the loading block
 * @param [out] offset
 * @param [out] count
 * @param [out] data (pointer) memory for data is alocated in this function and must be released somewhere in future
 * @param [out] minVITmp minimum float value from data read
 * @param [out] maxVITmp maximum float value from data read
 */
void HDF5Dataset::read3DBlock(const hsize_t index, HDF5Vector3D &offset, HDF5Vector3D &count, hsize_t *&data, hsize_t &minVITmp, hsize_t &maxVITmp)
{
    iterateToBlock(index);
    read3DDataset(this->offset, this->count, data, minVITmp, maxVITmp);
    count = this->count;
    offset = this->offset;
}

/**
 * @brief HDF5Dataset::readEmptyBlock
 */
void HDF5Dataset::readEmptyBlock()
{
    hid_t dataspace = H5Dget_space(dataset);
    H5Sselect_none(dataspace);
    HDF5Vector3D count;
    hid_t memspace = H5Screate_simple(3, count.getVectorPtr(), NULL);
    H5Sselect_none(memspace);
    double t4 = 0, t5 = 0;
    t4 = getTime();
    err = H5Dread(dataset, datatype, memspace, dataspace, plist_DATASET_XFER, NULL);
    t5 = getTime();
    if (err < 0){
        throw std::runtime_error("H5Dread error");
    }
    std::cout << name << " read time: " << (t5-t4) << " ms;\tempty block" << std::endl;
}

void HDF5Dataset::readFullDataset(void *data)
{
        double t4 = getTime();
        // Reading
        err = H5Dread(dataset, datatype, H5S_ALL, H5S_ALL, plist_DATASET_XFER, data);
        if (err < 0){
            throw std::runtime_error("H5Dread error");
        }
        double t5 = getTime();
        std::cout << name << " read time: " << (t5-t4) << " ms;" << std::endl;
}

void HDF5Dataset::read3DDataset(HDF5Vector3D offset, HDF5Vector3D count, void *data)
{
    HDF5Vector3D mem_offset;

    HDF5Dataset::checkOffsetAndCountParams(offset, count);

    hid_t dataspace = H5Dget_space(dataset);
    err = H5Sselect_hyperslab(dataspace, H5S_SELECT_SET, offset.getVectorPtr(), NULL, count.getVectorPtr(), NULL);
    if (err < 0){
        throw std::runtime_error("H5Sselect_hyperslab error");
    }
    hid_t memspace = H5Screate_simple(3, count.getVectorPtr(), NULL);
    err = H5Sselect_hyperslab(memspace, H5S_SELECT_SET, mem_offset.getVectorPtr(), NULL, count.getVectorPtr(), NULL);
    if (err < 0){
        throw std::runtime_error("H5Sselect_hyperslab error");
    }

    double t4 = getTime();
    // Reading
    err = H5Dread(dataset, datatype, memspace, dataspace, plist_DATASET_XFER, data);
    if (err < 0){
        throw std::runtime_error("H5Dread error");
    }
    double t5 = getTime();
    std::cout << name << " read time: " << (t5-t4) << " ms;\toffset: " << offset.z() << " x " << offset.y() << " x " << offset.x() << ";\tcount: " << count.z() << " x " << count.y() << " x " << count.x() << std::endl;

    H5Sclose(dataspace);
    H5Sclose(memspace);

    // Debug output
    if ((*hDF5File->getLogFileStream()).is_open()) {
        int r = 0;
        if (count.z() == 1) r = 0;
        if (count.y() == 1) r = 1;
        if (count.x() == 1) r = 2;
        *hDF5File->getLogFileStream() << (t5-t4) << ";"<< offset.z() << ";" << offset.y() << ";" << offset.x() << ";" << r << std::endl;
    }
}

void HDF5Dataset::write3DDataset(HDF5Vector3D offset, HDF5Vector3D count, void *data, bool log)
{
    HDF5Dataset::checkOffsetAndCountParams(offset, count);
    HDF5Vector3D mem_offset;

    hid_t dataspace = H5Dget_space(dataset);
    err = H5Sselect_hyperslab(dataspace, H5S_SELECT_SET, offset.getVectorPtr(), NULL, count.getVectorPtr(), NULL);
    if (err < 0){
        throw std::runtime_error("H5Sselect_hyperslab error");
    }
    hid_t memspace = H5Screate_simple(3, count.getVectorPtr(), NULL);
    err = H5Sselect_hyperslab(memspace, H5S_SELECT_SET, mem_offset.getVectorPtr(), NULL, count.getVectorPtr(), NULL);
    if (err < 0){
        throw std::runtime_error("H5Sselect_hyperslab error");
    }

    double t4 = 0, t5 = 0;
    if (log)
        t4 = getTime();
    err = H5Dwrite(dataset, datatype, memspace, dataspace, plist_DATASET_XFER, data);
    if (err < 0){
        throw std::runtime_error("H5Dwrite error");
    }
    /*err = H5Fflush(dataset, H5F_SCOPE_LOCAL);
    if (err < 0){
        throw std::runtime_error("H5Fflush error");
    }*/
    if (log)
        t5 = getTime();

    H5Sclose(dataspace);
    H5Sclose(memspace);

    if (log)
        std::cout << name << " write time: " << (t5-t4) << " ms;\toffset: " << offset.z() << " x " << offset.y() << " x " << offset.x() << ";\tcount: " << count.z() << " x " << count.y() << " x " << count.x() << std::endl;

}

void HDF5Dataset::read4DDataset(HDF5Vector4D offset, HDF5Vector4D count, void *data)
{
    HDF5Vector4D mem_offset;

    HDF5Dataset::checkOffsetAndCountParams(offset, count);

    hid_t dataspace = H5Dget_space(dataset);
    err = H5Sselect_hyperslab(dataspace, H5S_SELECT_SET, offset.getVectorPtr(), NULL, count.getVectorPtr(), NULL);
    if (err < 0){
        throw std::runtime_error("H5Sselect_hyperslab error");
    }
    hid_t memspace = H5Screate_simple(4, count.getVectorPtr(), NULL);
    err = H5Sselect_hyperslab(memspace, H5S_SELECT_SET, mem_offset.getVectorPtr(), NULL, count.getVectorPtr(), NULL);
    if (err < 0){
        throw std::runtime_error("H5Sselect_hyperslab error");
    }

    double t4 = getTime();
    // Reading
    err = H5Dread(dataset, datatype, memspace, dataspace, plist_DATASET_XFER, data);
    if (err < 0){
        throw std::runtime_error("H5Dread error");
    }
    double t5 = getTime();
    std::cout << name << " read time: " << (t5-t4) << " ms;\toffset: " << offset.w() << " x " << offset.z() << " x " << offset.y() << " x " << offset.x() << ";\tcount: " << count.w() << " x " << count.z() << " x " << count.y() << " x " << count.x() << std::endl;

    H5Sclose(dataspace);
    H5Sclose(memspace);

    // Debug output
    if ((*hDF5File->getLogFileStream()).is_open()) {
        int r = 0;
        if (count.w() == 1) r = 0;
        if (count.z() == 1) r = 1;
        if (count.y() == 1) r = 2;
        if (count.x() == 1) r = 3;
        *hDF5File->getLogFileStream() << (t5-t4) << ";" << offset.w() << ";" << offset.z() << ";" << offset.y() << ";" << offset.x() << ";" << r << std::endl;
    }
}

void HDF5Dataset::write4DDataset(HDF5Vector4D offset, HDF5Vector4D count, void *data, bool log)
{
    HDF5Dataset::checkOffsetAndCountParams(offset, count);
    HDF5Vector4D mem_offset;

    hid_t dataspace = H5Dget_space(dataset);
    err = H5Sselect_hyperslab(dataspace, H5S_SELECT_SET, offset.getVectorPtr(), NULL, count.getVectorPtr(), NULL);
    if (err < 0){
        throw std::runtime_error("H5Sselect_hyperslab error");
    }
    hid_t memspace = H5Screate_simple(4, count.getVectorPtr(), NULL);
    err = H5Sselect_hyperslab(memspace, H5S_SELECT_SET, mem_offset.getVectorPtr(), NULL, count.getVectorPtr(), NULL);
    if (err < 0){
        throw std::runtime_error("H5Sselect_hyperslab error");
    }

    double t4 = 0, t5 = 0;
    if (log)
        t4 = getTime();
    err = H5Dwrite(dataset, datatype, memspace, dataspace, plist_DATASET_XFER, data);
    if (err < 0){
        throw std::runtime_error("H5Dwrite error");
    }
    /*err = H5Fflush(dataset, H5F_SCOPE_LOCAL);
    if (err < 0){
        throw std::runtime_error("H5Fflush error");
    }*/
    if (log)
        t5 = getTime();

    H5Sclose(dataspace);
    H5Sclose(memspace);

    if (log)
        std::cout << name << " write time: " << (t5-t4) << " ms;\toffset: " << offset.w() << " x " << offset.z() << " x " << offset.y() << " x " << offset.x() << ";\tcount: " << count.w() << " x " << count.z() << " x " << count.y() << " x " << count.x() << std::endl;

}

/**
 * @brief HDF5Dataset::checkOffsetAndCountParams Check offset and count params according to the size of the dataset
 * @param offset
 * @param count
 * @throw std::runtime_error
 */
void HDF5Dataset::checkOffsetAndCountParams(HDF5Vector offset, HDF5Vector count)
{
    if ((dims.getLength() != offset.getLength()) || (dims.getLength() != count.getLength())) {
        throw std::runtime_error("Wrong offset or count");
    }

    for (int i = 0; i < offset.getLength(); i++) {
        if (offset[i] >= dims[i]) throw std::runtime_error("Wrong offset - too big offset of dimension " + std::to_string(i));
        if (count[i] <= 0) throw std::runtime_error("Wrong count - too small count of dimension " + std::to_string(i));
        if (offset[i] + count[i] > dims[i]) throw std::runtime_error("Wrong count - sum of offset and count of dimension " + std::to_string(i) + " is too big");
    }
}

/**
 * @brief HDF5Dataset::findGlobalMinAndMaxValueF Find global min and max float value
 */
void HDF5Dataset::findGlobalMinAndMaxValueF()
{
    HDF5Vector3D offset;
    HDF5Vector3D count;
    float minVFTmp;
    float maxVFTmp;
    bool first = true;
    for (hsize_t i = 0; i < numberOfBlocks; i++) {
        float *data;
        read3DBlock(i, offset, count, data, minVFTmp, maxVFTmp);
        if (first)
            minVF = maxVF = data[0];
        first = false;
        if (minVFTmp < minVF) minVF = minVFTmp;
        if (maxVFTmp > maxVF) maxVF = maxVFTmp;
        delete [] data;
    }
    issetGlobalMinAndMaxValue = true;
}

/**
 * @brief HDF5Dataset::findGlobalMinAndMaxValueI Find global min and max hsize_t value
 */
void HDF5Dataset::findGlobalMinAndMaxValueI()
{
    HDF5Vector3D offset;
    HDF5Vector3D count;
    hsize_t minVITmp;
    hsize_t maxVITmp;
    bool first = true;
    for (hsize_t i = 0; i < numberOfBlocks; i++) {
        hsize_t *data;
        read3DBlock(i, offset, count, data, minVITmp, maxVITmp);
        if (first)
            minVI = maxVI = data[0];
        first = false;
        if (minVITmp < minVI) minVI = minVITmp;
        if (maxVITmp > maxVI) maxVI = maxVITmp;
        delete [] data; // !!!
    }
    issetGlobalMinAndMaxValue = true;
}

/**
 * @brief HDF5Dataset::initBlockReading Init block reading
 * @param maxSize (volatile) max block size
 */
void HDF5Dataset::initBlockReading()
{
    // Compute maximal block size to read
    blockDims.z(numberOfElementsToLoad / (dims[2] * dims[1]));
    blockDims.y((numberOfElementsToLoad % (dims[2] * dims[1])) / dims[2]);
    blockDims.x((numberOfElementsToLoad % (dims[2] * dims[1])) % dims[2]);

    blockDims.z(std::min(blockDims.z(), dims[0]));

    offset.set(0, 0, 0);

    if (blockDims.z() > 0) { // Minimal size is slab xy
        count.set(blockDims.z(), dims[1], dims[2]);
        blockSize = blockDims.z() * dims[2] * dims[1];
    } else if (blockDims.y() > 0) { // Minimal size is part of slab xy
        count.set(1, blockDims.y(), dims[2]);
        blockSize = blockDims.y() * dims[2];
    } else { // Minimal size is smaller than x size
        count.set(1, 1, blockDims.x());
        blockSize = blockDims.x();
    }

    blockInitialized = true;
    lastBlock = false;
    actualBlock = 0;
}

/**
 * @brief HDF5Dataset::computeNumberOfBlocks Compute nuber of block by iterating
 */
void HDF5Dataset::computeNumberOfBlocks()
{
    initBlockReading();
    numberOfBlocks = 0;
    do {
        recomputeBlock();
        numberOfBlocks++;
    } while (lastBlock != true);
    initBlockReading();
}

/**
 * @brief HDF5Dataset::iterateToBlock Iterate to selected index
 * @param index
 */
void HDF5Dataset::iterateToBlock(const hsize_t index)
{
    if (index >= numberOfBlocks)
        throw std::runtime_error("Wrong index - index is too big");

    if (lastBlock)
        initBlockReading();

    if (actualBlock == index)
        return;
    else if (actualBlock < index) {
        for (hsize_t i = actualBlock; i < index; i++) {
            recomputeBlock();
        }
        actualBlock = index;
    } else {
        initBlockReading();
        for (hsize_t i = 0; i < index; i++) {
            recomputeBlock();
        }
        actualBlock = index;
    }
}

/**
 * @brief HDF5Dataset::recomputeBlock Recompute next block (offset and count)
 */
void HDF5Dataset::recomputeBlock()
{
    if (blockDims.z() > 0) { // Minimal size is slab xy
        offset.z(offset.z() + count.z());
        if (offset.z() >= dims[0]) {
            lastBlock = true;
            blockInitialized = false;
            return;
        }
        if (offset.z() + count.z() > dims[0] - 1) {
            count.z(dims[0] - offset.z());
        }
    } else if (blockDims.y() > 0) { // Minimal size is part of slab xy
        offset.y(offset.y() + count.y());
        if (offset.y() >= dims[1]) {
            offset.z(offset.z() + 1);
            if (offset.z() >= dims[0]) {
                lastBlock = true;
                blockInitialized = false;
                return;
            }
            offset.y(0);
            count.y(blockDims.y());
            return;
        }
        if (offset.y() + count.y() > dims[1] - 1) {
            count.y(dims[1] - offset.y());
        }
    } else { // Minimal size is smaller than x size
        offset.x(offset.x() + count.x());
        if (offset.x() >= dims[2]) {
            offset.y(offset.y() + 1);
            if (offset.y() >= dims[1]) {
                offset.z(offset.z() + 1);
                if (offset.z() >= dims[0]) {
                    lastBlock = true;
                    blockInitialized = false;
                    return;
                }
                offset.y(0);
            }
            offset.x(0);
            count.x(blockDims.x());
            return;
        }
        if (offset.x() + count.x() > dims[2] - 1) {
            count.x(dims[2] - offset.x());
        }
    }
}

}