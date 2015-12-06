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

const std::string HDF5Helper::File::NT("Nt");
const std::string HDF5Helper::File::NX("Nx");
const std::string HDF5Helper::File::NY("Ny");
const std::string HDF5Helper::File::NZ("Nz");

std::mutex HDF5Helper::File::mutex;

/**
 * @brief HDF5HDF5::File::HDF5File
 * @param filename path to HDF5 file
 * @param flag default: HDF5HDF5::File::OPEN - open (read and write) file, HDF5HDF5::File::CREATE - create new file
 * @param log enable/disable log file
 * @throw std::runtime_error
 */
#ifdef PARALLEL_HDF5
HDF5Helper::File::File(std::string filename, unsigned int flag, MPI_Comm comm, MPI_Info info, bool log)
#else
HDF5Helper::File::File(std::string filename, unsigned int flag, bool log)
#endif
{
    #ifdef PARALLEL_HDF5 
        // Check MPI is initialized
        int started, error = 0;
        error = MPI_Initialized(&started);
        if (error)
            throw std::runtime_error("MPI is not initialized");
    #endif
    // Set size of memory
    //numberOfElementsToLoad = NUMBER_OF_ELEMENTS_TO_LOAD;
    numberOfElementsToLoad = (HDF5Helper::getAvailableSystemPhysicalMemory() / 2) / 4;

    // Disable error HDF5 output
    H5Eset_auto(H5E_DEFAULT, NULL, NULL);

    // Save filename
    this->filename = filename;

    // Create log file
    if (log) {
        logFileStream.open(filename + "_" + std::to_string(time(NULL)) + ".log");
        logFileStream << filename << std::endl;
    }

    // Create File Access Property List
    plist_FILE_ACCESS = H5Pcreate(H5P_FILE_ACCESS);

    #ifdef PARALLEL_HDF5
        // Get number of processes -> set NUMBER_OF_ELEMENTS_TO_LOAD to max int (MPI limit)
        MPI_Comm_size(comm, &mPISize);
        if (mPISize > 1) {
            numberOfElementsToLoad = NUMBER_OF_ELEMENTS_TO_LOAD < std::numeric_limits<int>::max() ? NUMBER_OF_ELEMENTS_TO_LOAD : std::numeric_limits<int>::max();
            err = H5Pset_fapl_mpio(plist_FILE_ACCESS, comm, info);
            if (err < 0){
                throw std::runtime_error("H5Pset_fapl_mpio error");
                //MPI::COMM_WORLD.Abort(1);
            }
        }
    #endif

    // Set cache
    err = H5Pset_cache(plist_FILE_ACCESS, 0, 0, 1024 * 1024 * 64, 0);
    if (err < 0){
        throw std::runtime_error("H5Pset_cache error");
        //MPI::COMM_WORLD.Abort(1);
    }

    //herr_t H5Pset_alignment(hid_t plist, hsize_t threshold, hsize_t alignment )
    hsize_t threshold = 0;
    hsize_t alignment = 64 * 64 * 64 * 4;
    // Set alignment
    err = H5Pset_alignment(plist_FILE_ACCESS, threshold, alignment);
    if (err < 0){
        throw std::runtime_error("H5Pset_alignment error");
        //MPI::COMM_WORLD.Abort(1);
    }

    // Open or create file
    if (flag == HDF5Helper::File::OPEN) {
        std::cout << "Opening file \"" << filename << "\" ";
        file = H5Fopen(filename.c_str(), H5F_ACC_RDWR, plist_FILE_ACCESS);
        if (file < 0) {
            std::cout << "... error" << std::endl;
            throw std::runtime_error("H5Fopen error");
            //MPI::COMM_WORLD.Abort(1);
        }
        std::cout << "... OK " << std::endl;

        // Load basic datasets
        insertDataset(HDF5Helper::File::NT);
        insertDataset(HDF5Helper::File::NX);
        insertDataset(HDF5Helper::File::NY);
        insertDataset(HDF5Helper::File::NZ);

        //Set dimensions
        hsize_t *data = NULL;

        openDataset(HDF5Helper::File::NT)->readFullDataset(data);
        nT = data[0];
        delete [] data;

        openDataset(HDF5Helper::File::NX)->readFullDataset(data);
        nX = data[0];
        delete [] data;

        openDataset(HDF5Helper::File::NY)->readFullDataset(data);
        nY = data[0];
        delete [] data;

        openDataset(HDF5Helper::File::NZ)->readFullDataset(data);
        nZ = data[0];
        delete [] data;

        closeDataset(HDF5Helper::File::NT);
        closeDataset(HDF5Helper::File::NX);
        closeDataset(HDF5Helper::File::NY);
        closeDataset(HDF5Helper::File::NZ);

    } else if (flag == HDF5Helper::File::CREATE) {
        std::cout << "Creating file \"" << filename << "\" ";
        file = H5Fcreate(filename.c_str(), H5F_ACC_TRUNC, H5P_DEFAULT, plist_FILE_ACCESS);
        if (file < 0) {
            std::cout << "... error" << std::endl;
            throw std::runtime_error("H5Fcreate error");
            //MPI::COMM_WORLD.Abort(1);
        }
        std::cout << "... OK " << std::endl;
    } else {
        throw std::runtime_error("Wrong HDF5File flag");
    }
}

