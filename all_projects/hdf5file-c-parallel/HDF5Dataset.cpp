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

/**
 * @brief HDF5File::HDF5Dataset::HDF5Dataset
 * @param dataset dataset (H5::DataSet type)
 * @param name name of dataset
 * @param hDF5File file
 * @throw std::runtime_error
 */
HDF5File::HDF5Dataset::HDF5Dataset(hid_t dataset, std::string name, HDF5File *hDF5File) : HDF5Object(dataset)
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
        //MPI::COMM_WORLD.Abort(1);
    }

    // Get type
    datatype = H5Dget_type(dataset);
    if (datatype < 0){
        throw std::runtime_error("H5Dget_type error");
        //MPI::COMM_WORLD.Abort(1);
    }

    if (!H5Tequal(datatype, H5T_NATIVE_FLOAT) && !H5Tequal(datatype, H5T_NATIVE_UINT64))
        throw std::runtime_error("Wrong data type of dataset");

    // Get rank, dims and chunk dims
    rank = H5Sget_simple_extent_ndims(dataspace);
    if (rank < 0){
        throw std::runtime_error("H5Sget_simple_extent_ndims error");
        //MPI::COMM_WORLD.Abort(1);
    }

    if (rank > 3){
        throw std::runtime_error("Wrong dataset rank");
        //MPI::COMM_WORLD.Abort(1);
    }

    int dimsCount = H5Sget_simple_extent_dims(dataspace, dims.getVectorPtr(), NULL);
    if (dimsCount < 0){
        throw std::runtime_error("H5Sget_simple_extent_dims error");
        //MPI::COMM_WORLD.Abort(1);
    }

    //chunk_dims = new hsize_t[rank]();
    plist = H5Dget_create_plist(dataset);
    if (plist < 0){
        throw std::runtime_error("H5Dget_create_plist error");
        //MPI::COMM_WORLD.Abort(1);
    }
    if (H5D_CHUNKED == H5Pget_layout(plist)) {
        int chunkCount = H5Pget_chunk(plist, (int) rank, chunk_dims.getVectorPtr());
        if (chunkCount < 0){
            throw std::runtime_error("H5Pget_chunk error");
            //MPI::COMM_WORLD.Abort(1);
        }
    }

    H5Pclose(plist);

    setMPIOAccess(H5FD_MPIO_COLLECTIVE);

    // Init min/max
    maxVF = 0;
    minVF = 0;
    maxVI = 0;
    minVI = 0;

    // Init some flags for block reading
    blockInitialized = false;
    lastBlock = false;
    blockSize = 0;
    sizeOfDataPart = hDF5File->getSizeOfDataPart();
    setSizeOfDataPart(sizeOfDataPart);

    // Min/max flag
    issetGlobalMinAndMaxValue = false;
}

/**
 * @brief HDF5File::HDF5Dataset::~HDF5Dataset
 */
HDF5File::HDF5Dataset::~HDF5Dataset()
{
    std::cout << "Closing dataset \"" << name << "\"";
    H5Sclose(dataspace);
    H5Tclose(datatype);
    H5Dclose(dataset);
    std::cout << " ... OK" << std::endl;
}

/**
 * @brief HDF5File::HDF5Dataset::getName
 * @return name of dataset
 */
std::string HDF5File::HDF5Dataset::getName()
{
    return name;
}

/**
 * @brief HDF5File::HDF5Dataset::getId
 * @return id of dataset
 */
hsize_t HDF5File::HDF5Dataset::getId()
{
    return dataset;
}

/**
 * @brief HDF5File::HDF5Dataset::getRank
 * @return rank of dataset
 */
hsize_t HDF5File::HDF5Dataset::getRank()
{
    return rank;
}

/**
 * @brief HDF5File::HDF5Dataset::getDims
 * @return dimensions of dataset (array)
 */
HDF5File::HDF5Vector3D HDF5File::HDF5Dataset::getDims()
{
    return dims;
}

