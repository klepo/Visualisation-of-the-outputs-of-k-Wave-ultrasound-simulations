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

    dims = new hsize_t[rank]();
    int dimsCount = H5Sget_simple_extent_dims(dataspace, dims, NULL);
    if (dimsCount < 0){
        throw std::runtime_error("H5Sget_simple_extent_dims error");
        //MPI::COMM_WORLD.Abort(1);
    }

    chunk_dims = new hsize_t[rank]();
    hid_t plist = H5Dget_create_plist(dataset);
    if (plist < 0){
        throw std::runtime_error("H5Dget_create_plist error");
        //MPI::COMM_WORLD.Abort(1);
    }
    if (H5D_CHUNKED == H5Pget_layout(plist)) {
        int chunkCount = H5Pget_chunk(plist, (int) rank, chunk_dims);
        if (chunkCount < 0){
            throw std::runtime_error("H5Pget_chunk error");
            //MPI::COMM_WORLD.Abort(1);
        }
    }
    H5Pclose(plist);

    // Compute data size
    size = 1;
    for (int i = 0; i < rank; i++)
        size *= dims[i];

    // Init min/max
    maxVF = 0;
    minVF = 0;
    maxVI = 0;
    minVI = 0;

    // Init some flags for block reading
    blockInitialized = false;
    lastBlock = false;
    blockSize = 0;

    // Min/max flag
    issetGlobalMinAndMaxValue = false;
}

/**
 * @brief HDF5File::HDF5Dataset::~HDF5Dataset
 */
