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
    chunkDims = HDF5Vector(rank);

    int dimsCount = H5Sget_simple_extent_dims(dataspace, dims.getVectorPtr(), NULL);
    if (dimsCount < 0){
        throw std::runtime_error("H5Sget_simple_extent_dims error");
    }

    plist = H5Dget_create_plist(dataset);
    if (plist < 0){
        throw std::runtime_error("H5Dget_create_plist error");
    }
    if (H5D_CHUNKED == H5Pget_layout(plist)) {
        int chunkCount = H5Pget_chunk(plist, static_cast<int>(dims.getLength()), chunkDims.getVectorPtr());
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
    offsets = 0;
    counts = 0;
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

std::string HDF5Dataset::getOnlyName()
{
    std::string s = name;
    std::string delimiter = "/";
    std::string token = s.substr(s.rfind(delimiter) + 1);
    if (!token.empty())
        return token;
    else
        return name;
}

/**
 * @brief HDF5Dataset::getId
 * @return id of dataset
 */
hid_t HDF5Dataset::getId()
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
    return chunkDims;
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

hsize_t HDF5Dataset::getRealNumberOfElmsToLoad()
{
    return realNumberOfElementsToLoad;
}

/**
 * @brief HDF5Dataset::getNumberOfBlocks
 * @return number of blocks for block reading
 */
hsize_t HDF5Dataset::getNumberOfBlocks()
{
    return numberOfBlocks;
}

HDF5Vector HDF5Dataset::getGeneralBlockDims()
{
    return counts[0];
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
}

void HDF5Dataset::setMaxNumberOfElmsToLoad(hsize_t size)
{
    if (size < numberOfElementsToLoad) {
        setNumberOfElmsToLoad(size);
    }
}

/**
 * @brief HDF5Dataset::getSizeOfDataPart
 * @return maximal number of elements to read in one block
 */
hsize_t HDF5Dataset::getNumberOfElmsToLoad()
{
    return numberOfElementsToLoad;
}


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

#ifdef PARALLEL_HDF5
    err = H5Pset_dxpl_mpio(plist_DATASET_XFER, type);
    if (err < 0){
        throw std::runtime_error("H5Pset_dxpl_mpio error");
    }
#endif
}

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
        readFullDataset(static_cast<void *>(data));
    } else
        throw std::runtime_error("Wrong data type of dataset (not integer)");
}

void HDF5Dataset::readDataset(HDF5Vector offset, HDF5Vector count, float *&data, float &min, float &max)
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

    readDataset(offset, count, data);

    // Find the miminum and maximum value
    HDF5Dataset::getMinAndMaxValue(data, count.getSize(), min, max);
}

void HDF5Dataset::readDataset(HDF5Vector offset, HDF5Vector count, hsize_t *&data, hsize_t &min, hsize_t &max)
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

    readDataset(offset, count, data);

    // Find the miminum and maximum value
    HDF5Dataset::getMinAndMaxValue(data, count.getSize(), min, max);
}

void HDF5Dataset::writeDataset(HDF5Vector offset, HDF5Vector count, float *data, bool log)
{
    if (!H5Tequal(datatype, H5T_NATIVE_FLOAT))
        throw std::runtime_error("Wrong data type of dataset (not float)");

    writeDataset(offset, count, static_cast<void *>(data), log);
}

void HDF5Dataset::writeDataset(HDF5Vector offset, HDF5Vector count, hsize_t *data, bool log)
{
    if (!H5Tequal(datatype, H5T_NATIVE_UINT64))
        throw std::runtime_error("Wrong data type of dataset (not integer)");

    writeDataset(offset, count, static_cast<void *>(data), log);
}

void HDF5Dataset::readBlock(const hsize_t index, HDF5Vector &offset, HDF5Vector &count, float *&data, float &min, float &max)
{
    readDataset(offsets[index], counts[index], data, min, max);
    offset = offsets[index];
    count = counts[index];}

void HDF5Dataset::readBlock(const hsize_t index, HDF5Vector &offset, HDF5Vector &count, hsize_t *&data, hsize_t &min, hsize_t &max)
{
    readDataset(offsets[index], counts[index], data, min, max);
    offset = offsets[index];
    count = counts[index];
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
    std::cout << name << " \tread time:  \t" << (t5-t4) << " ms;\tempty block" << std::endl;
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
        std::cout << name << " \tread time:  \t" << (t5-t4) << " ms;" << std::endl;
}

