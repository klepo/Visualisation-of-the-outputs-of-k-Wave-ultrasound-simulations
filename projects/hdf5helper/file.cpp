/**
 * @file        file.cpp
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        30 July      2014 (created) \n
 *              28 August    2017 (updated)
 *
 * @brief       The implementation file containing HDF5Helper::File class definition. This class
 *              is used for better work with k-Wave HDF5 input and output files.
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

#include <file.h>

namespace HDF5Helper {

/**
 * @brief Creates File object with given filename
 * @param[in] filename Path to HDF5 file
 * @param[in] flag Default value: OPEN - open (read and write) file, CREATE - create new file
 * @param[in] log Enable/disable log file (optional)
 * @throw std::runtime_error
 *
 * For PARALLEL_HDF5 MPI_Comm and MPI_Info are passed into this constructor.
 */
#ifdef PARALLEL_HDF5
File::File(std::string filename, unsigned int flag, MPI_Comm comm, MPI_Info info, bool log)
#else
File::File(std::string filename, unsigned int flag, bool log)
#endif
{
#ifdef PARALLEL_HDF5
    // Check MPI is initialized
    int started, error = 0;
    error = MPI_Initialized(&started);
    if (error)
        throw std::runtime_error("MPI is not initialized");
#endif
    // Save filename
    this->filename = filename;

    // Set size of memory
    std::cout << "Available system physical memory: " << getAvailableSystemPhysicalMemory() << " bytes" << std::endl;
    // 1 x 32-bit float == 4 x bytes

    // max cca 4 GB
    hsize_t maxCount = 1024 * 1024 * 1024;
    if (getAvailableSystemPhysicalMemory() > maxCount * 4) {
        setNumberOfElmsToLoad(maxCount);
    } else {
        setNumberOfElmsToLoad((getAvailableSystemPhysicalMemory() / 8));
    }
    // setNumberOfElmsToLoad(1024 * 1024 * 1024 * 2); // cca 10 GB
    // setNumberOfElmsToLoad(1024 * 1024 * 1024);

    // Disable error HDF5 output
    H5Eset_auto(0, 0, 0);

    // Create log file
    if (log) {
        logFileStream.open(filename + "_" + std::to_string(time(0)) + ".log");
        logFileStream << filename << std::endl;
    }

    // Create File Access Property List
    plist_FILE_ACCESS = H5Pcreate(H5P_FILE_ACCESS);

#ifdef PARALLEL_HDF5
    // Get number of processes -> set NUMBER_OF_ELEMENTS_TO_LOAD to max int (MPI limit)
    MPI_Comm_size(comm, &mPISize);
    if (mPISize > 1) {
        setNumberOfElmsToLoad(getNumberOfElmsToLoad());
        err = H5Pset_fapl_mpio(plist_FILE_ACCESS, comm, info);
        if (err < 0) {
            throw std::runtime_error("H5Pset_fapl_mpio error");
        }
    }
#endif

    // Set cache
    hsize_t chunkBytes = 64 * 64 * 64 * 4;
    //521
    err = H5Pset_cache(plist_FILE_ACCESS, 0, 521, 521 * chunkBytes, 0.75);
    if (err < 0) {
        throw std::runtime_error("H5Pset_cache error");
    }

    //herr_t H5Pset_alignment(hid_t plist, hsize_t threshold, hsize_t alignment )
    hsize_t threshold = 0;
    hsize_t alignment = 64 * 64 * 64 * 4;
    // Set alignment
    err = H5Pset_alignment(plist_FILE_ACCESS, threshold, alignment);
    if (err < 0) {
        throw std::runtime_error("H5Pset_alignment error");
    }

    // Open or create file
    if (flag == File::OPEN) {
        std::cout << "Opening file \"" << filename << "\" ";
        file = H5Fopen(filename.c_str(), H5F_ACC_RDWR, plist_FILE_ACCESS);
        if (file < 0) {
            std::cout << "... error" << std::endl;
            throw std::runtime_error("H5Fopen error");
        }
        std::cout << "... OK " << std::endl;

        try {
            // Load basic datasets values
            hsize_t data;

            openDataset(NT_DATASET, false)->readDataset(data, false);
            nDims.w(data);
            closeDataset(NT_DATASET, false);

            openDataset(NX_DATASET, false)->readDataset(data, false);
            nDims.x(data);
            closeDataset(NX_DATASET, false);

            openDataset(NY_DATASET, false)->readDataset(data, false);
            nDims.y(data);
            closeDataset(NY_DATASET, false);

            openDataset(NZ_DATASET, false)->readDataset(data, false);
            nDims.z(data);
            closeDataset(NZ_DATASET, false);

        } catch(std::exception) {
            closeFileAndObjects();
            throw std::runtime_error("Wrong File");
        }
    } else if (flag == CREATE) {
        std::cout << "Creating file \"" << filename << "\" ";
        file = H5Fcreate(filename.c_str(), H5F_ACC_TRUNC, 0, plist_FILE_ACCESS);
        if (file < 0) {
            std::cout << "... error" << std::endl;
            throw std::runtime_error("H5Fcreate error");
        }
        std::cout << "... OK " << std::endl;
    } else {
        throw std::runtime_error("Wrong File flag");
    }
}