/**
 * @brief HDF5File::HDF5Dataset::getChunkDims
 * @return chunk dimensions of dataset
 */
HDF5File::HDF5Vector3D HDF5File::HDF5Dataset::getChunkDims()
{
    return chunk_dims;
}

/**
 * @brief HDF5File::HDF5Dataset::getSize
 * @return size of dataset (number of all elements)
 */
hsize_t HDF5File::HDF5Dataset::getSize()
{
    return dims.size();
}

/**
 * @brief HDF5File::HDF5Dataset::getDataType
 * @return data type of dataset (type)
 */
H5T_class_t HDF5File::HDF5Dataset::getDataTypeClass()
{
    return H5Tget_class(datatype);
}

/**
 * @brief HDF5File::HDF5Dataset::getGlobalMaxValueI Get global maximum integer value
 * @param reset (true - repeats search in dataset, false - read attribute)
 * @return max integer value
 */
uint64_t HDF5File::HDF5Dataset::getGlobalMaxValueI(bool reset)
{
    if (!H5Tequal(datatype, H5T_NATIVE_UINT64))
        throw std::runtime_error("Wrong data type of dataset (not integer)");
    if (issetGlobalMinAndMaxValue != true)
        findGlobalMinAndMaxValue(reset);
    return maxVI;
}

/**
 * @brief HDF5File::HDF5Dataset::getGlobalMinValueI Get global minimum integer value
 * @param reset (true - repeats search in dataset, false - read attribute)
 * @return min integer value
 */
uint64_t HDF5File::HDF5Dataset::getGlobalMinValueI(bool reset)
{
    if (!H5Tequal(datatype, H5T_NATIVE_UINT64))
        throw std::runtime_error("Wrong data type of dataset (not integer)");
    if (issetGlobalMinAndMaxValue != true)
        findGlobalMinAndMaxValue(reset);
    return minVI;
}

/**
 * @brief HDF5File::HDF5Dataset::getGlobalMaxValueF Get global minimum float value
 * @param reset (true - repeats search in dataset, false - read attribute)
 * @return max float value
 */
float HDF5File::HDF5Dataset::getGlobalMaxValueF(bool reset)
{
    if (!H5Tequal(datatype, H5T_NATIVE_FLOAT))
        throw std::runtime_error("Wrong data type of dataset (not float)");
    if (issetGlobalMinAndMaxValue != true)
        findGlobalMinAndMaxValue(reset);
    return maxVF;
}

/**
 * @brief HDF5File::HDF5Dataset::getGlobalMinValueF Get global minimum float value
 * @param reset (true - repeats search in dataset, false - read attribute)
 * @return min float value
 */
float HDF5File::HDF5Dataset::getGlobalMinValueF(bool reset)
{
    if (!H5Tequal(datatype, H5T_NATIVE_FLOAT))
        throw std::runtime_error("Wrong data type of dataset (not float)");
    if (issetGlobalMinAndMaxValue != true)
        findGlobalMinAndMaxValue(reset);
    return minVF;
}

/**
 * @brief HDF5File::HDF5Dataset::setMPIOAccess
 * @param type H5FD_MPIO_COLLECTIVE/H5FD_MPIO_INDEPENDENT
 */
void HDF5File::HDF5Dataset::setMPIOAccess(H5FD_mpio_xfer_t type)
{
    plist_DATASET_XFER = H5Pcreate(H5P_DATASET_XFER);
    if (plist_DATASET_XFER < 0){
        throw std::runtime_error("H5Pcreate error");
        //MPI::COMM_WORLD.Abort(1);
    }

    if (type == H5FD_MPIO_COLLECTIVE)
        std::cout << std::endl << "Setting H5FD_MPIO_COLLECTIVE access" << std::endl;
    else if (type == H5FD_MPIO_INDEPENDENT)
        std::cout << std::endl << "Setting H5FD_MPIO_INDEPENDENT access" << std::endl;
    else
        throw std::runtime_error("H5Pset_dxpl_mpio error - Wrong MPIO type");

    err = H5Pset_dxpl_mpio(plist_DATASET_XFER, type);
    if (err < 0){
        throw std::runtime_error("H5Pset_dxpl_mpio error");
        //MPI::COMM_WORLD.Abort(1);
    }
}