/**
 * @brief HDF5HDF5::File::~HDF5File
 */
HDF5Helper::File::~File()
{
    // Delete all loaded datasets
    for (MapOfDatasets::iterator it = datasets.begin(); it != datasets.end(); ++it) {
        delete it->second;
    }
    // Delete all loaded groups
    for (MapOfGroups::iterator it = groups.begin(); it != groups.end(); ++it) {
        delete it->second;
    }
    logFileStream.close();
    std::cout << "Closing file \"" << filename << "\"";
    err = H5Pclose(plist_FILE_ACCESS);
    if (err < 0){
        throw std::runtime_error("H5Pclose error");
    }
    err = H5Fclose(file);
    if (err < 0){
        throw std::runtime_error("H5Fclose error");
    }
    std::cout << " ... OK" << std::endl;
}

/**
 * @brief HDF5HDF5::File::getLogFileStream
 * @return log file stream
 */
std::ofstream *HDF5Helper::File::getLogFileStream()
{
    return &logFileStream;
}

/**
 * @brief HDF5HDF5::File::insertDataset Open, create and insert dataset (HDF5Dataset) to std::map datasets
 * @param datasetName name of dataset
 * @throw std::runtime_error
 */
void HDF5Helper::File::insertDataset(const std::string datasetName)
{
    std::cout << "Opening dataset \"" << datasetName << "\" ";
    hid_t d = H5Dopen(file, datasetName.c_str(), H5P_DEFAULT);
    if (d < 0){
        std::cout << "... error" << std::endl;
        throw std::runtime_error("H5Dopen error");
        //MPI::COMM_WORLD.Abort(1);
    }
    HDF5Dataset *hDF5Dataset = new HDF5Dataset(d, datasetName, this);
    std::cout << "... OK" << std::endl;
    datasets.insert(std::pair<const std::string, HDF5Dataset *>(datasetName, hDF5Dataset));
}

/**
 * @brief HDF5HDF5::File::insertGroup Open, create and insert group (HDF5Group) to std::map groups
 * @param groupName name of group
 * @throw std::runtime_error
 */
void HDF5Helper::File::insertGroup(const std::string groupName)
{
    std::cout << "Opening group \"" << groupName << "\" ";
    hid_t g = H5Gopen(file, groupName.c_str(), H5P_DEFAULT);
    if (g < 0){
        std::cout << "... error" << std::endl;
        throw std::runtime_error("H5Gopen error");
        //MPI::COMM_WORLD.Abort(1);
    }
    HDF5Group *hDF5Group = new HDF5Group(g, groupName, this);
    std::cout << "... OK" << std::endl;
    groups.insert(std::pair<const std::string, HDF5Group *>(groupName, hDF5Group));
}

/**
 * @brief HDF5HDF5::File::createDatasetI Create new hsize_t dataset in file
 * @param datasetName name of dataset
 * @param rank
 * @param size
 * @param chunk_size
 * @param rewrite flag for rewriting existing dataset
 * @throw std::runtime_error
 */
void HDF5Helper::File::createDatasetI(const std::string datasetName, hsize_t rank, HDF5Vector3D size, HDF5Vector3D chunk_size, bool rewrite)
{
    HDF5Helper::File::createDataset(datasetName, H5T_NATIVE_UINT64, rank, size, chunk_size, rewrite);
}

/**
 * @brief HDF5HDF5::File::createDatasetF Create new float dataset in file
 * @param datasetName name of dataset
 * @param rank
 * @param size
 * @param chunk_size
 * @param rewrite flag for rewriting existing dataset
 * @throw std::runtime_error
 */