/**
 * @brief Destructor of File object
 *
 * Closes all objects and file.
 */
File::~File()
{
    closeFileAndObjects();
}

/**
 * @brief Creates new integer dataset in file
 * @param[in] name Name of dataset
 * @param[in] size Size of dataset
 * @param[in] chunkSize Chunk size of dataset
 * @param[in] rewrite Flag for rewriting existing dataset (optional)
 * @param[in] log Logging flag (optional)
 */
void File::createDatasetI(const std::string name, Vector size, Vector chunkSize, bool rewrite, bool log)
{
    File::createDataset(name, H5T_NATIVE_UINT64, size, chunkSize, rewrite, log);
}

/**
 * @brief Creates new float dataset in file
 * @param[in] name Name of dataset
 * @param[in] size Size of dataset
 * @param[in] chunkSize Chunk size of dataset
 * @param[in] rewrite Flag for rewriting existing dataset (optional)
 * @param[in] log Logging flag (optional)
 */
void File::createDatasetF(const std::string name, Vector size, Vector chunkSize, bool rewrite, bool log)
{
    File::createDataset(name, H5T_NATIVE_FLOAT, size, chunkSize, rewrite, log);
}

/**
 * @brief Creates new dataset of given type in file
 * @param[in] name Name of dataset
 * @param[in] datatype Datatype (H5T_NATIVE_FLOAT | H5T_NATIVE_UINT64)
 * @param[in] size Size of dataset
 * @param[in] chunkSize Chunk size of dataset
 * @param[in] rewrite Flag for rewriting existing dataset (optional)
 * @param[in] log Logging flag (optional)
 * @throw std::runtime_error
 */