void HDF5Dataset::readDataset(HDF5Vector offset, HDF5Vector count, void *data)
{
    HDF5Vector mem_offset(offset.getLength());

    HDF5Dataset::checkOffsetAndCountParams(offset, count);

    hid_t dataspace = H5Dget_space(dataset);
    err = H5Sselect_hyperslab(dataspace, H5S_SELECT_SET, offset.getVectorPtr(), NULL, count.getVectorPtr(), NULL);
    if (err < 0){
        throw std::runtime_error("H5Sselect_hyperslab error");
    }
    hid_t memspace = H5Screate_simple(static_cast<int>(count.getLength()), count.getVectorPtr(), NULL);
    if (memspace < 0){
        throw std::runtime_error("H5Screate_simple error");
    }

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
    std::cout << name << " \tread time: \t" << (t5-t4) << " ms;\toffset: " << offset << ";\tcount: " << count << std::endl;

    H5Sclose(dataspace);
    H5Sclose(memspace);

    // Debug output
    if ((*hDF5File->getLogFileStream()).is_open()) {
        int r = 0;
        if (count[0] == 1) r = 0;
        if (count[1] == 1) r = 1;
        if (count[2] == 1) r = 2;
        *hDF5File->getLogFileStream() << (t5-t4) << ";"<< offset << ";" << r << std::endl;
    }
}

void HDF5Dataset::writeDataset(HDF5Vector offset, HDF5Vector count, void *data, bool log)
{
    HDF5Dataset::checkOffsetAndCountParams(offset, count);
    HDF5Vector mem_offset(offset.getLength());

    hid_t dataspace = H5Dget_space(dataset);
    err = H5Sselect_hyperslab(dataspace, H5S_SELECT_SET, offset.getVectorPtr(), NULL, count.getVectorPtr(), NULL);
    if (err < 0){
        throw std::runtime_error("H5Sselect_hyperslab error");
    }
    hid_t memspace = H5Screate_simple(static_cast<int>(count.getLength()), count.getVectorPtr(), NULL);
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
    if (log)
        t5 = getTime();

    H5Sclose(dataspace);
    H5Sclose(memspace);

    if (log)
        std::cout << name << " \twrite time:  \t" << (t5-t4) << " ms;\toffset: " << offset << ";\tcount: " << count << std::endl;
}

/**
 * @brief HDF5Dataset::checkOffsetAndCountParams Check offset and count params according to the size of the dataset
 * @param offset
 * @param count
 * @throw std::runtime_error
 */
void HDF5Dataset::checkOffsetAndCountParams(HDF5Vector offset, HDF5Vector count)
{
    if ((dims.getLength() != offset.getLength()) || (dims.getLength() != count.getLength()) || count.getLength() != getRank()) {
        throw std::runtime_error("Wrong offset or count");
    }

    for (unsigned int i = 0; i < offset.getLength(); i++) {
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
    HDF5Vector offset;
    HDF5Vector count;
    float minVFTmp;
    float maxVFTmp;
    bool first = true;
    for (hsize_t i = 0; i < numberOfBlocks; i++) {
        float *data;
        readBlock(i, offset, count, data, minVFTmp, maxVFTmp);
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
    HDF5Vector offset;
    HDF5Vector count;
    hsize_t minVITmp;
    hsize_t maxVITmp;
    bool first = true;
    for (hsize_t i = 0; i < numberOfBlocks; i++) {
        hsize_t *data;
        readBlock(i, offset, count, data, minVITmp, maxVITmp);
        if (first)
            minVI = maxVI = data[0];
        first = false;
        if (minVITmp < minVI) minVI = minVITmp;
        if (maxVITmp > maxVI) maxVI = maxVITmp;
        delete [] data; // !!!
    }
    issetGlobalMinAndMaxValue = true;
}

void HDF5Dataset::initBlockReading()
{
    hsize_t prod = 1;
    HDF5Vector blockDims(dims.getLength(), 1);
    HDF5Vector blockDimsLast(dims.getLength(), 1);
    numberOfBlocksInDims = dims;

    hsize_t c = 0;
    bool diffSizeFlag = false;
    realNumberOfElementsToLoad = numberOfElementsToLoad;
    if (dims.getSize() <= numberOfElementsToLoad) {
        numberOfElementsToLoad = dims.getSize();
        realNumberOfElementsToLoad = numberOfElementsToLoad;
    }

    for (hsize_t i = dims.getLength(); i > 0; i--) {
        hsize_t j = i - 1;
        hsize_t newProd = prod * dims[j];
        if (newProd > numberOfElementsToLoad) {
            blockDims[j] = numberOfElementsToLoad / prod;
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

    offsets = new HDF5Vector[numberOfBlocks];
    counts = new HDF5Vector[numberOfBlocks];

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

HDF5Vector HDF5Dataset::getNumberOfBlocksInDims() const
{
    return numberOfBlocksInDims;
}

}
