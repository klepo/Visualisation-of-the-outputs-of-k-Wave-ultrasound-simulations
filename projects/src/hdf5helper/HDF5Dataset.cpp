/**
 * @file        HDF5Dataset.cpp
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.0
 * @date        30 July      2014 (created)
 *              8  September 2016 (updated)
 *
 * @brief       The implementation file containing HDF5Dataset class definition.
 *              This class is for better work with HDF5 dataset (read, write, find min/max values, ...).
 *
 * @license     This file is partof the hdf5helper library for k-Wave h5 processing
 *              for preprocessing the HDF5 data created by the k-Wave toolbox - http://www.k-wave.org.
 *              The hdf5helper library is free software.
 *
 * @copyright   Copyright © 2016, Petr Kleparnik, VUT FIT Brno. All Rights Reserved.
 *
 */

#include "HDF5Dataset.h"

namespace HDF5Helper {

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

std::string HDF5Dataset::getName() const
{
    return name;
}

std::string HDF5Dataset::getOnlyName() const
{
    std::string s = name;
    std::string delimiter = "/";
    std::string token = s.substr(s.rfind(delimiter) + 1);
    if (!token.empty())
        return token;
    else
        return name;
}

hid_t HDF5Dataset::getId() const
{
    return dataset;
}

hsize_t HDF5Dataset::getRank() const
{
    return dims.getLength();
}

HDF5Vector HDF5Dataset::getDims() const
{
    return dims;
}

HDF5Vector HDF5Dataset::getChunkDims() const
{
    return chunkDims;
}

hsize_t HDF5Dataset::getSize() const
{
    return dims.getSize();
}

H5T_class_t HDF5Dataset::getDataTypeClass() const
{
    return H5Tget_class(datatype);
}

void HDF5Dataset::getGlobalMaxValue(hsize_t &value, bool reset)
{
    if (!H5Tequal(datatype, H5T_NATIVE_UINT64))
        throw std::runtime_error("Wrong data type of dataset (not integer)");
    if (issetGlobalMinAndMaxValue != true)
        findGlobalMinAndMaxValue(reset);
    value = maxVI;
}

void HDF5Dataset::getGlobalMinValue(hsize_t &value, bool reset)
{
    if (!H5Tequal(datatype, H5T_NATIVE_UINT64))
        throw std::runtime_error("Wrong data type of dataset (not integer)");
    if (issetGlobalMinAndMaxValue != true)
        findGlobalMinAndMaxValue(reset);
    value = minVI;
}

void HDF5Dataset::getGlobalMaxValue(float &value, bool reset)
{
    if (!H5Tequal(datatype, H5T_NATIVE_FLOAT))
        throw std::runtime_error("Wrong data type of dataset (not float)");
    if (issetGlobalMinAndMaxValue != true)
        findGlobalMinAndMaxValue(reset);
    value = maxVF;
}

void HDF5Dataset::getGlobalMinValue(float &value, bool reset)
{
    if (!H5Tequal(datatype, H5T_NATIVE_FLOAT))
        throw std::runtime_error("Wrong data type of dataset (not float)");
    if (issetGlobalMinAndMaxValue != true)
        findGlobalMinAndMaxValue(reset);
    value = minVF;
}

void HDF5Dataset::getMinAndMaxValue(const float *data, const hsize_t size, float &minVF, float &maxVF)
{
    maxVF = minVF = data[0];
    for (hsize_t i = 0; i < size; i++) {
        float value = data[i];
        if (value < minVF) minVF = value;
        if (value > maxVF) maxVF = value;
    }
}

void HDF5Dataset::getMinAndMaxValue(const hsize_t *data, const hsize_t size, hsize_t &minVI, hsize_t &maxVI)
{
    maxVI = minVI = data[0];
    for (hsize_t i = 0; i < size; i++) {
        hsize_t value = data[i];
        if (value < minVI) minVI = value;
        if (value > maxVI) maxVI = value;
    }
}

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

hsize_t HDF5Dataset::getRealNumberOfElmsToLoad() const
{
    return realNumberOfElementsToLoad;
}

hsize_t HDF5Dataset::getNumberOfBlocks() const
{
    return numberOfBlocks;
}

HDF5Vector HDF5Dataset::getGeneralBlockDims() const
{
    return counts[0];
}

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

hsize_t HDF5Dataset::getNumberOfElmsToLoad()
{
    return numberOfElementsToLoad;
}

void HDF5Dataset::setMPIOAccess(H5FD_mpio_xfer_t type)
{
    if (type == H5FD_MPIO_COLLECTIVE) {
        std::cout << "Setting H5FD_MPIO_COLLECTIVE access (" << name << ")" << std::endl;
    } else if (type == H5FD_MPIO_INDEPENDENT) {
        std::cout << "Setting H5FD_MPIO_INDEPENDENT access (" << name << ")" << std::endl;
    } else {
        throw std::runtime_error("H5Pset_dxpl_mpio error - Wrong MPIO type");
    }
    #ifdef PARALLEL_HDF5
    err = H5Pset_dxpl_mpio(plist_DATASET_XFER, type);
    if (err < 0){
        throw std::runtime_error("H5Pset_dxpl_mpio error");
    }
    #endif
}

void HDF5Dataset::readFullDataset(float *&data, bool log)
{
    readDataset(HDF5Vector(dims.getLength(), 0), dims, data, log);
}

void HDF5Dataset::readFullDataset(hsize_t *&data, bool log)
{
    readDataset(HDF5Vector(dims.getLength(), 0), dims, data, log);
}

void HDF5Dataset::readDataset(HDF5Vector offset, HDF5Vector count, float *&data, float &min, float &max, bool log)
{
    checkTypeAndAllocation(data, H5T_NATIVE_FLOAT, count.getSize());
    readDatasetGeneral(offset, count, data, log);
    HDF5Dataset::getMinAndMaxValue(data, count.getSize(), min, max);
}

void HDF5Dataset::readDataset(HDF5Vector offset, HDF5Vector count, hsize_t *&data, hsize_t &min, hsize_t &max, bool log)
{
    checkTypeAndAllocation(data, H5T_NATIVE_UINT64, count.getSize());
    readDatasetGeneral(offset, count, data, log);
    HDF5Dataset::getMinAndMaxValue(data, count.getSize(), min, max);
}

void HDF5Dataset::readDataset(HDF5Vector offset, HDF5Vector count, float *&data, bool log)
{
    checkTypeAndAllocation(data, H5T_NATIVE_FLOAT, count.getSize());
    readDatasetGeneral(offset, count, data, log);
}

void HDF5Dataset::readDataset(HDF5Vector offset, HDF5Vector count, hsize_t *&data, bool log)
{
    checkTypeAndAllocation(data, H5T_NATIVE_UINT64, count.getSize());
    readDatasetGeneral(offset, count, data, log);
}

void HDF5Dataset::writeDataset(HDF5Vector offset, HDF5Vector count, float *data, bool log)
{
    if (!H5Tequal(datatype, H5T_NATIVE_FLOAT))
        throw std::runtime_error("Wrong data type of dataset (not float)");

    writeDatasetGeneral(offset, count, static_cast<void *>(data), log);
}

void HDF5Dataset::writeDataset(HDF5Vector offset, HDF5Vector count, hsize_t *data, bool log)
{
    if (!H5Tequal(datatype, H5T_NATIVE_UINT64))
        throw std::runtime_error("Wrong data type of dataset (not integer)");

    writeDatasetGeneral(offset, count, static_cast<void *>(data), log);
}

void HDF5Dataset::readBlock(const hsize_t index, HDF5Vector &offset, HDF5Vector &count, float *&data, float &min, float &max, bool log)
{
    readDataset(offsets[index], counts[index], data, min, max, log);
    offset = offsets[index];
    count = counts[index];
}

void HDF5Dataset::readBlock(const hsize_t index, HDF5Vector &offset, HDF5Vector &count, hsize_t *&data, hsize_t &min, hsize_t &max, bool log)
{
    readDataset(offsets[index], counts[index], data, min, max, log);
    offset = offsets[index];
    count = counts[index];
}

void HDF5Dataset::readBlock(const hsize_t index, HDF5Vector &offset, HDF5Vector &count, float *&data, bool log)
{
    readDataset(offsets[index], counts[index], data, log);
    offset = offsets[index];
    count = counts[index];
}

void HDF5Dataset::readBlock(const hsize_t index, HDF5Vector &offset, HDF5Vector &count, hsize_t *&data, bool log)
{
    readDataset(offsets[index], counts[index], data, log);
    offset = offsets[index];
    count = counts[index];
}

void HDF5Dataset::readEmptyBlock()
{
    hid_t dataspace = H5Dget_space(dataset);
    H5Sselect_none(dataspace);
    HDF5Vector3D count;
    hid_t memspace = H5Screate_simple(3, count.getVectorPtr(), NULL);
    H5Sselect_none(memspace);
    double t0 = 0, t1 = 0;
    t0 = getTime();
    err = H5Dread(dataset, datatype, memspace, dataspace, plist_DATASET_XFER, NULL);
    t1 = getTime();
    if (err < 0){
        throw std::runtime_error("H5Dread error");
    }
    std::cout << name << " \tread time:  \t" << (t1 - t0) << " ms;\tempty block" << std::endl;
}

void HDF5Dataset::readDatasetGeneral(HDF5Vector offset, HDF5Vector count, void *data, bool log)
{
    HDF5Dataset::checkOffsetAndCountParams(offset, count);
    HDF5Vector mem_offset(offset.getLength());

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

    double t0 = 0, t1 = 0;

    if (log)
        t0 = getTime();

    // Reading
    err = H5Dread(dataset, datatype, memspace, dataspace, plist_DATASET_XFER, data);
    if (err < 0){
        throw std::runtime_error("H5Dread error");
    }

    if (log)
        t1 = getTime();

    H5Sclose(dataspace);
    H5Sclose(memspace);

    if (log)
        std::cout << name << " \tread time: \t" << (t1 - t0) << " ms;\toffset: " << offset << ";\tcount: " << count << std::endl;

    // Debug output
    if ((*hDF5File->getLogFileStream()).is_open()) {
        int r = 0;
        if (count[0] == 1) r = 0;
        if (count[1] == 1) r = 1;
        if (count[2] == 1) r = 2;
        *hDF5File->getLogFileStream() << (t1 - t0) << ";" << offset << ";" << r << std::endl;
    }
}

void HDF5Dataset::writeDatasetGeneral(HDF5Vector offset, HDF5Vector count, void *data, bool log)
{
    HDF5Dataset::checkOffsetAndCountParams(offset, count);
    HDF5Vector mem_offset(offset.getLength());

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

    double t0 = 0, t1 = 0;

    if (log)
        t0 = getTime();

    // Writing
    err = H5Dwrite(dataset, datatype, memspace, dataspace, plist_DATASET_XFER, data);
    if (err < 0){
        throw std::runtime_error("H5Dwrite error");
    }

    if (log)
        t1 = getTime();

    H5Sclose(dataspace);
    H5Sclose(memspace);

    if (log)
        std::cout << name << " \twrite time:  \t" << (t1 - t0) << " ms;\toffset: " << offset << ";\tcount: " << count << std::endl;
}

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
        delete [] data; // !!!
    }
    issetGlobalMinAndMaxValue = true;
}

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