void File::createDataset(const std::string name, hid_t datatype, Vector size, Vector chunkSize, bool rewrite, bool log)
{
    hid_t dataspace = H5Screate_simple(int(size.getLength()), size.getVectorPtr(), 0);
    if (dataspace < 0) {
        throw std::runtime_error("H5Screate_simple error");
    }
    if (datatype < 0) {
        throw std::runtime_error("HDF5 datatype error");
    }
    hid_t plist = H5Pcreate(H5P_DATASET_CREATE);
    if (plist < 0) {
        throw std::runtime_error("H5Pcreate error");
    }

    // Set chunking
    if (chunkSize.getLength() != size.getLength()) {
        throw std::runtime_error("Error - Chunk size length is not equal dataset size length");
    }

    if (!chunkSize.hasZeros()) {
        err = H5Pset_chunk(plist, int(chunkSize.getLength()), chunkSize.getVectorPtr());
        if (err < 0) {
            throw std::runtime_error("H5Pset_chunk error");
        }
    } else {
        err = H5Pset_layout(plist, H5D_CONTIGUOUS);
        if (err < 0) {
            throw std::runtime_error("H5Pset_layout error");
        }
    }

    // Create groups
    std::string s = name;
    std::string delimiter = "/";

    size_t pos = 0;
    std::string token;
    while ((pos = s.find(delimiter)) != std::string::npos) {
        token = s.substr(0, pos);
        if (token.length() > 0)
            createGroup(token, false);
        s.erase(0, pos + delimiter.length());
    }

    if (log)
        std::cout << "Creating dataset \"" << name << "\" ";

    if (rewrite) {
        if (H5Lexists(file, name.c_str(), 0)) {
            Dataset *dataset = openDataset(name, false);
            if (H5Tequal(dataset->getDataType(), datatype) && dataset->getDims() == size && dataset->getChunkDims() == chunkSize) {
                if (log)
                    std::cout << " rewrite original space ... OK" << std::endl;
                err = H5Sclose(dataspace);
                if (err < 0) {
                    throw std::runtime_error("H5Sclose error");
                }
                err = H5Pclose(plist);
                if (err < 0) {
                    throw std::runtime_error("H5Pclose error");
                }
                closeDataset(dataset, false);
                return;
            }
            closeDataset(dataset, false);
            if (log)
                std::cout << "... delete original link";
            H5Ldelete(file, name.c_str(), 0);
        }
    }

    hid_t dataset = H5Dcreate(file, name.c_str(), datatype, dataspace, 0, plist, 0);
    if (dataset < 0) {
        if (log)
            std::cout << "... error" << std::endl;
        throw std::runtime_error("H5Dcreate error");
    }

    err = H5Sclose(dataspace);
    if (err < 0) {
        throw std::runtime_error("H5Sclose error");
    }
    err = H5Pclose(plist);
    if (err < 0) {
        throw std::runtime_error("H5Pclose error");
    }
    err = H5Dclose(dataset);
    if (err < 0) {
        throw std::runtime_error("H5Dclose error");
    }
    if (log)
        std::cout << "... OK" << std::endl;
}

/**
 * @brief Creates new dataset with name, type, size, and chunkSize same as given dataset has
 * @param[in] dataset Dataset
 * @param[in] rewrite Flag for rewriting existing dataset (optional)
 * @param[in] log Logging flag (optional)
 * @throw std::runtime_error
 */
void File::createDataset(Dataset *dataset, bool rewrite, bool log)
{
    createDataset(dataset->getName(), dataset->getDataType(), dataset->getDims(), dataset->getChunkDims(), rewrite, log);
}

/**
 * @brief Opens dataset by name in HDF5 file
 * @param[in] name Name of dataset
 * @param[in] log Logging flag (optional)
 * @return Opened dataset
 */
Dataset *File::openDataset(const std::string name, bool log)
{
    std::string nameTmp = name;
    if (nameTmp.find("/") != 0)
        nameTmp = "/" + nameTmp;
    if (datasets.find(nameTmp) == datasets.end()) {
        insertDataset(nameTmp, log);
        return openDataset(nameTmp, log);
    } else
        return datasets.find(nameTmp)->second;
}

/**
 * @brief Opens dataset by index in HDF5 file
 * @param[in] idx Index of dataset in file
 * @param[in] log Logging flag (optional)
 * @return Opened dataset
 */
Dataset *File::openDataset(hsize_t idx, bool log)
{
    return openDataset(getObjNameByIdx(idx), log);
}

/**
 * @brief Is dataset opened?
 * @param[in] name Name of dataset
 * @return True/False
 */
bool File::isDatasetOpened(const std::string name)
{
    std::string nameTmp = name;
    if (nameTmp.find("/") != 0)
        nameTmp = "/" + nameTmp;
    if (datasets.find(nameTmp) == datasets.end())
        return false;
    else
        return true;
}

/**
 * @brief Is dataset opened?
 * @param[in] idx Index of dataset in file
 * @return True/False
 */
bool File::isDatasetOpened(hsize_t idx)
{
    return isDatasetOpened(getObjNameByIdx(idx));
}

/**
 * @brief Closes dataset with given name in HDF5 file
 * @param[in] name Name of dataset
 * @param[in] log Logging flag (optional)
 */
