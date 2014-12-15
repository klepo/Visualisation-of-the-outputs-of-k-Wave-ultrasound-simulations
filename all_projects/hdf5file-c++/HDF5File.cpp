/*
 * @file        HDF5File.cpp
 * @author      Petr Kleparnik, VUT FIT Brno, xklepa01@stud.fit.vutbr.cz
 * @version     0.0
 * @date        30 July 2014
 *
 * @brief       The implementation file containing HDF5File class definition.
 *              This class contains HDF5Dataset class and HDF5Group class.
 *
 * @section     Licence
 * This file is part of hdf5file library for k-Wave h5 processing
 * for preprocessing HDF5 data created by the k-Wave toolbox - http://www.k-wave.org.
 * Copyright © 2014, Petr Kleparnik, VUT FIT Brno.
 * hdf5file library is free software.
 */

#include "HDF5File.h"
#include "HDF5Dataset.h"
#include "HDF5Group.h"

const H5std_string HDF5File::NT("Nt");
const H5std_string HDF5File::NX("Nx");
const H5std_string HDF5File::NY("Ny");
const H5std_string HDF5File::NZ("Nz");
hsize_t HDF5File::ZERO_CHUNK[3];

std::mutex HDF5File::mutex;

/**
 * @brief HDF5File::HDF5File
 * @param filename path to HDF5 file
 * @param flag default: HDF5File::OPEN - open (read and write) file, HDF5File::CREATE - create new file
 * @param log enable/disable log file
 * @throw std::runtime_error
 */
HDF5File::HDF5File(std::string filename, unsigned int flag, bool log)
{
    this->filename = filename;

    sizeOfDataPart = SIZE_OF_DATA_PART;

    // Try block to detect exceptions raised by any of the calls inside it
    try {
        H5::Exception::dontPrint();
        H5::FileAccPropList access_plist = H5::FileAccPropList::DEFAULT;
        H5::FileCreatPropList create_plist = H5::FileCreatPropList::DEFAULT;
        //access_plist.setSieveBufSize(1024 * 1024 * 4);
        //access_plist.setCache(0, 1048576 * 4, 1048576 * 16, 0.75);
        //access_plist.setCache(0, 1048576 / 4, 1048576, 0.75);
        // Set chunk cache
        //access_plist.setCache(0, 1048576, 64 * 64 * 64 * 32 * 8 * 2, 0.75);
        //access_plist.setCache(0, 0, 0, 0.75);

        // Create log file
        if (log) {
            //std::srand((unsigned int) time(NULL));
            logFileStream.open(filename + "_" + std::to_string(time(NULL)) + ".log");
            logFileStream << filename << std::endl;
        }

        if (flag == HDF5File::OPEN) {
            std::cout << "Opening file \"" << filename << "\"";
            file = H5::H5File(filename, H5F_ACC_RDWR, create_plist, access_plist);
            std::cout << " ... OK " << std::endl;

            std::cout << "SieveBufSize: " << file.getAccessPlist().getSieveBufSize() << std::endl;
            int mdc_nelmts;
            size_t rdcc_nelmts;
            size_t rdcc_nbytes;
            double rdcc_w0;
            file.getAccessPlist().getCache(mdc_nelmts, rdcc_nelmts, rdcc_nbytes, rdcc_w0);
            std::cout << "mdc_nelmts: " << mdc_nelmts << std::endl;
            std::cout << "rdcc_nelmts: " << rdcc_nelmts << std::endl;
            std::cout << "rdcc_nbytes: " << rdcc_nbytes << std::endl;
            std::cout << "rdcc_w0: " << rdcc_w0 << std::endl;

            if (log) {
                logFileStream << "mdc_nelmts: " << mdc_nelmts << std::endl;
                logFileStream << "rdcc_nelmts: " << rdcc_nelmts << std::endl;
                logFileStream << "rdcc_nbytes: " << rdcc_nbytes << std::endl;
                logFileStream << "rdcc_w0: " << rdcc_w0 << std::endl;
            }

            // Load basic datasets
            insertDataset(HDF5File::NT);
            insertDataset(HDF5File::NX);
            insertDataset(HDF5File::NY);
            insertDataset(HDF5File::NZ);

            //Set dimensions
            uint64_t *data = NULL;

            openDataset(HDF5File::NT)->readFullDataset(data);
            nT = data[0];
            delete [] data;

            openDataset(HDF5File::NX)->readFullDataset(data);
            nX = data[0];
            delete [] data;

            openDataset(HDF5File::NY)->readFullDataset(data);
            nY = data[0];
            delete [] data;

            openDataset(HDF5File::NZ)->readFullDataset(data);
            nZ = data[0];
            delete [] data;

            closeDataset(HDF5File::NT);
            closeDataset(HDF5File::NX);
            closeDataset(HDF5File::NY);
            closeDataset(HDF5File::NZ);
        } else if (flag == HDF5File::CREATE) {
            std::cout << "Creating file \"" << filename << "\"";
            file = H5::H5File(filename, H5F_ACC_TRUNC, create_plist, access_plist);
            std::cout << " ... OK " << std::endl;
        } else {
            throw std::runtime_error("Wrong HDF5File flag");
        }
    }
    // catch failure caused by the H5File operations
    catch(H5::FileIException error) {
        std::cout << " ... error " << std::endl;
        error.printError();
        throw std::runtime_error(error.getCDetailMsg());
    }
    // catch failure caused by the DataSet operations
    catch(H5::DataSetIException error) {
        error.printError();
        throw std::runtime_error(error.getCDetailMsg());
    }
    // catch failure caused by the DataSpace operations
    catch(H5::DataSpaceIException error) {
        error.printError();
        throw std::runtime_error(error.getCDetailMsg());
    }
}