void HDF5Dataset::checkTypeAndAllocation(hsize_t *&data, int type, hsize_t size)
{
    if (!H5Tequal(datatype, H5T_NATIVE_UINT64))
        throw std::runtime_error("Wrong data type of dataset (not " + typeString(type) + ")");

    if (size > numberOfElementsToLoad)
        throw std::runtime_error(readErrorMessage(size, H5T_NATIVE_UINT64));

    try {
        data = new hsize_t[size](); // TODO check available memory
        if (data == nullptr)
            throw std::runtime_error("Bad memory allocation");
    } catch (std::bad_alloc) {
        throw std::runtime_error(memoryErrorMessage(size, H5T_NATIVE_UINT64));
    }
}

void HDF5Dataset::checkTypeAndAllocation(float *&data, int type, hsize_t size)
{
    if (!H5Tequal(datatype, H5T_NATIVE_FLOAT))
        throw std::runtime_error("Wrong data type of dataset (not " + typeString(type) + ")");

    if (size > numberOfElementsToLoad)
        throw std::runtime_error(readErrorMessage(size, H5T_NATIVE_FLOAT));

    try {
        data = new float[size](); // TODO check available memory
        if (data == nullptr)
            throw std::runtime_error("Bad memory allocation");
    } catch (std::bad_alloc) {
        throw std::runtime_error(memoryErrorMessage(size, H5T_NATIVE_FLOAT));
    }
}

std::string HDF5Dataset::typeString(int type)
{
    std::string typeStr = "unknown type";
    if (type == H5T_NATIVE_UINT64)
        typeStr = "unsigned 64-bit integers";
    if (type == H5T_NATIVE_FLOAT)
        typeStr = "floats";
    return typeStr;
}

std::string HDF5Dataset::memoryErrorMessage(hsize_t size, int type)
{
    return "There is not enough memory to allocate dataset (dataset size: " + std::to_string(size) + " " + typeString(type) + ")";

}

std::string HDF5Dataset::readErrorMessage(hsize_t size, int type)
{
    return "Can not read the entire dataset, size: " + std::to_string(size) + " " + typeString(type) + " (max size: " + std::to_string(numberOfElementsToLoad) + " " + typeString(type);

}
}