void File::closeDataset(const std::string name, bool log)
{
    std::string nameTmp = name;
    if (nameTmp.find("/") != 0)
        nameTmp = "/" + nameTmp;
    if (datasets.find(nameTmp) != datasets.end()) {
        Dataset *dataset = datasets.find(nameTmp)->second;
        dataset->setDeleteLog(log);
        delete dataset;
        datasets.erase(datasets.find(nameTmp));
    }
}

/**
 * @brief Closes dataset with given index in HDF5 file
 * @param[in] idx Index of dataset in file
 * @param[in] log Logging flag (optional)
 */
void File::closeDataset(hsize_t idx, bool log)
{
    std::string name = getObjNameByIdx(idx);
    closeDataset(name, log);
}

/**
 * @brief Closes dataset with same name as given dataset has
 * @param[in] dataset Dataset
 * @param[in] log Logging flag (optional)
 */
void File::closeDataset(Dataset *dataset, bool log)
{
    closeDataset(dataset->getName(), log);
}

/**
 * @brief Creates new group with given name
 * @param[in] name Name of group
 * @param[in] rewrite Flag for rewriting existing group (optional)
 * @param[in] log Logging flag (optional)
 * @throw std::runtime_error
 */
void File::createGroup(const std::string name, bool rewrite, bool log)
{
    if (log)
        std::cout << "Creating group \"" << name << "\" ";
    if (rewrite) {
        H5Ldelete(file, name.c_str(), 0);
        if (log)
            std::cout << "... rewrite ";
    }

    if (!H5Lexists(file, name.c_str(), 0)) {
        hid_t group = H5Gcreate(file, name.c_str(), 0, 0, 0);
        if (group < 0) {
            if (log)
                std::cout << "... error" << std::endl;
            throw std::runtime_error("H5Dcreate error");
        }
        H5Gclose(group);
        if (err < 0) {
            throw std::runtime_error("H5Gclose error");
        }
        if (log)
            std::cout << "... OK" << std::endl;
    } else {
        if (log)
            std::cout << "... OK - group already exists" << std::endl;
    }
}

/**
 * @brief Opens group with given name in HDF5 file
 * @param[in] name Name of group
 * @param[in] log Logging flag (optional)
 * @return Opened group
 */
Group *File::openGroup(const std::string name, bool log)
{
    std::string nameTmp = name;
    if (nameTmp.find("/") != 0)
        nameTmp = "/" + nameTmp;
    if (groups.find(nameTmp) == groups.end()) {
        insertGroup(nameTmp, log);
        return openGroup(nameTmp, log);
    } else
        return groups.find(nameTmp)->second;
}

/**
 * @brief Opens group with given index in HDF5 file
 * @param[in] idx Index of group in file
 * @param[in] log Logging flag (optional)
 * @return Opened group
 */
Group *File::openGroup(hsize_t idx, bool log)
{
    std::string name = getObjNameByIdx(idx);
    return openGroup(name, log);
}

/**
 * @brief Closes group with given name in HDF5 file
 * @param[in] name Name of group
 * @param[in] log Logging flag (optional)
 */
void File::closeGroup(const std::string name, bool log)
{
    std::string nameTmp = name;
    if (nameTmp.find("/") != 0)
        nameTmp = "/" + nameTmp;
    if (groups.find(nameTmp) != groups.end()) {
        Group *group = groups.find(nameTmp)->second;
        group->setDeleteLog(log);
        delete group;
        groups.erase(groups.find(nameTmp));
    }
}

/**
 * @brief Closes group with given index in HDF5 file
 * @param[in] idx Index of group in file
 * @param[in] log Logging flag (optional)
 */
void File::closeGroup(hsize_t idx, bool log)
{
    std::string name = getObjNameByIdx(idx);
    closeGroup(name, log);
}

/**
 * @brief Closes group with same name as given group has
 * @param[in] group Group
 * @param[in] log Logging flag (optional)
 */
void File::closeGroup(Group *group, bool log)
{
    closeGroup(group->getName(), log);
}