/**
 * @brief HDF5File::~HDF5File
 */
HDF5File::~HDF5File()
{
    // Delete all loaded datasets
    for (std::map<const H5std_string, HDF5Dataset *>::iterator it = datasets.begin(); it != datasets.end(); ++it) {
        delete it->second;
    }
    // Delete all loaded groups
    for (std::map<const H5std_string, HDF5Group *>::iterator it = groups.begin(); it != groups.end(); ++it) {
        delete it->second;
    }
    logFileStream.close();
    std::cout << "Closing file \"" << filename << "\"";
    file.close();
    std::cout << " ... OK" << std::endl;

}

/**
 * @brief HDF5File::getLogFileStream
 * @return log file stream
 */
std::ofstream *HDF5File::getLogFileStream()
{
    return &logFileStream;
}

/**
 * @brief HDF5File::insertDataset Open, create and insert dataset (HDF5Dataset) to std::map datasets
 * @param datasetName name of dataset
 * @throw std::runtime_error
 */
void HDF5File::insertDataset(const H5std_string datasetName)
{
    try {
        std::cout << "Opening dataset \"" << datasetName << "\" ";
        H5::DataSet d = file.openDataSet(datasetName);
        HDF5Dataset *hDF5Dataset = new HDF5Dataset(d, datasetName, this);
        std::cout << "... OK" << std::endl;
        datasets.insert(std::pair<const H5std_string, HDF5Dataset *>(datasetName, hDF5Dataset));
    } catch(H5::FileIException &) {
        std::cout << "... error" << std::endl;
        //error.printError();
        throw std::runtime_error(std::string("Dataset \"" + datasetName + "\" does not exist").c_str());
    }
}

/**
 * @brief HDF5File::insertGroup Open, create and insert group (HDF5Group) to std::map groups
 * @param groupName name of group
 * @throw std::runtime_error
 */