/**
 * @brief HDF5File::HDF5Dataset::readFullDataset Read full float dataset.
 * @param [out] data (pointer) memory for data is alocated in this function and must be released somewhere in future
 * @throw std::runtime_error
 */
void HDF5File::HDF5Dataset::readFullDataset(float *&data)
{
    if (datatype == H5T_NATIVE_FLOAT) {
        if (dims.size() > sizeOfDataPart)
            throw std::runtime_error(std::string("Can not read the entire dataset, size: " + std::to_string(dims.size()) + " floats (max size: " + std::to_string(sizeOfDataPart) + " floats)"));
        try {
            data = new float[dims.size()](); // TODO kontrola dostupné paměti
        } catch (std::bad_alloc) {
            throw std::runtime_error(std::string("There is not enough memory to allocate dataset (dataset size: " + std::to_string(dims.size()) + " floats)").c_str());
        }
        double t4 = HDF5Helper::getTime();
        // Reading
        err = H5Dread(dataset, datatype, H5S_ALL, H5S_ALL, plist_DATASET_XFER, data);
        if (err < 0){
            throw std::runtime_error("H5Dread error");
            //MPI::COMM_WORLD.Abort(1);
        }
        double t5 = HDF5Helper::getTime();
        std::cout << name << " read time: " << (t5-t4) << " ms;" << std::endl;
    } else
        throw std::runtime_error("Wrong data type of dataset (not float)");
}

/**
 * @brief HDF5File::HDF5Dataset::readFullDataset Read full unsigned integer dataset.
 * @param [out] data (pointer) memory for data is alocated in this function and must be released somewhere in future
 * @throw std::runtime_error
 */
void HDF5File::HDF5Dataset::readFullDataset(uint64_t *&data)
{
    if (H5Tequal(datatype, H5T_NATIVE_UINT64)) {
        if (dims.size() > sizeOfDataPart)
            throw std::runtime_error(std::string("Can not read the entire dataset, size: " + std::to_string(dims.size()) + " unsigned 64-bit integers (max size: " + std::to_string(sizeOfDataPart) + " unsigned 64-bit integers)"));
        try {
            data = new uint64_t[dims.size()]();
        } catch (std::bad_alloc) {
            throw std::runtime_error(std::string("There is not enough memory to allocate dataset (dataset size: " + std::to_string(dims.size()) + " unsigned 64-bit integers)").c_str());
        }
        double t4 = HDF5Helper::getTime();
        // Read
        err = H5Dread(dataset, datatype, H5S_ALL, H5S_ALL, plist_DATASET_XFER, data);
        if (err < 0){
            throw std::runtime_error("H5Dread error");
            //MPI::COMM_WORLD.Abort(1);
        }
        double t5 = HDF5Helper::getTime();
        std::cout << name << " read time: " << (t5-t4) << " ms;" << std::endl;
    } else
        throw std::runtime_error("Wrong data type of dataset (not integer)");
}

/**
 * @brief HDF5File::HDF5Dataset::read3DDataset Read 3D float dataset by offset (zO, yO, xO) and count (zC, xC, yC)
 * @param offset
 * @param count
 * @param [out] data (pointer) memory for data is alocated in this function and must be released somewhere in future
 * @param [out] minVF minimum float value from data read
 * @param [out] maxVF maximum float value from data read
 * @throw std::runtime_error
 */