HDF5File::HDF5Dataset::~HDF5Dataset()
{
    std::cout << "Closing dataset \"" << name << "\"";
    delete [] dims;
    delete [] chunk_dims;
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
hsize_t *HDF5File::HDF5Dataset::getDims()
{
    return dims;
}

/**
 * @brief HDF5File::HDF5Dataset::getChunkDims
 * @return chunk dimensions of dataset
 */
hsize_t *HDF5File::HDF5Dataset::getChunkDims()
{
    return chunk_dims;
}

/**
 * @brief HDF5File::HDF5Dataset::getSize
 * @return size of dataset (number of all elements)
 */
hsize_t HDF5File::HDF5Dataset::getSize()
{
    return size;
}

/**
 * @brief HDF5File::HDF5Dataset::getDataType
 * @return data type of dataset (type)
 */
H5T_class_t HDF5File::HDF5Dataset::getDataType()
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
 * @brief HDF5File::HDF5Dataset::readFullDataset Read full float dataset.
 * @param [out] data (pointer) memory for data is alocated in this function and must be released somewhere in future
 * @throw std::runtime_error
 */
void HDF5File::HDF5Dataset::readFullDataset(float *&data)
{
    if (datatype == H5T_NATIVE_FLOAT) {
        if (size > this->hDF5File->getSizeOfDataPart())
            throw std::runtime_error(std::string("Can not read the entire dataset, size: " + std::to_string(size) + " floats (max size: " + std::to_string(this->hDF5File->getSizeOfDataPart()) + " floats)"));
        try {
            data = new float[size](); // TODO kontrola dostupné paměti
        } catch (std::bad_alloc &) {
            throw std::runtime_error(std::string("There is not enough memory to allocate dataset (dataset size: " + std::to_string(size) + " floats)").c_str());
        }
        double t4 = getTime();
        // Reading
        err = H5Dread(dataset, datatype, H5S_ALL, H5S_ALL, H5P_DEFAULT, data);
        if (err < 0){
            throw std::runtime_error("H5Dread error");
            //MPI::COMM_WORLD.Abort(1);
        }
        double t5 = getTime();
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
        if (size > this->hDF5File->getSizeOfDataPart())
            throw std::runtime_error(std::string("Can not read the entire dataset, size: " + std::to_string(size) + " unsigned 64-bit integers (max size: " + std::to_string(this->hDF5File->getSizeOfDataPart()) + " unsigned 64-bit integers)"));
        try {
            data = new uint64_t[size]();
        } catch (std::bad_alloc &) {
            throw std::runtime_error(std::string("There is not enough memory to allocate dataset (dataset size: " + std::to_string(size) + " unsigned 64-bit integers)").c_str());
        }
        double t4 = getTime();
        // Read
        err = H5Dread(dataset, datatype, H5S_ALL, H5S_ALL, H5P_DEFAULT, data);
        if (err < 0){
            throw std::runtime_error("H5Dread error");
            //MPI::COMM_WORLD.Abort(1);
        }
        double t5 = getTime();
        std::cout << name << " read time: " << (t5-t4) << " ms;" << std::endl;
    } else
        throw std::runtime_error("Wrong data type of dataset (not integer)");
}

/**
 * @brief HDF5File::HDF5Dataset::read3DDataset Read 3D float dataset by offset (zO, yO, xO) and count (zC, xC, yC)
 * @param zO
 * @param yO
 * @param xO
 * @param zC
 * @param yC
 * @param xC
 * @param [out] data (pointer) memory for data is alocated in this function and must be released somewhere in future
 * @param [out] minVF minimum float value from data read
 * @param [out] maxVF maximum float value from data read
 * @throw std::runtime_error
 */
void HDF5File::HDF5Dataset::read3DDataset(const hsize_t zO, const hsize_t yO, const hsize_t xO, const hsize_t zC, const hsize_t yC, const hsize_t xC, float *&data, float &minVF, float &maxVF)
{
    if (!H5Tequal(datatype, H5T_NATIVE_FLOAT)) throw std::runtime_error("Wrong data type of dataset (not float)");
    HDF5Dataset::checkOffsetAndCountParams(zO, yO, xO, zC, yC, xC);
    hsize_t offset[3];   // hyperslab offset in the file
    hsize_t count[3];    // size of the hyperslab in the file
    offset[0] = zO;
    offset[1] = yO;
    offset[2] = xO;
    count[0] = zC;
    count[1] = yC;
    count[2] = xC;
    hsize_t mem_offset[3];
    mem_offset[0] = 0;
    mem_offset[1] = 0;
    mem_offset[2] = 0;
    if (xC * yC * zC > this->hDF5File->getSizeOfDataPart())
        throw std::runtime_error(std::string("Can not read the entire dataset, size: " + std::to_string(xC * yC * zC) + " floats (max size: " + std::to_string(this->hDF5File->getSizeOfDataPart()) + " floats)"));

    hid_t dataspace = H5Dget_space(dataset);
    err = H5Sselect_hyperslab(dataspace, H5S_SELECT_SET, offset, NULL, count, NULL);
    if (err < 0){
        throw std::runtime_error("H5Sselect_hyperslab error");
        //MPI::COMM_WORLD.Abort(1);
    }
    hid_t memspace = H5Screate_simple(3, count, NULL);
    err = H5Sselect_hyperslab(memspace, H5S_SELECT_SET, mem_offset, NULL, count, NULL);
    if (err < 0){
        throw std::runtime_error("H5Sselect_hyperslab error");
        //MPI::COMM_WORLD.Abort(1);
    }

    try {
        data = new float[xC * yC * zC]();
    } catch (std::bad_alloc &) {
        throw std::runtime_error(std::string("There is not enough memory to allocate dataset (dataset size: " + std::to_string(xC * yC * zC) + " floats)").c_str());
    }

    double t4 = getTime();
    // Reading
    err = H5Dread(dataset, datatype, memspace, dataspace, H5P_DEFAULT, data);
    if (err < 0){
        throw std::runtime_error("H5Dread error");
        //MPI::COMM_WORLD.Abort(1);
    }
    double t5 = getTime();
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
    HDF5Dataset::getMinAndMaxValue(data, xC * yC * zC, minVF, maxVF);

}

/**
 * @brief HDF5File::HDF5Dataset::read3DDataset Read 3D uint64_t dataset by offset (zO, yO, xO) and count (zC, xC, yC)
 * @param zO
 * @param yO
 * @param xO
 * @param zC
 * @param yC
 * @param xC
 * @param [out] data (pointer) memory for data is alocated in this function and must be released somewhere in future
 * @param [out] minVI minimum uint64_t value from data read
 * @param [out] maxVI maximum uint64_t value from data read
 * @throw std::runtime_error
 */
void HDF5File::HDF5Dataset::read3DDataset(const hsize_t zO, const hsize_t yO, const hsize_t xO, const hsize_t zC, const hsize_t yC, const hsize_t xC, uint64_t *&data, uint64_t &minVI, uint64_t &maxVI)
{
    if (!H5Tequal(datatype, H5T_NATIVE_UINT64)) throw std::runtime_error("Wrong data type of dataset (not integer)");
    HDF5Dataset::checkOffsetAndCountParams(zO, yO, xO, zC, yC, xC);
    hsize_t offset[3];   // hyperslab offset in the file
    hsize_t count[3];    // size of the hyperslab in the file
    offset[0] = zO;
    offset[1] = yO;
    offset[2] = xO;
    count[0] = zC;
    count[1] = yC;
    count[2] = xC;
    hsize_t mem_offset[3];
    mem_offset[0] = 0;
    mem_offset[1] = 0;
    mem_offset[2] = 0;
    if (xC * yC * zC > this->hDF5File->getSizeOfDataPart())
        throw std::runtime_error(std::string("Can not read dataset, size: " + std::to_string(xC * yC * zC) + " unsigned 64-bit integers (max size: " + std::to_string(this->hDF5File->getSizeOfDataPart()) + " unsigned 64-bit integers)"));

    hid_t dataspace = H5Dget_space(dataset);
    err = H5Sselect_hyperslab(dataspace, H5S_SELECT_SET, offset, NULL, count, NULL);
    if (err < 0){
        throw std::runtime_error("H5Sselect_hyperslab error");
        //MPI::COMM_WORLD.Abort(1);
    }
    hid_t memspace = H5Screate_simple(3, count, NULL);
    err = H5Sselect_hyperslab(memspace, H5S_SELECT_SET, mem_offset, NULL, count, NULL);
    if (err < 0){
        throw std::runtime_error("H5Sselect_hyperslab error");
        //MPI::COMM_WORLD.Abort(1);
    }

    try {
        data = new uint64_t[xC * yC * zC]();
    } catch (std::bad_alloc &) {

        throw std::runtime_error(std::string("There is not enough memory to allocate dataset (dataset size: " + std::to_string(xC * yC * zC) + " unsigned 64-bit integers)").c_str());
    }

    double t4 = getTime();
    err = H5Dread(dataset, datatype, memspace, dataspace, H5P_DEFAULT, data);
    if (err < 0){
        throw std::runtime_error("H5Dread error");
        //MPI::COMM_WORLD.Abort(1);
    }
    double t5 = getTime();
    std::cout << name << " read time: " << (t5-t4) << " ms; \t" << " offset: " << offset[0] << " x " << offset[1] << " x " << offset[2] << ";\tcount: " << count[0] << " x " << count[1] << " x " << count[2] << std::endl;

    H5Sclose(dataspace);
    H5Sclose(memspace);

    HDF5Dataset::getMinAndMaxValue(data, xC * yC * zC, minVI, maxVI);

}

/**
 * @brief HDF5File::HDF5Dataset::write3DDataset Write 3D float dataset by offset (zO, yO, xO) and count (zC, xC, yC)
 * @param zO
 * @param yO
 * @param xO
 * @param zC
 * @param yC
 * @param xC
 * @param data data to write
 * @param log (volatile) debug flag
 * @throw std::runtime_error
 */
void HDF5File::HDF5Dataset::write3DDataset(const hsize_t zO, const hsize_t yO, const hsize_t xO, const hsize_t zC, const hsize_t yC, const hsize_t xC, float *data, bool log)
{
    if (!H5Tequal(datatype, H5T_NATIVE_FLOAT)) throw std::runtime_error("Wrong data type of dataset (not float)");
    HDF5Dataset::checkOffsetAndCountParams(zO, yO, xO, zC, yC, xC);
    hsize_t offset[3];   // hyperslab offset in the file
    hsize_t count[3];    // size of the hyperslab in the file
    offset[0] = zO;
    offset[1] = yO;
    offset[2] = xO;
    count[0] = zC;
    count[1] = yC;
    count[2] = xC;
    hsize_t mem_offset[3];
    mem_offset[0] = 0;
    mem_offset[1] = 0;
    mem_offset[2] = 0;

    hid_t dataspace = H5Dget_space(dataset);
    err = H5Sselect_hyperslab(dataspace, H5S_SELECT_SET, offset, NULL, count, NULL);
    if (err < 0){
        throw std::runtime_error("H5Sselect_hyperslab error");
        //MPI::COMM_WORLD.Abort(1);
    }
    hid_t memspace = H5Screate_simple(3, count, NULL);
    err = H5Sselect_hyperslab(memspace, H5S_SELECT_SET, mem_offset, NULL, count, NULL);
    if (err < 0){
        throw std::runtime_error("H5Sselect_hyperslab error");
        //MPI::COMM_WORLD.Abort(1);
    }

    double t4, t5;
    if (log)
        t4 = getTime();
    err = H5Dwrite(dataset, datatype, memspace, dataspace, H5P_DEFAULT, data);
    if (err < 0){
        throw std::runtime_error("H5Dread error");
        //MPI::COMM_WORLD.Abort(1);
    }
    if (log)
        t5 = getTime();

    H5Sclose(dataspace);
    H5Sclose(memspace);

    if (log)
        std::cout << name << " write time: " << (t5-t4) << " ms; \t" << " offset: " << offset[0] << " x " << offset[1] << " x " << offset[2] << ";\tcount: " << count[0] << " x " << count[1] << " x " << count[2] << std::endl;
}

/**
 * @brief HDF5File::HDF5Dataset::write3DDataset Write 3D uint64_t dataset by offset (zO, yO, xO) and count (zC, xC, yC)
 * @param zO
 * @param yO
 * @param xO
 * @param zC
 * @param yC
 * @param xC
 * @param data data to write
 * @param log (volatile) debug flag
 * @throw std::runtime_error
 */
void HDF5File::HDF5Dataset::write3DDataset(const hsize_t zO, const hsize_t yO, const hsize_t xO, const hsize_t zC, const hsize_t yC, const hsize_t xC, uint64_t *data, bool log)
{
    if (!H5Tequal(datatype, H5T_NATIVE_UINT64)) throw std::runtime_error("Wrong data type of dataset (not integer)");
    HDF5Dataset::checkOffsetAndCountParams(zO, yO, xO, zC, yC, xC);
    hsize_t offset[3];   // hyperslab offset in the file
    hsize_t count[3];    // size of the hyperslab in the file
    offset[0] = zO;
    offset[1] = yO;
    offset[2] = xO;
    count[0] = zC;
    count[1] = yC;
    count[2] = xC;
    hsize_t mem_offset[3];
    mem_offset[0] = 0;
    mem_offset[1] = 0;
    mem_offset[2] = 0;

    hid_t dataspace = H5Dget_space(dataset);
    err = H5Sselect_hyperslab(dataspace, H5S_SELECT_SET, offset, NULL, count, NULL);
    if (err < 0){
        throw std::runtime_error("H5Sselect_hyperslab error");
        //MPI::COMM_WORLD.Abort(1);
    }
    hid_t memspace = H5Screate_simple(3, count, NULL);
    err = H5Sselect_hyperslab(memspace, H5S_SELECT_SET, mem_offset, NULL, count, NULL);
    if (err < 0){
        throw std::runtime_error("H5Sselect_hyperslab error");
        //MPI::COMM_WORLD.Abort(1);
    }

    double t4, t5;
    if (log)
        t4 = getTime();
    err = H5Dwrite(dataset, datatype, memspace, dataspace, H5P_DEFAULT, data);
    if (err < 0){
        throw std::runtime_error("H5Dread error");
        //MPI::COMM_WORLD.Abort(1);
    }
    if (log)
        t5 = getTime();

    H5Sclose(dataspace);
    H5Sclose(memspace);

    if (log)
        std::cout << name << " write time: " << (t5-t4) << " ms; \t" << " offset: " << offset[0] << " x " << offset[1] << " x " << offset[2] << ";\tcount: " << count[0] << " x " << count[1] << " x " << count[2] << std::endl;
}

/**
 * @brief HDF5File::HDF5Dataset::checkOffsetAndCountParams Check offset (zO, yO, xO) and count (zC, xC, yC) params according to the size of the dataset
 * @param zO
 * @param yO
 * @param xO
 * @param zC
 * @param yC
 * @param xC
 * @throw std::runtime_error
 */
void HDF5File::HDF5Dataset::checkOffsetAndCountParams(const hsize_t zO, const hsize_t yO, const hsize_t xO, const hsize_t zC, const hsize_t yC, const hsize_t xC)
{
    if (rank != 3) throw std::runtime_error("Wrong rank - dataset is not 3D matrix");
    if (zO >= dims[0]) throw std::runtime_error("Wrong offset - too big z offset");
    if (yO >= dims[1]) throw std::runtime_error("Wrong offset - too big y offset");
    if (xO >= dims[2]) throw std::runtime_error("Wrong offset - too big x offset");
    if (zC <= 0) throw std::runtime_error("Wrong count - too small z count");
    if (yC <= 0) throw std::runtime_error("Wrong count - too small y count");
    if (xC <= 0) throw std::runtime_error("Wrong count - too small x count");
    if (zO + zC > dims[0]) throw std::runtime_error("Wrong count - sum of z offset and z count is too big");
    if (yO + yC > dims[1]) throw std::runtime_error("Wrong count - sum of y offset and y count is too big");
    if (xO + xC > dims[2]) throw std::runtime_error("Wrong count - sum of x offset and x count is too big");
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
 * @brief HDF5File::HDF5Dataset::isLastBlock
 * @return true/false
 */
bool HDF5File::HDF5Dataset::isLastBlock()
{
    return lastBlock;
}

/**
 * @brief HDF5File::HDF5Dataset::getBlockSize
 * @return size fo block
 */
hsize_t HDF5File::HDF5Dataset::getBlockSize()
{
    return blockSize;
}

/**
 * @brief HDF5File::HDF5Dataset::initBlockReading Init block reading
 * @param maxSize (volatile) max block size
 */
void HDF5File::HDF5Dataset::initBlockReading(hsize_t maxSize)
{
    // Compute maximal block size to read
    z = maxSize / (dims[2] * dims[1]);
    y = (maxSize % (dims[2] * dims[1])) / dims[2];
    x = (maxSize % (dims[2] * dims[1])) % dims[2];

    z = std::min(z, dims[0]);

    xO = 0;
    yO = 0;
    zO = 0;

    if (z > 0) { // Minimal size is slab xy
        zC = z;
        yC = dims[1];
        xC = dims[2];
        blockSize = z * dims[2] * dims[1];
    } else if (y > 0) { // Minimal size is part of slab xy
        zC = 1;
        yC = y;
        xC = dims[2];
        blockSize = y * dims[2];
    } else { // Minimal size is smaller than x size
        zC = 1;
        yC = 1;
        xC = blockSize = x;
    }

    blockInitialized = true;
    lastBlock = false;
}

/**
 * @brief HDF5File::HDF5Dataset::initBlockReading Init block reading
 * @param maxSize (volatile) max block size
 */
void HDF5File::HDF5Dataset::initBlockReading()
{
    initBlockReading(this->hDF5File->getSizeOfDataPart());
}

/**
 * @brief HDF5File::HDF5Dataset::recomputeBlock Recompute next block (offset and count)
 */
void HDF5File::HDF5Dataset::recomputeBlock()
{
    if (z > 0) { // Minimal size is slab xy
        zO += zC;
        if (zO >= dims[0]) {
            lastBlock = true;
            blockInitialized = false;
            return;
        }
        if (zO + zC > dims[0] - 1) {
            zC = dims[0] - zO;
        }
    } else if (y > 0) { // Minimal size is part of slab xy
        yO += yC;
        if (yO >= dims[1]) {
            zO += 1;
            if (zO >= dims[0]) {
                lastBlock = true;
                blockInitialized = false;
                return;
            }
            yO = 0;
            yC = y;
            return;
        }
        if (yO + yC > dims[1] - 1) {
            yC = dims[1] - yO;
        }
    } else { // Minimal size is smaller than x size
        xO += xC;
        if (xO >= dims[2]) {
            yO += 1;
            if (yO >= dims[1]) {
                zO += 1;
                if (zO >= dims[0]) {
                    lastBlock = true;
                    blockInitialized = false;
                    return;
                }
                yO = 0;
            }
            xO = 0;
            xC = x;
            return;
        }
        if (xO + xC > dims[2] - 1) {
            xC = dims[2] - xO;
        }
    }
}

/**
 * @brief HDF5File::HDF5Dataset::readBlock Read float data block and recompute offset (zO, yO, xO) and count (zC, xC, yC) for next reading
 * @param [out] zO
 * @param [out] yO
 * @param [out] xO
 * @param [out] zC
 * @param [out] yC
 * @param [out] xC
 * @param [out] data (pointer) memory for data is alocated in this function and must be released somewhere in future
 * @param [out] minVFTmp minimum float value from data read
 * @param [out] maxVFTmp maximum float value from data read
 */
void HDF5File::HDF5Dataset::readBlock(hsize_t &zO, hsize_t &yO, hsize_t &xO, hsize_t &zC, hsize_t &yC, hsize_t &xC, float *&data, float &minVFTmp, float &maxVFTmp)
{
    if (!blockInitialized) {
        initBlockReading();
    }
    read3DDataset(this->zO, this->yO, this->xO, this->zC, this->yC, this->xC, data, minVFTmp, maxVFTmp);
    zO = this->zO;
    yO = this->yO;
    xO = this->xO;
    zC = this->zC;
    yC = this->yC;
    xC = this->xC;
    recomputeBlock();
}

/**
 * @brief HDF5File::HDF5Dataset::readBlock Read uint64_t data block and recompute offset (zO, yO, xO) and count (zC, xC, yC) for next reading
 * @param [out] zO
 * @param [out] yO
 * @param [out] xO
 * @param [out] zC
 * @param [out] yC
 * @param [out] xC
 * @param [out] data (pointer) memory for data is alocated in this function and must be released somewhere in future
 * @param [out] minVITmp minimum float value from data read
 * @param [out] maxVITmp maximum float value from data read
 */
void HDF5File::HDF5Dataset::readBlock(hsize_t &zO, hsize_t &yO, hsize_t &xO, hsize_t &zC, hsize_t &yC, hsize_t &xC, uint64_t *&data, uint64_t &minVITmp, uint64_t &maxVITmp)
{
    if (!blockInitialized) {
        initBlockReading();
    }
    read3DDataset(this->zO, this->yO, this->xO, this->zC, this->yC, this->xC, data, minVITmp, maxVITmp);
    zO = this->zO;
    yO = this->yO;
    xO = this->xO;
    zC = this->zC;
    yC = this->yC;
    xC = this->xC;
    recomputeBlock();
}

/**
 * @brief HDF5File::HDF5Dataset::findGlobalMinAndMaxValueF Find global min and max float value
 */
void HDF5File::HDF5Dataset::findGlobalMinAndMaxValueF()
{
    hsize_t xO, yO, zO;
    hsize_t xC, yC, zC;
    float minVFTmp;
    float maxVFTmp;
    bool first = true;
    do {
        float *data;
        readBlock(zO, yO, xO, zC, yC, xC, data, minVFTmp, maxVFTmp);
        if (first)
            minVF = maxVF = data[0];
        first = false;
        if (minVFTmp < minVF) minVF = minVFTmp;
        if (maxVFTmp > maxVF) maxVF = maxVFTmp;
        delete [] data;
    } while (lastBlock == false);
    issetGlobalMinAndMaxValue = true;
}

/**
 * @brief HDF5File::HDF5Dataset::findGlobalMinAndMaxValueI Find global min and max uint64_t value
 */
void HDF5File::HDF5Dataset::findGlobalMinAndMaxValueI()
{
    hsize_t xO, yO, zO;
    hsize_t xC, yC, zC;
    uint64_t minVITmp;
    uint64_t maxVITmp;
    bool first = true;
    do {
        uint64_t *data;
        readBlock(zO, yO, xO, zC, yC, xC, data, minVITmp, maxVITmp);
        if (first)
            minVI = maxVI = data[0];
        first = false;
        if (minVITmp < minVI) minVI = minVITmp;
        if (maxVITmp > maxVI) maxVI = maxVITmp;
        delete [] data; // !!!
    } while (lastBlock == false);
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