void HDF5File::insertGroup(const H5std_string groupName)
{
    try {
        std::cout << "Opening group \"" << groupName << "\" ";
        H5::Group g = file.openGroup(groupName);
        HDF5Group *hDF5Group = new HDF5Group(g, groupName, this);
        std::cout << "... OK" << std::endl;
        groups.insert(std::pair<const H5std_string, HDF5Group *>(groupName, hDF5Group));
    } catch(H5::FileIException &) {
        std::cout << "... error" << std::endl;
        //error.printError();
        throw std::runtime_error(std::string("Group \"" + groupName + "\" does not exist").c_str());
    }
}

/**
 * @brief HDF5File::createDatasetI Create new uint64_t dataset in file
 * @param datasetName name of dataset
 * @param rank
 * @param size
 * @param chunk_size
 * @param rewrite flag for rewriting existing dataset
 * @throw std::runtime_error
 */
void HDF5File::createDatasetI(const H5std_string datasetName, hsize_t rank, hsize_t *size, hsize_t *chunk_size, bool rewrite)
{
    try {
        H5::DataSpace dataspace((int) rank, size);
        H5::DataType datatype(H5::PredType::NATIVE_UINT64);
        H5::DSetCreatPropList list = H5::DSetCreatPropList::DEFAULT;

        // Set chunking
        if (chunk_size != HDF5File::ZERO_CHUNK)
            list.setChunk((int) rank, chunk_size);
        else
            list.setLayout(H5D_CONTIGUOUS);

        std::cout << "Creating dataset \"" << datasetName << "\"";
        if (rewrite) {
            try {
                file.unlink(datasetName);
                std::cout << " ... rewrite";
            } catch(H5::FileIException error) {
            }
        }
        file.createDataSet(datasetName, datatype, dataspace, list);
        std::cout << " ... OK" << std::endl;
    } catch(H5::FileIException error) {
        std::cout << " ... error" << std::endl;
        error.printError();
        throw std::runtime_error(error.getCDetailMsg());
    } catch(H5::GroupIException error) {
        std::cout << " ... error" << std::endl;
        error.printError();
        throw std::runtime_error(error.getCDetailMsg());
    }
}

/**
 * @brief HDF5File::createDatasetF Create new float dataset in file
 * @param datasetName name of dataset
 * @param rank
 * @param size
 * @param chunk_size
 * @param rewrite flag for rewriting existing dataset
 * @throw std::runtime_error
 */
void HDF5File::createDatasetF(const H5std_string datasetName, hsize_t rank, hsize_t *size, hsize_t *chunk_size, bool rewrite)
{
    try {
        H5::DataSpace dataspace((int) rank, size);
        H5::DataType datatype(H5::PredType::NATIVE_FLOAT);
        H5::DSetCreatPropList list = H5::DSetCreatPropList::DEFAULT;
        if (chunk_size != HDF5File::ZERO_CHUNK)
            list.setChunk((int) rank, chunk_size);
        else
            list.setLayout(H5D_CONTIGUOUS);
        std::cout << "Creating dataset \"" << datasetName << "\"";
        if (rewrite) {
            try {
                file.unlink(datasetName);
                std::cout << " ... rewrite";
                closeDataset(datasetName);
            } catch(H5::FileIException error) {
            }
        }
        file.createDataSet(datasetName, datatype, dataspace, list);
        std::cout << " ... OK" << std::endl;

    } catch (H5::FileIException error) {
        std::cout << " ... error" << std::endl;
        error.printError();
        throw std::runtime_error(error.getCDetailMsg());
    } catch(H5::GroupIException error) {
        std::cout << " ... error" << std::endl;
        error.printError();
        throw std::runtime_error(error.getCDetailMsg());
    }
}

/**
 * @brief HDF5File::createGroup Create new group
 * @param name
 * @param rewrite flag for rewriting existing group
 * @throw std::runtime_error
 */