void HDF5File::HDF5Dataset::read3DDataset(HDF5Vector3D offset, HDF5Vector3D count, float *&data, float &minVF, float &maxVF)
{
    if (!H5Tequal(datatype, H5T_NATIVE_FLOAT)) throw std::runtime_error("Wrong data type of dataset (not float)");
    HDF5Dataset::checkOffsetAndCountParams(offset, count);
    HDF5Vector3D mem_offset;

    if (count.size() > sizeOfDataPart)
        throw std::runtime_error(std::string("Can not read dataset, size: " + std::to_string(count.size()) + " floats (max size: " + std::to_string(sizeOfDataPart) + " floats)"));

    hid_t dataspace = H5Dget_space(dataset);
    err = H5Sselect_hyperslab(dataspace, H5S_SELECT_SET, offset.getVectorPtr(), NULL, count.getVectorPtr(), NULL);
    if (err < 0){
        throw std::runtime_error("H5Sselect_hyperslab error");
        //MPI::COMM_WORLD.Abort(1);
    }
    hid_t memspace = H5Screate_simple(3, count.getVectorPtr(), NULL);
    err = H5Sselect_hyperslab(memspace, H5S_SELECT_SET, mem_offset.getVectorPtr(), NULL, count.getVectorPtr(), NULL);
    if (err < 0){
        throw std::runtime_error("H5Sselect_hyperslab error");
        //MPI::COMM_WORLD.Abort(1);
    }

    try {
        data = new float[count.size()]();
    } catch (std::bad_alloc) {
        throw std::runtime_error(std::string("There is not enough memory to allocate dataset (dataset size: " + std::to_string(count.size()) + " floats)").c_str());
    }

    double t4 = HDF5Helper::getTime();
    // Reading
    err = H5Dread(dataset, datatype, memspace, dataspace, plist_DATASET_XFER, data);
    if (err < 0){
        throw std::runtime_error("H5Dread error");
        //MPI::COMM_WORLD.Abort(1);
    }
    double t5 = HDF5Helper::getTime();
    std::cout << name << " read time: " << (t5-t4) << " ms; \t" << " offset: " << offset[0] << " x " << offset[1] << " x " << offset[2] << ";\tcount: " << count[0] << " x " << count[1] << " x " << count[2] << std::endl;

    H5Sclose(dataspace);
    H5Sclose(memspace);

    // Debug output
    if ((*hDF5File->getLogFileStream()).is_open()) {
        int r = 0;
        if (count[0] == 1) r = 0;
        if (count[1] == 1) r = 1;
        if (count[2] == 1) r = 2;
        *hDF5File->getLogFileStream() << (t5-t4) << ";"<< offset[0] << ";" << offset[1] << ";" << offset[2] << ";" << r << std::endl;
    }

    // Find the miminum and maximum value
    HDF5Dataset::getMinAndMaxValue(data, count.size(), minVF, maxVF);

}

/**
 * @brief HDF5File::HDF5Dataset::read3DDataset Read 3D uint64_t dataset by offset (zO, yO, xO) and count (zC, xC, yC)
 * @param offset
 * @param count
 * @param [out] data (pointer) memory for data is alocated in this function and must be released somewhere in future
 * @param [out] minVI minimum uint64_t value from data read
 * @param [out] maxVI maximum uint64_t value from data read
 * @throw std::runtime_error
 */