void HDF5Helper::File::createDatasetF(const std::string datasetName, hsize_t rank, HDF5Vector3D size, HDF5Vector3D chunk_size, bool rewrite)
{
    HDF5Helper::File::createDataset(datasetName, H5T_NATIVE_FLOAT, rank, size, chunk_size, rewrite);
}

/**
 * @brief HDF5HDF5::File::createDataset Create new dataset in file of given type
 * @param datasetName name of dataset
 * @param rank
 * @param size
 * @param chunk_size
 * @param rewrite flag for rewriting existing dataset
 * @param type (H5T_NATIVE_FLOAT | H5T_NATIVE_UINT64)
 * @throw std::runtime_error
 */
void HDF5Helper::File::createDataset(const std::string datasetName, hid_t datatype, hsize_t rank, HDF5Vector3D size, HDF5Vector3D chunk_size, bool rewrite)
{
    hid_t dataspace = H5Screate_simple((int) rank, size.getVectorPtr(), NULL);
    if (dataspace < 0){
        throw std::runtime_error("H5Screate_simple error");
        //MPI::COMM_WORLD.Abort(1);
    }
    //hid_t datatype = H5Tcopy(type);
    if (datatype < 0){
        throw std::runtime_error("H5Tcopy error");
        //MPI::COMM_WORLD.Abort(1);
    }
    hid_t plist = H5Pcreate(H5P_DATASET_CREATE);
    if (plist < 0){
        throw std::runtime_error("H5Pcreate error");
        //MPI::COMM_WORLD.Abort(1);
    }

    // Set chunking
    if (chunk_size.z() != 0 && chunk_size.y() != 0 && chunk_size.x() != 0) {
        err = H5Pset_chunk(plist, (int) rank, chunk_size.getVectorPtr());
        if (err < 0){
            throw std::runtime_error("H5Pset_chunk error");
            //MPI::COMM_WORLD.Abort(1);
        }
    } else {
        err = H5Pset_layout(plist, H5D_CONTIGUOUS);
        if (err < 0){
            throw std::runtime_error("H5Pset_layout error");
            //MPI::COMM_WORLD.Abort(1);
        }
    }

    std::cout << "Creating dataset \"" << datasetName << "\" ";
    if (rewrite) {
        H5Ldelete(file, datasetName.c_str(), H5P_DEFAULT);
        std::cout << "... rewrite";
    }

    hid_t dataset = H5Dcreate(file, datasetName.c_str(), datatype, dataspace, H5P_DEFAULT, plist, H5P_DEFAULT);
    if (dataset < 0){
        std::cout << "... error" << std::endl;
        throw std::runtime_error("H5Dcreate error");
    }

    err = H5Sclose(dataspace);
    if (err < 0){
        throw std::runtime_error("H5Sclose error");
    }
    err = H5Pclose(plist);
    if (err < 0){
        throw std::runtime_error("H5Pclose error");
    }
    err = H5Dclose(dataset);
    if (err < 0){
        throw std::runtime_error("H5Dclose error");
    }
    std::cout << "... OK" << std::endl;
}

/**
 * @brief HDF5HDF5::File::createGroup Create new group
 * @param name
 * @param rewrite flag for rewriting existing group
 * @throw std::runtime_error
 */