void HDF5File::createGroup(const H5std_string name, bool rewrite)
{
    try {
        std::cout << "Creating group \"" << name << "\"";
        if (rewrite) {
            try {
                file.unlink(name);
                std::cout << " ... rewrite";
                closeGroup(name);
            } catch(H5::FileIException error) {
            }
        }
        file.createGroup(name);
        std::cout << " ... OK" << std::endl;
    } catch(H5::FileIException error) {
        std::cout << " ... error" << std::endl;
        error.printError();
        throw std::runtime_error(error.getCDetailMsg());
    } catch(H5::GroupIException error) {
        std::cout << " ... error" << std::endl;
        error.printError();
        throw std::runtime_error(error.getCDetailMsg());
    }
}

/**
 * @brief HDF5File::openDataset Open dataset (create new HDF5Dataset) by idx in HDF5 file
 * @param idx
 * @return dataset (HDF5Dataset)
 * @throw std::runtime_error
 */
HDF5File::HDF5Dataset *HDF5File::openDataset(hsize_t idx)
{
    H5std_string name;
    try {
        name = file.getObjnameByIdx(idx);
    } catch(H5::FileIException error) {
        error.printError();
        throw std::runtime_error(error.getCDetailMsg());
    } catch(H5::GroupIException error) {
        error.printError();
        throw std::runtime_error(error.getCDetailMsg());
    }

    if (datasets.find(name) == datasets.end()) {
        HDF5File::insertDataset(name);
        return HDF5File::openDataset(name);
    } else
        return datasets.find(name)->second;
}

/**
 * @brief HDF5File::openDataset Open dataset (create new HDF5Dataset) by datasetName in HDF5 file
 * @param datasetName
 * @return dataset (HDF5Dataset)
 * @throw std::runtime_error
 */
HDF5File::HDF5Dataset *HDF5File::openDataset(const H5std_string datasetName)
{
    if (datasets.find(datasetName) == datasets.end()) {
        HDF5File::insertDataset(datasetName);
        return HDF5File::openDataset(datasetName);
    } else
        return datasets.find(datasetName)->second;
}

/**
 * @brief HDF5File::closeDataset Close dataset with given name in HDF5 file
 * @param datasetName
 */
void HDF5File::closeDataset(const H5std_string datasetName)
{
    if (datasets.find(datasetName) != datasets.end()){
        HDF5Dataset *dataset = datasets.find(datasetName)->second;
        delete dataset;
        datasets.erase(datasets.find(datasetName));
    }
}

/**
 * @brief HDF5File::openGroup Open group (create new HDF5Group) with given name in HDF5 file
 * @param groupName
 * @return group
 * @throw std::runtime_error
 */
HDF5File::HDF5Group *HDF5File::openGroup(const H5std_string groupName)
{
    if (groups.find(groupName) == groups.end()) {
        HDF5File::insertGroup(groupName);
        return HDF5File::openGroup(groupName);
    } else
        return groups.find(groupName)->second;
}

/**
 * @brief HDF5File::openGroup Open group (create new HDF5Group) with given idx in HDF5 file
 * @param idx
 * @return group
 * @throw std::runtime_error
 */
HDF5File::HDF5Group *HDF5File::openGroup(hsize_t idx)
{
    H5std_string name;
    try {
        name = file.getObjnameByIdx(idx);

    } catch(H5::FileIException error) {
        error.printError();
        throw std::runtime_error(error.getCDetailMsg());
    } catch(H5::GroupIException error) {
        error.printError();
        throw std::runtime_error(error.getCDetailMsg());
    }

    if (groups.find(name) == groups.end()) {
        HDF5File::insertGroup(name);
        return HDF5File::openGroup(name);
    } else
        return groups.find(name)->second;
}

/**
 * @brief HDF5File::closeGroup Close group with given name in HDF5 file
 * @param groupName
 */
void HDF5File::closeGroup(const H5std_string groupName)
{
    if (groups.find(groupName) != groups.end()){
        HDF5Group *group = groups.find(groupName)->second;
        delete group;
        groups.erase(groups.find(groupName));
    }
}