void HDF5File::HDF5Dataset::read3DDataset(HDF5Vector3D offset, HDF5Vector3D count, uint64_t *&data, uint64_t &minVI, uint64_t &maxVI)
{
    if (!H5Tequal(datatype, H5T_NATIVE_UINT64)) throw std::runtime_error("Wrong data type of dataset (not integer)");
    HDF5Dataset::checkOffsetAndCountParams(offset, count);
    HDF5Vector3D mem_offset;

    if (count.size() > sizeOfDataPart)
        throw std::runtime_error(std::string("Can not read dataset, size: " + std::to_string(count.size()) + " unsigned 64-bit integers (max size: " + std::to_string(sizeOfDataPart) + " unsigned 64-bit integers)"));

    hid_t dataspace = H5Dget_space(dataset);
    err = H5Sselect_hyperslab(dataspace, H5S_SELECT_SET, offset.getVectorPtr(), NULL, count.getVectorPtr(), NULL);
    if (err < 0){
        throw std::runtime_error("H5Sselect_hyperslab error");
        //MPI::COMM_WORLD.Abort(1);
    }
    hid_t memspace = H5Screate_simple(3, count.getVectorPtr(), NULL);
    err = H5Sselect_hyperslab(memspace, H5S_SELECT_SET, mem_offset.getVectorPtr(), NULL, count.getVectorPtr(), NULL);
    if (err < 0){
        throw std::runtime_error("H5Sselect_hyperslab error");
        //MPI::COMM_WORLD.Abort(1);
    }

    try {
        data = new uint64_t[count.size()]();
    } catch (std::bad_alloc) {

        throw std::runtime_error(std::string("There is not enough memory to allocate dataset (dataset size: " + std::to_string(count.size()) + " unsigned 64-bit integers)").c_str());
    }

    double t4 = HDF5Helper::getTime();
    err = H5Dread(dataset, datatype, memspace, dataspace, plist_DATASET_XFER, data);
    if (err < 0){
        throw std::runtime_error("H5Dread error");
        //MPI::COMM_WORLD.Abort(1);
    }
    double t5 = HDF5Helper::getTime();
    std::cout << name << " read time: " << (t5-t4) << " ms; \t" << " offset: " << offset[0] << " x " << offset[1] << " x " << offset[2] << ";\tcount: " << count[0] << " x " << count[1] << " x " << count[2] << std::endl;

    H5Sclose(dataspace);
    H5Sclose(memspace);

    HDF5Dataset::getMinAndMaxValue(data, count.size(), minVI, maxVI);
}

/**
 * @brief HDF5File::HDF5Dataset::write3DDataset Write 3D float dataset by offset (zO, yO, xO) and count (zC, xC, yC)
 * @param offset
 * @param count
 * @param data data to write
 * @param log (volatile) debug flag
 * @throw std::runtime_error
 */
void HDF5File::HDF5Dataset::write3DDataset(HDF5Vector3D offset, HDF5Vector3D count, float *data, bool log)
{
    if (!H5Tequal(datatype, H5T_NATIVE_FLOAT)) throw std::runtime_error("Wrong data type of dataset (not float)");
    HDF5Dataset::checkOffsetAndCountParams(offset, count);
    HDF5Vector3D mem_offset;


    hid_t dataspace = H5Dget_space(dataset);
    err = H5Sselect_hyperslab(dataspace, H5S_SELECT_SET, offset.getVectorPtr(), NULL, count.getVectorPtr(), NULL);
    if (err < 0){
        throw std::runtime_error("H5Sselect_hyperslab error");
        //MPI::COMM_WORLD.Abort(1);
    }
    hid_t memspace = H5Screate_simple(3, count.getVectorPtr(), NULL);
    err = H5Sselect_hyperslab(memspace, H5S_SELECT_SET, mem_offset.getVectorPtr(), NULL, count.getVectorPtr(), NULL);
    if (err < 0){
        throw std::runtime_error("H5Sselect_hyperslab error");
        //MPI::COMM_WORLD.Abort(1);
    }

    double t4, t5;
    if (log)
        t4 = HDF5Helper::getTime();
    err = H5Dwrite(dataset, datatype, memspace, dataspace, plist_DATASET_XFER, data);
    if (err < 0){
        throw std::runtime_error("H5Dwrite error");
        //MPI::COMM_WORLD.Abort(1);
    }
    if (log)
        t5 = HDF5Helper::getTime();

    H5Sclose(dataspace);
    H5Sclose(memspace);

    if (log)
        std::cout << name << " write time: " << (t5-t4) << " ms; \t" << " offset: " << offset[0] << " x " << offset[1] << " x " << offset[2] << ";\tcount: " << count[0] << " x " << count[1] << " x " << count[2] << std::endl;
}

