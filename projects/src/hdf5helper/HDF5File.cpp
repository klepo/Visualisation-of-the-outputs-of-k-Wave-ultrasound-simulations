/**
 * @file        HDF5File.cpp
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        30 July      2014 (created)
 *              8  September 2016 (updated)
 *              3  November  2016 (updated)
 *
 * @brief       The implementation file containing HDF5File class definition.
 *              This class contains HDF5Dataset class and HDF5Group class.
 *
 * @license     This file is part of the hdf5helper library for k-Wave h5 processing
 *              for preprocessing the HDF5 data created by the k-Wave toolbox - http://www.k-wave.org.
 *              The hdf5helper library is free software.
 *
 * @copyright   Copyright © 2016, Petr Kleparnik, VUT FIT Brno. All Rights Reserved.
 *
 */

#include "HDF5File.h"
#include "HDF5Dataset.h"
#include "HDF5Group.h"

namespace HDF5Helper {

/**
 * @brief HDF5File
 * @param filename path to HDF5 file
 * @param flag default: OPEN - open (read and write) file, CREATE - create new file
 * @param log enable/disable log file
 * @throw std::runtime_error
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

    // Set size of memory
    //std::cout << getAvailableSystemPhysicalMemory() << std::endl;
    //numberOfElementsToLoad = (getAvailableSystemPhysicalMemory() / 2) / 4;
    //numberOfElementsToLoad = 1024 * 1024 * 1024 * 2; // cca 10 GB
    //numberOfElementsToLoad = 1024 * 1024 * 1024;
    // 4 x numberOfElementsToLoad == in bytes

    // Disable error HDF5 output
    H5Eset_auto(H5E_DEFAULT, 0, 0);

    // Save filename
    this->filename = filename;

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
    if (flag == File::OPEN) {
        std::cout << "Opening file \"" << filename << "\" ";
        file = H5Fopen(filename.c_str(), H5F_ACC_RDWR, plist_FILE_ACCESS);
        if (file < 0) {
            std::cout << "... error" << std::endl;
            throw std::runtime_error("H5Fopen error");
            //MPI::COMM_WORLD.Abort(1);
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
            throw std::runtime_error("Wrong HDF5File");
        }
    } else if (flag == CREATE) {
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
 * @brief ~HDF5File
 */
File::~File()
{
    closeFileAndObjects();
}

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
 * @brief getLogFileStream
 * @return log file stream
 */
std::ofstream *File::getLogFileStream()
{
    return &logFileStream;
}

/**
 * @brief insertDataset Open, create and insert dataset (HDF5Dataset) to std::map datasets
 * @param datasetName name of dataset
 * @throw std::runtime_error
 */
void File::insertDataset(const std::string datasetName, bool log)
{
    std::string datasetNameTmp = datasetName;
    if (datasetNameTmp.find("/") != 0)
        datasetNameTmp = "/" + datasetNameTmp;
    if (log)
        std::cout << "Opening dataset \"" << datasetNameTmp << "\" ";
    hid_t d = H5Dopen(file, datasetNameTmp.c_str(), H5P_DEFAULT);
    if (d < 0) {
        if (log)
            std::cout << "... error" << std::endl;
        throw std::runtime_error("H5Dopen error");
        //MPI::COMM_WORLD.Abort(1);
    }
    HDF5Dataset *hDF5Dataset = new HDF5Dataset(d, datasetNameTmp, this);
    if (log)
        std::cout << "... OK" << std::endl;
    datasets.insert(HDF5Helper::PairOfDatasets(datasetNameTmp, hDF5Dataset));
}

/**
 * @brief insertGroup Open, create and insert group (HDF5Group) to std::map groups
 * @param groupName name of group
 * @throw std::runtime_error
 */
void File::insertGroup(const std::string groupName, bool log)
{
    std::string groupNameTmp = groupName;
    if (groupNameTmp.find("/") != 0)
        groupNameTmp = "/" + groupNameTmp;
    if (log)
        std::cout << "Opening group \"" << groupNameTmp << "\" ";
    hid_t g = H5Gopen(file, groupNameTmp.c_str(), H5P_DEFAULT);
    if (g < 0){
        if (log)
            std::cout << "... error" << std::endl;
        throw std::runtime_error("H5Gopen error");
        //MPI::COMM_WORLD.Abort(1);
    }
    HDF5Group *hDF5Group = new HDF5Group(g, groupNameTmp, this);
    if (log)
        std::cout << "... OK" << std::endl;
    groups.insert(HDF5Helper::PairOfGroups(groupNameTmp, hDF5Group));
}

int File::getMPISize() const
{
    return mPISize;
}

/**
 * @brief createDatasetI Create new hsize_t dataset in file
 * @param datasetName name of dataset
 * @param rank
 * @param size
 * @param chunk_size
 * @param rewrite flag for rewriting existing dataset
 * @throw std::runtime_error
 */
void File::createDatasetI(const std::string datasetName, HDF5Vector size, HDF5Vector chunkSize, bool rewrite, bool log)
{
    File::createDataset(datasetName, H5T_NATIVE_UINT64, size, chunkSize, rewrite, log);
}

/**
 * @brief createDatasetF Create new float dataset in file
 * @param datasetName name of dataset
 * @param rank
 * @param size
 * @param chunk_size
 * @param rewrite flag for rewriting existing dataset
 * @throw std::runtime_error
 */
void File::createDatasetF(const std::string datasetName, HDF5Vector size, HDF5Vector chunkSize, bool rewrite, bool log)
{
    File::createDataset(datasetName, H5T_NATIVE_FLOAT, size, chunkSize, rewrite, log);
}

/**
 * @brief createDataset Create new dataset in file of given type
 * @param datasetName name of dataset
 * @param rank
 * @param size
 * @param chunk_size
 * @param rewrite flag for rewriting existing dataset
 * @param type (H5T_NATIVE_FLOAT | H5T_NATIVE_UINT64)
 * @throw std::runtime_error
 */
void File::createDataset(const std::string datasetName, hid_t datatype, HDF5Vector size, HDF5Vector chunkSize, bool rewrite, bool log)
{
    hid_t dataspace = H5Screate_simple(static_cast<int>(size.getLength()), size.getVectorPtr(), 0);
    if (dataspace < 0){
        throw std::runtime_error("H5Screate_simple error");
        //MPI::COMM_WORLD.Abort(1);
    }
    if (datatype < 0){
        throw std::runtime_error("HDF5 datatype error");
        //MPI::COMM_WORLD.Abort(1);
    }
    hid_t plist = H5Pcreate(H5P_DATASET_CREATE);
    if (plist < 0){
        throw std::runtime_error("H5Pcreate error");
        //MPI::COMM_WORLD.Abort(1);
    }

    // Set chunking
    if (chunkSize.getLength() != size.getLength()) {
        throw std::runtime_error("Error - Chunk size length is not equal dataset size length");
        //MPI::COMM_WORLD.Abort(1);
    }

    if (!chunkSize.hasZeros()) {
        err = H5Pset_chunk(plist, static_cast<int>(chunkSize.getLength()), chunkSize.getVectorPtr());
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

    // Create groups
    std::string s = datasetName;
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
        std::cout << "Creating dataset \"" << datasetName << "\" ";
    if (rewrite) {
        H5Ldelete(file, datasetName.c_str(), H5P_DEFAULT);
        if (log)
            std::cout << "... rewrite ";
    }

    hid_t dataset = H5Dcreate(file, datasetName.c_str(), datatype, dataspace, H5P_DEFAULT, plist, H5P_DEFAULT);
    if (dataset < 0){
        if (log)
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
    if (log)
        std::cout << "... OK" << std::endl;
}

void File::createDataset(HDF5Dataset *dataset, bool rewrite, bool log)
{
    createDataset(dataset->getName(), dataset->getDataType(), dataset->getDims(), dataset->getChunkDims(), rewrite, log);
}

/**
 * @brief createGroup Create new group
 * @param name
 * @param rewrite flag for rewriting existing group
 * @throw std::runtime_error
 */
void File::createGroup(const std::string name, bool rewrite, bool log)
{
    if (log)
        std::cout << "Creating group \"" << name << "\" ";
    if (rewrite) {
        H5Ldelete(file, name.c_str(), H5P_DEFAULT);
        if (log)
            std::cout << "... rewrite ";
    }

    if (!H5Lexists(file, name.c_str(), H5P_DEFAULT)) {
        hid_t group = H5Gcreate(file, name.c_str(), H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
        if (group < 0){
            if (log)
                std::cout << "... error" << std::endl;
            throw std::runtime_error("H5Dcreate error");
        }
        H5Gclose(group);
        if (err < 0){
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
 * @brief openDataset Open dataset (create new HDF5Dataset) by index in HDF5 file
 * @param idx
 * @return dataset (HDF5Dataset)
 * @throw std::runtime_error
 */
HDF5Dataset *File::openDataset(hsize_t idx, bool log)
{
    std::string name = getObjNameByIdx(idx);
    return openDataset(name, log);
}

/**
 * @brief openDataset Open dataset (create new HDF5Dataset) by datasetName in HDF5 file
 * @param datasetName
 * @return dataset (HDF5Dataset)
 * @throw std::runtime_error
 */
HDF5Dataset *File::openDataset(const std::string datasetName, bool log)
{
    std::string datasetNameTmp = datasetName;
    if (datasetNameTmp.find("/") != 0)
        datasetNameTmp = "/" + datasetNameTmp;
    if (datasets.find(datasetNameTmp) == datasets.end()) {
        insertDataset(datasetNameTmp, log);
        return openDataset(datasetNameTmp, log);
    } else
        return datasets.find(datasetNameTmp)->second;
}

/**
 * @brief closeDataset Close dataset with given name in HDF5 file
 * @param datasetName
 */
void File::closeDataset(const std::string datasetName, bool log)
{
    std::string datasetNameTmp = datasetName;
    if (datasetNameTmp.find("/") != 0)
        datasetNameTmp = "/" + datasetNameTmp;
    if (datasets.find(datasetNameTmp) != datasets.end()){
        HDF5Dataset *dataset = datasets.find(datasetNameTmp)->second;
        dataset->setDeleteLog(log);
        delete dataset;
        datasets.erase(datasets.find(datasetNameTmp));
    }
}

/**
 * @brief File::closeDataset
 * @param idx
 */
void File::closeDataset(hsize_t idx, bool log)
{
    std::string name = getObjNameByIdx(idx);
    closeDataset(name, log);
}

void File::closeDataset(HDF5Dataset *dataset, bool log)
{
    closeDataset(dataset->getName(), log);
}

/**
 * @brief openGroup Open group (create new HDF5Group) with given name in HDF5 file
 * @param groupName
 * @return group
 */
HDF5Group *File::openGroup(const std::string groupName, bool log)
{
    std::string groupNameTmp = groupName;
    if (groupNameTmp.find("/") != 0)
        groupNameTmp = "/" + groupNameTmp;
    if (groups.find(groupNameTmp) == groups.end()) {
        insertGroup(groupNameTmp, log);
        return openGroup(groupNameTmp, log);
    } else
        return groups.find(groupNameTmp)->second;
}

/**
 * @brief openGroup Open group (create new HDF5Group) with given index
 * @param idx
 * @return group
 */
HDF5Group *File::openGroup(hsize_t idx, bool log)
{
    std::string name = getObjNameByIdx(idx);
    return openGroup(name, log);
}

/**
 * @brief closeGroup Close group with given name in HDF5 file
 * @param groupName
 */
void File::closeGroup(const std::string groupName, bool log)
{
    std::string groupNameTmp = groupName;
    if (groupNameTmp.find("/") != 0)
        groupNameTmp = "/" + groupNameTmp;
    if (groups.find(groupNameTmp) != groups.end()){
        HDF5Group *group = groups.find(groupNameTmp)->second;
        group->setDeleteLog(log);
        delete group;
        groups.erase(groups.find(groupNameTmp));
    }
}

/**
 * @brief closeGroup Close group with given index
 * @param idx
 */
void File::closeGroup(hsize_t idx, bool log)
{
    std::string name = getObjNameByIdx(idx);
    closeGroup(name, log);
}

void File::closeGroup(HDF5Group *group, bool log)
{
    closeGroup(group->getName(), log);
}

/**
 * @brief getNumObjs Get number of objects in HDF5 file (root group)
 * @return
 */
hsize_t File::getNumObjs()
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
 * @brief getObjNameById Get object name by id
 * @param id
 * @return object name
 */
std::string File::getObjNameByIdx(hsize_t idx, hid_t fileGroupId)
{
    int fGIdTmp = fileGroupId;
    if (fileGroupId <= 0)
        fGIdTmp = file;

    char *nameC = 0;
    size_t size = 0;
    ssize_t sizeR = 0;
    sizeR = H5Gget_objname_by_idx(fGIdTmp, idx, nameC, size);
    if (sizeR <= 0){
        throw std::runtime_error("H5Gget_objname_by_idx error");
        //MPI::COMM_WORLD.Abort(1);
    }
    nameC = new char[sizeR + 1];
    H5Gget_objname_by_idx(fGIdTmp, idx, nameC, static_cast<size_t>(sizeR) + 1);
    std::string name(nameC);
    delete [] nameC;
    return name;
}

/**
 * @brief getObjTypeById Get object type by id
 * @param id
 * @return object type
 */
H5G_obj_t File::getObjTypeByIdx(hsize_t idx, hid_t fileGroupId)
{
    int fGIdTmp = fileGroupId;
    if (fileGroupId <= 0)
        fGIdTmp = file;

    int type = 0;
    type = H5Gget_objtype_by_idx(fGIdTmp, idx);
    if (type < 0){
        throw std::runtime_error("H5Gget_objtype_by_idx error");
        //MPI::COMM_WORLD.Abort(1);
    }
    return static_cast<H5G_obj_t>(type);
}

/**
 * @brief objExistsByName
 * @param name
 * @return true/false
 */
bool File::objExistsByName(const std::string name)
{
    if (H5Lexists(file, name.c_str(), H5P_DEFAULT))
        return H5Oexists_by_name(file, name.c_str(), H5P_DEFAULT) != 0;
    else
        return false;
}

/**
 * @brief getFilename
 * @return filename
 */
std::string File::getFilename()
{
    return filename;
}

/**
 * @brief setSizeOfDataPart
 * @param size
 */
void File::setNumberOfElmsToLoad(hsize_t size)
{
    #ifdef PARALLEL_HDF5
        if (mPISize > 1 && size > std::numeric_limits<int>::max())
            throw std::runtime_error("setNumberOfElmsToLoad error");
    #endif
    numberOfElementsToLoad = size;
}

/**
 * @brief getSizeOfDataPart
 * @return size of data part
 */
hsize_t File::getNumberOfElmsToLoad()
{
    return numberOfElementsToLoad;
}

HDF5Vector4D File::getNdims()
{
    return nDims;
}

/**
 * @brief getTime
 * @return
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
        return static_cast<size_t>(status.ullTotalPhys);
    #endif
}

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
        return static_cast<size_t>(status.ullAvailPhys);
    #endif
}

/**
 * @brief convertlinearTo3D Convert linear index to 3D position (z, y, x)
 */
void convertlinearToMultiDim(hsize_t index, HDF5Vector &position, HDF5Vector dims)
{
    position = HDF5Vector(dims.getLength(), 0);

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
 * @brief convert3DToLinear Convert 3D position (z, y, x) to linear index
 */
void convertMultiDimToLinear(HDF5Vector position, hsize_t &index, HDF5Vector dims)
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

void copyDataset(HDF5Dataset *srcDataset, File *dstFile, bool rewrite, bool log)
{
    dstFile->createDataset(srcDataset, rewrite, log);
    HDF5Dataset *dstDataset = dstFile->openDataset(srcDataset->getName(), log);

    HDF5Vector offset;
    HDF5Vector count;

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
        HDF5Attribute *attr = srcDataset->getAttribute(i);
        dstDataset->setAttribute(attr, log);
        delete attr;
    }
    dstFile->closeDataset(dstDataset, log);
}

void copyDataset(File *srcFile, File *dstFile, std::string name, bool rewrite, bool log)
{
    if (srcFile->getFilename() != dstFile->getFilename()) {
        HDF5Dataset *srcDataset = srcFile->openDataset(name, log);
        copyDataset(srcDataset, dstFile, rewrite, log);
        srcFile->closeDataset(srcDataset, log);
    } else {
        if (log)
            std::cout << "Source file == destination file -> cannot copy datasets" << std::endl;
    }
}

}