/**
 * @brief Returns number of objects in HDF5 file (root group)
 * @param[in] groupId Group id (optional)
 * @return Number of objects in HDF5 file
 * @throw std::runtime_error
 */
hsize_t File::getNumObjs(hid_t groupId)
{
    int groupIdTmp = groupId;
    if (groupId <= 0)
        groupIdTmp = file;

    H5G_info_t group_info;
    err = H5Gget_info(groupIdTmp, &group_info);
    if (err < 0) {
        throw std::runtime_error("H5Gget_info error");
    }
    return group_info.nlinks;
}

/**
 * @brief Returns object name by index
 * @param[in] idx Index of object in file
 * @param[in] groupId Group id (optional)
 * @return Object name
 * @throw std::runtime_error
 */
std::string File::getObjNameByIdx(hsize_t idx, hid_t groupId)
{
    int groupIdTmp = groupId;
    if (groupId <= 0)
        groupIdTmp = file;

    char *nameC = 0;
    size_t size = 0;
    ssize_t sizeR = 0;
    sizeR = H5Gget_objname_by_idx(groupIdTmp, idx, nameC, size);
    if (sizeR <= 0) {
        throw std::runtime_error("H5Gget_objname_by_idx error");
    }
    nameC = new char[size_t(sizeR) + 1]();
    H5Gget_objname_by_idx(groupIdTmp, idx, nameC, size_t(sizeR) + 1);
    std::string name(nameC);
    delete [] nameC;
    return name;
}

/**
 * @brief Returns object type by index
 * @param[in] idx Index of object in file
 * @param[in] groupId Group id (optional)
 * @return Object type
 * @throw std::runtime_error
 */
H5G_obj_t File::getObjTypeByIdx(hsize_t idx, hid_t groupId)
{
    int groupIdTmp = groupId;
    if (groupId <= 0)
        groupIdTmp = file;

    int type = 0;
    type = H5Gget_objtype_by_idx(groupIdTmp, idx);
    if (type < 0) {
        throw std::runtime_error("H5Gget_objtype_by_idx error");
    }
    return H5G_obj_t(type);
}

/**
 * @brief Exists object with given name?
 * @param[in] name Object name
 * @return True/False
 */
bool File::objExistsByName(const std::string name)
{
    if (H5Lexists(file, name.c_str(), 0))
        return H5Oexists_by_name(file, name.c_str(), 0) != 0;
    else
        return false;
}

/**
 * @brief Returns filename
 * @return Filename
 */
std::string File::getFilename()
{
    return filename;
}

/**
 * @brief Sets number of elements to load
 * @param[in] size
 * @throw std::runtime_error
 */
void File::setNumberOfElmsToLoad(hsize_t size)
{
    #ifdef PARALLEL_HDF5
        if (mPISize > 1 && size > std::numeric_limits<int>::max())
            throw std::runtime_error("setNumberOfElmsToLoad error");
    #endif
    std::cout << "Number of elements to load: " << size << " (floats: " << size * 4 << " bytes, unsigned 64-bit integers: " << size * 8 << " bytes)" << std::endl;
    numberOfElementsToLoad = size;
}

/**
 * @brief Returns number of elements to load
 * @return Number of elements to load
 */
hsize_t File::getNumberOfElmsToLoad()
{
    return numberOfElementsToLoad;
}

/**
 * @brief Returns N dimensions (Nt, Nz, Ny, Nx)
 * @return N dimensions
 */
Vector4D File::getNdims()
{
    return nDims;
}

/**
 * @brief Returns log file stream
 * @return Log file stream
 */
std::ofstream *File::getLogFileStream()
{
    return &logFileStream;
}

/**
 * @brief Returns MPI size
 * @return MPI size
 */
int File::getMPISize() const
{
    return mPISize;
}

/**
 * @brief Opens, creates and inserts Dataset to std::map datasets
 * @param[in] name Name of dataset
 * @param[in] log Logging flag (optional)
 * @throw std::runtime_error
 */