/**
 * @brief HDF5File::HDF5Dataset::write3DDataset Write 3D uint64_t dataset by offset (zO, yO, xO) and count (zC, xC, yC)
 * @param offset
 * @param count
 * @param data data to write
 * @param log (volatile) debug flag
 * @throw std::runtime_error
 */
void HDF5File::HDF5Dataset::write3DDataset(HDF5Vector3D offset, HDF5Vector3D count, uint64_t *data, bool log)
{
    if (!H5Tequal(datatype, H5T_NATIVE_UINT64)) throw std::runtime_error("Wrong data type of dataset (not integer)");
    HDF5Dataset::checkOffsetAndCountParams(offset, count);
    HDF5Vector3D mem_offset;

    hid_t dataspace = H5Dget_space(dataset);
    err = H5Sselect_hyperslab(dataspace, H5S_SELECT_SET, offset.getVectorPtr(), NULL, count.getVectorPtr(), NULL);
    if (err < 0){
        throw std::runtime_error("H5Sselect_hyperslab error");
        //MPI::COMM_WORLD.Abort(1);
    }
    hid_t memspace = H5Screate_simple(3, count.getVectorPtr(), NULL);
    err = H5Sselect_hyperslab(memspace, H5S_SELECT_SET, mem_offset.getVectorPtr(), NULL, count.getVectorPtr(), NULL);
    if (err < 0){
        throw std::runtime_error("H5Sselect_hyperslab error");
        //MPI::COMM_WORLD.Abort(1);
    }

    double t4, t5;
    if (log)
        t4 = HDF5Helper::getTime();
    err = H5Dwrite(dataset, datatype, memspace, dataspace, plist_DATASET_XFER, data);
    if (err < 0){
        throw std::runtime_error("H5Dwrite error");
        //MPI::COMM_WORLD.Abort(1);
    }
    if (log)
        t5 = HDF5Helper::getTime();

    H5Sclose(dataspace);
    H5Sclose(memspace);

    if (log)
        std::cout << name << " write time: " << (t5-t4) << " ms; \t" << " offset: " << offset[0] << " x " << offset[1] << " x " << offset[2] << ";\tcount: " << count[0] << " x " << count[1] << " x " << count[2] << std::endl;
}

/**
 * @brief HDF5File::HDF5Dataset::checkOffsetAndCountParams Check offset and count params according to the size of the dataset
 * @param offset
 * @param count
 * @throw std::runtime_error
 */
void HDF5File::HDF5Dataset::checkOffsetAndCountParams(HDF5Vector3D offset, HDF5Vector3D count)
{
    if (rank != 3) throw std::runtime_error("Wrong rank - dataset is not 3D matrix");
    if (offset.z() >= dims[0]) throw std::runtime_error("Wrong offset - too big z offset");
    if (offset.y() >= dims[1]) throw std::runtime_error("Wrong offset - too big y offset");
    if (offset.x() >= dims[2]) throw std::runtime_error("Wrong offset - too big x offset");
    if (count.z() <= 0) throw std::runtime_error("Wrong count - too small z count");
    if (count.y() <= 0) throw std::runtime_error("Wrong count - too small y count");
    if (count.x() <= 0) throw std::runtime_error("Wrong count - too small x count");
    if (offset.z() + count.z() > dims[0]) throw std::runtime_error("Wrong count - sum of z offset and z count is too big");
    if (offset.y() + count.y() > dims[1]) throw std::runtime_error("Wrong count - sum of y offset and y count is too big");
    if (offset.x() + count.x() > dims[2]) throw std::runtime_error("Wrong count - sum of x offset and x count is too big");
}

/**
 * @brief HDF5File::HDF5Dataset::findAndSetGlobalMinAndMaxValue Find and set (setAttribute) global min and max value
 * @param reset (true - repeats search in dataset, false - read attribute)
 */
void HDF5File::HDF5Dataset::findAndSetGlobalMinAndMaxValue(bool reset)
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
 * @brief HDF5File::HDF5Dataset::findGlobalMinAndMaxValue Find global min and max value
 * @param reset (true - repeats search in dataset, false - read attribute)
 */