/**
 * @brief HDF5File::getNumObjs Get number of objects in HDF5 file (root group)
 * @return
 */
hsize_t HDF5File::getNumObjs()
{
    hsize_t num = file.getNumObjs();
    return num;
}

/**
 * @brief HDF5File::getObjNameById Get object name by id
 * @param id
 * @return object name
 */
H5std_string HDF5File::getObjNameById(hsize_t id)
{
    return file.getObjnameByIdx(id);
}

/**
 * @brief HDF5File::getObjTypeById Get object type by id
 * @param id
 * @return object type
 */
H5G_obj_t HDF5File::getObjTypeById(hsize_t id)
{
    return file.getObjTypeByIdx(id);
}

/**
 * @brief HDF5File::getFilename
 * @return filename
 */
H5std_string HDF5File::getFilename()
{
    return filename;
}

/**
 * @brief HDF5File::getNT
 * @return Nt
 */
uint64_t HDF5File::getNT()
{
    return nT;
}

/**
 * @brief HDF5File::getNX
 * @return Nx
 */
uint64_t HDF5File::getNX()
{
    return nX;
}

/**
 * @brief HDF5File::getNY
 * @return Ny
 */
uint64_t HDF5File::getNY()
{
    return nY;
}

/**
 * @brief HDF5File::getNZ
 * @return Nz
 */
uint64_t HDF5File::getNZ()
{
    return nZ;
}

/**
 * @brief HDF5File::convertlinearTo3D Convert linear index to 3D position (z, y, x)
 * @param index 1..Nz*Ny*Nx
 * @param [out] z 0..Nz - 1
 * @param [out] y 0..Ny - 1
 * @param [out] x 0..Nx - 1
 * @throw std::runtime_error
 */
void HDF5File::convertlinearTo3D(hsize_t index, hsize_t &z, hsize_t &y, hsize_t &x)
{
    if (index > nX * nY * nZ) throw std::runtime_error("Wrong index - too big index");
    if (index == 0) throw std::runtime_error("Wrong index - too small index");

    z = (hsize_t) ceil((double) index / (nX * nY)) - 1;
    y = (hsize_t) fmod((double) index - 1, (nX * nY)) / nX;
    x = (hsize_t) fmod(fmod((double) index - 1, (nX * nY)), nX);
}

/**
 * @brief HDF5File::convert3DToLinear Convert 3D position (z, y, x) to linear index
 * @param z 0..Nz - 1
 * @param y 0..Ny - 1
 * @param x 0..Nx - 1
 * @param [out] index 1..Nz*Ny*Nx
 * @throw std::runtime_error
 */
void HDF5File::convert3DToLinear(hsize_t z, hsize_t y, hsize_t x, hsize_t &index)
{
    if (x >= nX) throw std::runtime_error("Wrong x - too big x");
    if (y >= nY) throw std::runtime_error("Wrong y - too big y");
    if (z >= nZ) throw std::runtime_error("Wrong z - too big z");

    index = x + 1 + nX * (y) + (z) * nX * nY;
}

/**
 * @brief HDF5File::setSizeOfDataPart
 * @param size
 */
void HDF5File::setSizeOfDataPart(uint64_t size)
{
    sizeOfDataPart = size;
}

/**
 * @brief HDF5File::getSizeOfDataPart
 * @return size of data part
 */
uint64_t HDF5File::getSizeOfDataPart()
{
    return sizeOfDataPart;
}

/**
 * @brief HDF5Helper::getTime
 * @return
 */
double HDF5Helper::getTime()
{
    #ifdef __unix
        timeval tv;
        gettimeofday (&tv, NULL);
        return double (tv.tv_sec) * 1000 + (tv.tv_usec) / 1000;
    #endif

    #ifdef _WIN32
        return GetTickCount();
        //SYSTEMTIME time;
        //GetSystemTime(&time);
        //return double(time.wSecond * 1000) + time.wMilliseconds;
    #endif
}