void File::insertDataset(const std::string name, bool log)
{
    std::string nameTmp = name;
    if (nameTmp.find("/") != 0)
        nameTmp = "/" + nameTmp;
    if (log)
        std::cout << "Opening dataset \"" << nameTmp << "\" ";
    hid_t d = H5Dopen(file, nameTmp.c_str(), 0);
    if (d < 0) {
        if (log)
            std::cout << "... error" << std::endl;
        throw std::runtime_error("H5Dopen error");
    }
    Dataset *dataset = new Dataset(d, nameTmp, this);
    if (log)
        std::cout << "... OK" << std::endl;
    datasets.insert(HDF5Helper::PairOfDatasets(nameTmp, dataset));
}

/**
 * @brief Opens, creates and inserts Group to std::map groups
 * @param[in] name Name of group
 * @param[in] log Logging flag (optional)
 * @throw std::runtime_error
 */
void File::insertGroup(const std::string name, bool log)
{
    std::string nameTmp = name;
    if (nameTmp.find("/") != 0)
        nameTmp = "/" + nameTmp;
    if (log)
        std::cout << "Opening group \"" << nameTmp << "\" ";
    hid_t g = H5Gopen(file, nameTmp.c_str(), 0);
    if (g < 0) {
        if (log)
            std::cout << "... error" << std::endl;
        throw std::runtime_error("H5Gopen error");
    }
    Group *group = new Group(g, nameTmp, this);
    if (log)
        std::cout << "... OK" << std::endl;
    groups.insert(HDF5Helper::PairOfGroups(nameTmp, group));
}

/**
 * @brief Closes file and objects
 * @throw std::runtime_error
 */
void File::closeFileAndObjects()
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
    if (err < 0) {
        throw std::runtime_error("H5Pclose error");
    }
    err = H5Fclose(file);
    if (err < 0) {
        throw std::runtime_error("H5Fclose error");
    }
    std::cout << " ... OK" << std::endl;
}

/**
 * @brief Returns time
 * @return Time
 */
double getTime()
{
    #ifdef PARALLEL_HDF5
        return MPI_Wtime() * 1000;
    #endif
    #ifdef __unix
        timeval tv;
        gettimeofday (&tv, 0);
        return double (tv.tv_sec) * 1000 + (tv.tv_usec) / 1000;
    #endif

    #ifdef _WIN32
        return GetTickCount();
    #endif
}

/**
 * @brief Returns total system physical memory
 * @return Total system physical memory
 */
size_t getTotalSystemPhysicalMemory()
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
        return size_t(status.ullTotalPhys);
    #endif
}

/**
 * @brief Returns available system physical memory
 * @return Available system physical memory
 */
size_t getAvailableSystemPhysicalMemory()
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
        return size_t(status.ullAvailPhys);
    #endif
}

/**
 * @brief Converts linear index to space (Cartesian) position
 * @param[in] index Linear index
 * @param[out] position Space position
 * @param[in] dims Dimensions of the space
 */
void convertlinearToMultiDim(hsize_t index, Vector &position, Vector dims)
{
    position = Vector(dims.getLength(), 0);

    hsize_t indexTemp = index;
    for (hsize_t i = 0; i < dims.getLength(); i++) {
        hsize_t prod = 1;
        for (hsize_t j = i + 1; j < dims.getLength(); j++) {
            prod *= dims[j];
        }
        position[i] = indexTemp / prod;
        indexTemp = indexTemp - position[i] * prod;
    }
}

/**
 * @brief Converts space (Cartesian) position to linear index
 * @param[in] position Space position
 * @param[out] index Linear index
 * @param[in] dims Dimensions of the space
 */
void convertMultiDimToLinear(Vector position, hsize_t &index, Vector dims)
{
    index = 0;
    for (hsize_t i = 0; i < dims.getLength(); i++) {
        hsize_t prod = 1;
        for (hsize_t j = i + 1; j < dims.getLength(); j++) {
            prod *= dims[j];
        }
        index += position[i] * prod;
    }
}

/**
 * @brief Checks or sets minimal and maximal float value
 * @param[in,out] first First value flag
 * @param[out] minV Minimal value
 * @param[out] maxV Maximal value
 * @param[in] value Input value
 *
 * This function exists due to OpenMP pragrmas
 */