void HDF5File::HDF5Dataset::findGlobalMinAndMaxValue(bool reset)
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
 * @brief HDF5File::HDF5Dataset::getBlockSize
 * @return real number of elements used in block reading
 */
hsize_t HDF5File::HDF5Dataset::getBlockSize()
{
    return blockSize;
}

/**
 * @brief HDF5File::HDF5Dataset::setSizeOfDataPart Set maximal number of elements to read in one block
 * @param size
 */
void HDF5File::HDF5Dataset::setSizeOfDataPart(uint64_t size)
{
    sizeOfDataPart = size;
    initBlockReading();
    computeNumberOfBlocks();
}

/**
 * @brief HDF5File::HDF5Dataset::getSizeOfDataPart
 * @return maximal number of elements to read in one block
 */
uint64_t HDF5File::HDF5Dataset::getSizeOfDataPart()
{
    return sizeOfDataPart;
}

/**
 * @brief HDF5File::HDF5Dataset::initBlockReading Init block reading
 * @param maxSize (volatile) max block size
 */
void HDF5File::HDF5Dataset::initBlockReading()
{
    // Compute maximal block size to read
    blockDims.z() = sizeOfDataPart / (dims[2] * dims[1]);
    blockDims.y() = (sizeOfDataPart % (dims[2] * dims[1])) / dims[2];
    blockDims.x() = (sizeOfDataPart % (dims[2] * dims[1])) % dims[2];

    blockDims.z() = std::min(blockDims.z(), dims[0]);

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
 * @brief HDF5File::HDF5Dataset::computeNumberOfBlocks Compute nuber of block by iterating
 */
void HDF5File::HDF5Dataset::computeNumberOfBlocks()
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
 * @brief HDF5File::HDF5Dataset::getNumberOfBlocks
 * @return number of blocks for block reading
 */
hsize_t HDF5File::HDF5Dataset::getNumberOfBlocks()
{
    return numberOfBlocks;
}

/**
 * @brief HDF5File::HDF5Dataset::iterateToBlock Iterate to selected index
 * @param index
 */
void HDF5File::HDF5Dataset::iterateToBlock(const hsize_t index)
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
 * @brief HDF5File::HDF5Dataset::recomputeBlock Recompute next block (offset and count)
 */
void HDF5File::HDF5Dataset::recomputeBlock()
{
    if (blockDims.z() > 0) { // Minimal size is slab xy
        offset.z() += count.z();
        if (offset.z() >= dims[0]) {
            lastBlock = true;
            blockInitialized = false;
            return;
        }
        if (offset.z() + count.z() > dims[0] - 1) {
            count.z() = dims[0] - offset.z();
        }
    } else if (blockDims.y() > 0) { // Minimal size is part of slab xy
        offset.y() += count.y();
        if (offset.y() >= dims[1]) {
            offset.z() += 1;
            if (offset.z() >= dims[0]) {
                lastBlock = true;
                blockInitialized = false;
                return;
            }
            offset.y() = 0;
            count.y() = blockDims.y();
            return;
        }
        if (offset.y() + count.y() > dims[1] - 1) {
            count.y() = dims[1] - offset.y();
        }
    } else { // Minimal size is smaller than x size
        offset.x() += count.x();
        if (offset.x() >= dims[2]) {
            offset.y() += 1;
            if (offset.y() >= dims[1]) {
                offset.z() += 1;
                if (offset.z() >= dims[0]) {
                    lastBlock = true;
                    blockInitialized = false;
                    return;
                }
                offset.y() = 0;
            }
            offset.x() = 0;
            count.x() = blockDims.x();
            return;
        }
        if (offset.x() + count.x() > dims[2] - 1) {
            count.x() = dims[2] - offset.x();
        }
    }
}

/**
 * @brief HDF5File::HDF5Dataset::readBlock Read float data block by index
 * @param index index of the loading block
 * @param [out] offset
 * @param [out] count
 * @param [out] data (pointer) memory for data is alocated in this function and must be released somewhere in future
 * @param [out] minVFTmp minimum float value from data read
 * @param [out] maxVFTmp maximum float value from data read
 */
void HDF5File::HDF5Dataset::readBlock(const hsize_t index, HDF5Vector3D &offset, HDF5Vector3D &count, float *&data, float &minVFTmp, float &maxVFTmp)
{
    iterateToBlock(index);
    read3DDataset(this->offset, this->count, data, minVFTmp, maxVFTmp);
    count.set(this->count);
    offset.set(this->offset);
}

/**
 * @brief HDF5File::HDF5Dataset::readBlock Read uint64_t data block by index
 * @param index index of the loading block
 * @param [out] offset
 * @param [out] count
 * @param [out] data (pointer) memory for data is alocated in this function and must be released somewhere in future
 * @param [out] minVITmp minimum float value from data read
 * @param [out] maxVITmp maximum float value from data read
 */
void HDF5File::HDF5Dataset::readBlock(const hsize_t index, HDF5Vector3D &offset, HDF5Vector3D &count, uint64_t *&data, uint64_t &minVITmp, uint64_t &maxVITmp)
{
    iterateToBlock(index);
    read3DDataset(this->offset, this->count, data, minVITmp, maxVITmp);
    count.set(this->count);
    offset.set(this->offset);
}

/**
 * @brief HDF5File::HDF5Dataset::readEmptyBlock
 */
void HDF5File::HDF5Dataset::readEmptyBlock()
{
    hid_t space = H5Screate(H5S_NULL);
    if (space < 0){
        throw std::runtime_error("H5Screate error");
        //MPI::COMM_WORLD.Abort(1);
    }
    err = H5Dread(dataset, datatype, space, space, plist_DATASET_XFER, NULL);
    if (err < 0){
        throw std::runtime_error("H5Dread error");
        //MPI::COMM_WORLD.Abort(1);
    }
    std::cout << name << " - read empty block" << std::endl;
}

/**
 * @brief HDF5File::HDF5Dataset::findGlobalMinAndMaxValueF Find global min and max float value
 */
void HDF5File::HDF5Dataset::findGlobalMinAndMaxValueF()
{
    HDF5Vector3D offset;
    HDF5Vector3D count;
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
 * @brief HDF5File::HDF5Dataset::findGlobalMinAndMaxValueI Find global min and max uint64_t value
 */
void HDF5File::HDF5Dataset::findGlobalMinAndMaxValueI()
{
    HDF5Vector3D offset;
    HDF5Vector3D count;
    uint64_t minVITmp;
    uint64_t maxVITmp;
    bool first = true;
    for (hsize_t i = 0; i < numberOfBlocks; i++) {
        uint64_t *data;
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

/**
 * @brief HDF5File::HDF5Dataset::getMinAndMaxValue Find min/max value in float data
 * @param data
 * @param size
 * @param [out] minVF
 * @param [out] maxVF
 */
void HDF5File::HDF5Dataset::getMinAndMaxValue(const float *data, const hsize_t size, float &minVF, float &maxVF)
{
    maxVF = minVF = data[0];
    for (hsize_t i = 0; i < size; i++) {
        float value = data[i];
        if (value < minVF) minVF = value;
        if (value > maxVF) maxVF = value;
    }
}

/**
 * @brief HDF5File::HDF5Dataset::getMinAndMaxValue Find min/max value in uint64_t data
 * @param data
 * @param size
 * @param [out] minVI
 * @param [out] maxVI
 */
void HDF5File::HDF5Dataset::getMinAndMaxValue(const uint64_t *data, const hsize_t size, uint64_t &minVI, uint64_t &maxVI)
{
    maxVI = minVI = data[0];
    for (hsize_t i = 0; i < size; i++) {
        uint64_t value = data[i];
        if (value < minVI) minVI = value;
        if (value > maxVI) maxVI = value;
    }
}