void HDF5Helper::File::createGroup(const std::string name, bool rewrite)
{
        std::cout << "Creating group \"" << name << "\" ";
        if (rewrite) {
            H5Ldelete(file, name.c_str(), H5P_DEFAULT);
            std::cout << "... rewrite";
        }

        hid_t group = H5Gcreate(file, name.c_str(), H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
        if (group < 0){
            std::cout << "... error" << std::endl;
            throw std::runtime_error("H5Dcreate error");
        }
        H5Gclose(group);
        if (err < 0){
            throw std::runtime_error("H5Gclose error");
        }
        std::cout << "... OK" << std::endl;
}

/**
 * @brief HDF5HDF5::File::openDataset Open dataset (create new HDF5Dataset) by idx in HDF5 file
 * @param idx
 * @return dataset (HDF5Dataset)
 * @throw std::runtime_error
 */
HDF5Helper::File::HDF5Dataset *HDF5Helper::File::openDataset(hsize_t idx)
{
    std::string name = getObjNameById(idx);

    if (datasets.find(name) == datasets.end()) {
        HDF5Helper::File::insertDataset(name);
        return HDF5Helper::File::openDataset(name);
    } else
        return datasets.find(name)->second;
}

/**
 * @brief HDF5HDF5::File::openDataset Open dataset (create new HDF5Dataset) by datasetName in HDF5 file
 * @param datasetName
 * @return dataset (HDF5Dataset)
 * @throw std::runtime_error
 */
HDF5Helper::File::HDF5Dataset *HDF5Helper::File::openDataset(const std::string datasetName)
{
    if (datasets.find(datasetName) == datasets.end()) {
        HDF5Helper::File::insertDataset(datasetName);
        return HDF5Helper::File::openDataset(datasetName);
    } else
        return datasets.find(datasetName)->second;
}

/**
 * @brief HDF5HDF5::File::closeDataset Close dataset with given name in HDF5 file
 * @param datasetName
 */
void HDF5Helper::File::closeDataset(const std::string datasetName)
{
    if (datasets.find(datasetName) != datasets.end()){
        HDF5Dataset *dataset = datasets.find(datasetName)->second;
        delete dataset;
        datasets.erase(datasets.find(datasetName));
    }
}

/**
 * @brief HDF5HDF5::File::openGroup Open group (create new HDF5Group) with given name in HDF5 file
 * @param groupName
 * @return group
 * @throw std::runtime_error
 */
HDF5Helper::File::HDF5Group *HDF5Helper::File::openGroup(const std::string groupName)
{
    if (groups.find(groupName) == groups.end()) {
        HDF5Helper::File::insertGroup(groupName);
        return HDF5Helper::File::openGroup(groupName);
    } else
        return groups.find(groupName)->second;
}

/**
 * @brief HDF5HDF5::File::openGroup Open group (create new HDF5Group) with given idx in HDF5 file
 * @param idx
 * @return group
 * @throw std::runtime_error
 */
HDF5Helper::File::HDF5Group *HDF5Helper::File::openGroup(hsize_t idx)
{
    std::string name = getObjNameById(idx);

    if (groups.find(name) == groups.end()) {
        HDF5Helper::File::insertGroup(name);
        return HDF5Helper::File::openGroup(name);
    } else
        return groups.find(name)->second;
}

/**
 * @brief HDF5HDF5::File::closeGroup Close group with given name in HDF5 file
 * @param groupName
 */
void HDF5Helper::File::closeGroup(const std::string groupName)
{
    if (groups.find(groupName) != groups.end()){
        HDF5Group *group = groups.find(groupName)->second;
        delete group;
        groups.erase(groups.find(groupName));
    }
}

/**
 * @brief HDF5HDF5::File::getNumObjs Get number of objects in HDF5 file (root group)
 * @return
 */
hsize_t HDF5Helper::File::getNumObjs()
{
    H5G_info_t group_info;
    err = H5Gget_info(file, &group_info);
    if (err < 0){
        throw std::runtime_error("H5Gget_info error");
        //MPI::COMM_WORLD.Abort(1);
    }
    return group_info.nlinks;
}

/**
 * @brief HDF5HDF5::File::getObjNameById Get object name by id
 * @param id
 * @return object name
 */
std::string HDF5Helper::File::getObjNameById(hsize_t idx)
{
    char *nameC = NULL;
    ssize_t size = 0;
    size = H5Gget_objname_by_idx(file, idx, nameC, size);
    if (size <= 0){
        throw std::runtime_error("H5Gget_objname_by_idx error");
        //MPI::COMM_WORLD.Abort(1);
    }
    nameC = new char[size + 1];
    H5Gget_objname_by_idx(file, idx, nameC, size + 1);
    std::string name(nameC);
    delete [] nameC;
    return name;
}

/**
 * @brief HDF5HDF5::File::getObjTypeById Get object type by id
 * @param id
 * @return object type
 */
H5G_obj_t HDF5Helper::File::getObjTypeById(hsize_t idx)
{
    int type = 0;
    type = H5Gget_objtype_by_idx(file, idx);
    if (type < 0){
        throw std::runtime_error("H5Gget_objtype_by_idx error");
        //MPI::COMM_WORLD.Abort(1);
    }
    return (H5G_obj_t) type;
}

/**
 * @brief HDF5HDF5::File::objExistsByName
 * @param name
 * @return true/false
 */
bool HDF5Helper::File::objExistsByName(const std::string name)
{
    if (H5Lexists(file, name.c_str(), H5P_DEFAULT))
        return H5Oexists_by_name(file, name.c_str(), H5P_DEFAULT) != 0;
    else
        return false;
}

/**
 * @brief HDF5HDF5::File::getFilename
 * @return filename
 */
std::string HDF5Helper::File::getFilename()
{
    return filename;
}

/**
 * @brief HDF5HDF5::File::getNT
 * @return Nt
 */
hsize_t HDF5Helper::File::getNT()
{
    return nT;
}

/**
 * @brief HDF5HDF5::File::getNX
 * @return Nx
 */
hsize_t HDF5Helper::File::getNX()
{
    return nX;
}

/**
 * @brief HDF5HDF5::File::getNY
 * @return Ny
 */
hsize_t HDF5Helper::File::getNY()
{
    return nY;
}

/**
 * @brief HDF5HDF5::File::getNZ
 * @return Nz
 */
hsize_t HDF5Helper::File::getNZ()
{
    return nZ;
}

/**
 * @brief HDF5HDF5::File::convertlinearTo3D Convert linear index to 3D position (z, y, x)
 * @param index 1..Nz*Ny*Nx
 * @param [out] z 0..Nz - 1
 * @param [out] y 0..Ny - 1
 * @param [out] x 0..Nx - 1
 * @throw std::runtime_error
 */
void HDF5Helper::File::convertlinearTo3D(hsize_t index, HDF5Vector3D &position)
{
    if (index > nX * nY * nZ) throw std::runtime_error("Wrong index - too big index");
    if (index == 0) throw std::runtime_error("Wrong index - too small index");

    position.z() = (hsize_t) ceil((double) index / (nX * nY)) - 1;
    position.y() = (hsize_t) fmod((double) index - 1, (nX * nY)) / nX;
    position.x() = (hsize_t) fmod(fmod((double) index - 1, (nX * nY)), nX);
}

/**
 * @brief HDF5HDF5::File::convert3DToLinear Convert 3D position (z, y, x) to linear index
 * @param z 0..Nz - 1
 * @param y 0..Ny - 1
 * @param x 0..Nx - 1
 * @param [out] index 1..Nz*Ny*Nx
 * @throw std::runtime_error
 */
void HDF5Helper::File::convert3DToLinear(HDF5Vector3D position, hsize_t &index)
{
    if (position.x() >= nX) throw std::runtime_error("Wrong x - too big x");
    if (position.y() >= nY) throw std::runtime_error("Wrong y - too big y");
    if (position.z() >= nZ) throw std::runtime_error("Wrong z - too big z");

    index = position.x() + 1 + nX * (position.y()) + (position.z()) * nX * nY;
}

/**
 * @brief HDF5HDF5::File::setSizeOfDataPart
 * @param size
 */
void HDF5Helper::File::setNumberOfElmsToLoad(hsize_t size)
{
    #ifdef PARALLEL_HDF5    
        if (mPISize > 1 && size > std::numeric_limits<int>::max())
            throw std::runtime_error("setNumberOfElmsToLoad error");
    #endif
    numberOfElementsToLoad = size;
}

/**
 * @brief HDF5HDF5::File::getSizeOfDataPart
 * @return size of data part
 */
hsize_t HDF5Helper::File::getNumberOfElmsToLoad()
{
    return numberOfElementsToLoad;
}

HDF5Helper::File::HDF5Vector3D HDF5Helper::File::getNdims()
{
    return HDF5Vector3D(nZ, nY, nX);
}

/**
 * @brief HDF5Helper::getTime
 * @return
 */
double HDF5Helper::getTime()
{
    #ifdef PARALLEL_HDF5
        return MPI_Wtime() * 1000;
    #endif
    #ifdef __unix
        timeval tv;
        gettimeofday (&tv, NULL);
        return double (tv.tv_sec) * 1000 + (tv.tv_usec) / 1000;
    #endif

    #ifdef _WIN32
        return GetTickCount();
    #endif
}

size_t HDF5Helper::getTotalSystemPhysicalMemory()
{
    #ifdef __unix
        long pages = sysconf(_SC_PHYS_PAGES);
        long page_size = sysconf(_SC_PAGE_SIZE);
        return pages * page_size;
    #endif

    #ifdef _WIN32
        MEMORYSTATUSEX status;
        status.dwLength = sizeof(status);
        GlobalMemoryStatusEx(&status);
        return status.ullTotalPhys;
    #endif
}

size_t HDF5Helper::getAvailableSystemPhysicalMemory()
{
    #ifdef __unix
        long pages = sysconf(_SC_AVPHYS_PAGES);
        long page_size = sysconf(_SC_PAGE_SIZE);
        return pages * page_size;
    #endif

    #ifdef _WIN32
        MEMORYSTATUSEX status;
        status.dwLength = sizeof(status);
        GlobalMemoryStatusEx(&status);
        return status.ullAvailPhys;
    #endif
}