void checkOrSetMinMaxValue(bool &first, float &minV, float &maxV, const float value)
{
    if (first) {
        #pragma omp critical
        {
            if (first) {
                minV = value;
                maxV = value;
                first = false;
            }
        }
    } else {
        if (minV > value) {
            #pragma omp critical
            {
                if (minV > value) minV = value;
            }
        }
        if (maxV < value) {
            #pragma omp critical
            {
                if (maxV < value) maxV = value;
            }
        }
    }
}

/**
 * @brief Checks or sets minimal and maximal integer value
 * @param[in,out] first First value flag
 * @param[out] minV Minimal value
 * @param[out] maxV Maximal value
 * @param[in] value Input value
 *
 * This function exists due to OpenMP pragrmas
 */
void checkOrSetMinMaxValue(bool &first, hsize_t &minV, hsize_t &maxV, const hsize_t value)
{
    if (first) {
        #pragma omp critical
        {
            if (first) {
                minV = value;
                maxV = value;
                first = false;
            }
        }
    } else {
        if (minV > value) {
            #pragma omp critical
            {
                if (minV > value) minV = value;
            }
        }
        if (maxV < value) {
            #pragma omp critical
            {
                if (maxV < value) maxV = value;
            }
        }
    }
}

/**
 * @brief File exists?
 * @param[in] name File name
 * @return True/False
 */
bool fileExists(const std::string& name) {
    std::ifstream infile(name);
    return infile.good();
}

/**
 * @brief Copies dataset
 * @param[in] srcDataset Source dataset
 * @param[in] dstFile Destination file
 * @param[in] rewrite Rewrite flag (optional)
 * @param[in] log Logging flag (optional)
 * @throw std::runtime_error
 */
void copyDataset(Dataset *srcDataset, File *dstFile, bool rewrite, bool log)
{
    dstFile->createDataset(srcDataset, rewrite, log);
    Dataset *dstDataset = dstFile->openDataset(srcDataset->getName(), log);

    Vector offset;
    Vector count;

    if (H5Tequal(srcDataset->getDataType(), H5T_NATIVE_FLOAT)) {
        float *data = 0;
        for (hsize_t i = 0; i < srcDataset->getNumberOfBlocks(); i++) {
            srcDataset->readBlock(i, offset, count, data, log);
            dstDataset->writeDataset(offset, count, data, log);
            delete[] data;
        }
    } else if (H5Tequal(srcDataset->getDataType(), H5T_NATIVE_UINT64)) {
        hsize_t *data = 0;
        for (hsize_t i = 0; i < srcDataset->getNumberOfBlocks(); i++) {
            srcDataset->readBlock(i, offset, count, data, log);
            dstDataset->writeDataset(offset, count, data, log);
            delete[] data;
        }
    } else {
        throw std::runtime_error("Wrong data type of dataset (not float or 64-bit unsigned integer)");
    }

    // Copy attributes
    for (hsize_t i = 0; i < srcDataset->getNumAttrs(); i++) {
        Attribute *attr = srcDataset->getAttribute(i);
        dstDataset->setAttribute(attr, log);
        delete attr;
    }
    dstFile->closeDataset(dstDataset, log);
}

/**
 * @brief Copies dataset
 * @param[in] srcFile Source file
 * @param[in] dstFile Destination file
 * @param[in] name Dataset name
 * @param[in] rewrite Rewrite flag (optional)
 * @param[in] log Logging flag (optional)
 */
void copyDataset(File *srcFile, File *dstFile, std::string name, bool rewrite, bool log)
{
    if (srcFile->getFilename() != dstFile->getFilename()) {
        bool openedFlag = false;
        if (srcFile->isDatasetOpened(name))
            openedFlag = true;
        Dataset *srcDataset = srcFile->openDataset(name, log);
        copyDataset(srcDataset, dstFile, rewrite, log);
        if (!openedFlag)
            srcFile->closeDataset(srcDataset, log);
    } else {
        if (log)
            std::cout << "Source file == destination file -> cannot copy datasets" << std::endl;
    }
}
}
