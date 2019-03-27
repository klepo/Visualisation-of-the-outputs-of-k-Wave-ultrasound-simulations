/**
 * @file        file.cpp
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        30 July      2014 (created) <br>
 *              3  March     2019 (updated)
 *
 * @brief       The implementation file containing H5Helper::File class definition.
 *
 * This class is used for better work with k-Wave HDF5 input and output files.
 *
 * @license     This file is part of the k-wave-h5-helper library for processing the HDF5 data
 *              created by the k-Wave toolbox - http://www.k-wave.org. This file may be used,
 *              distributed and modified under the terms of the LGPL version 3 open source
 *              license. A copy of the LGPL license should have been received with this file.
 *              Otherwise, it can be found at: http://www.gnu.org/copyleft/lesser.html.
 *
 * @copyright   Copyright © 2019, Petr Kleparnik, VUT FIT Brno. All Rights Reserved.
 *
 */

#include "file.h"
#include "object.h"
#include "dataset.h"
#include "group.h"

namespace H5Helper {

/**
 * @brief Creates File object with given filename
 * @param[in] filename Path to HDF5 file
 * @param[in] flag Default value: OPEN - open (read and write) file, CREATE - create new file
 * @param[in] log Delete logging flag (optional)
 * @throw std::runtime_error
 *
 * For PARALLEL_HDF5 MPI_Comm and MPI_Info are passed into this constructor.
 */
#ifdef PARALLEL_HDF5
File::File(std::string filename, unsigned int flag, MPI_Comm comm, MPI_Info info, bool log) : filename(filename)
  #else
File::File(std::string filename, unsigned int flag, bool log)
    : filename(filename)
    , deleteLogging(log)
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
    //std::cout << "Available system physical memory: " << getAvailableSystemPhysicalMemory() << " bytes" << std::endl;
    // 1 x 32-bit float == 4 x bytes

    // max ca 4 GB
    hsize_t maxCount = 1024 * 1024 * 1024;
    if (getAvailableSystemPhysicalMemory() > maxCount * 4) {
        setNumberOfElmsToLoad(maxCount);
    } else {
        setNumberOfElmsToLoad((getAvailableSystemPhysicalMemory() / 8));
    }
    // setNumberOfElmsToLoad(1024 * 1024 * 1024 * 2); // ca 10 GB
    // setNumberOfElmsToLoad(25 * 55 * 82);

    // Disable error HDF5 output
    H5Eset_auto(0, nullptr, nullptr);

    // Create log file
    //if (log) {
    //logFileStream.open(filename + "_" + std::to_string(time(0)) + ".log");
    //logFileStream << filename << std::endl;
    //}

    // Create File access property list
    pListFileAccessId = H5Pcreate(H5P_FILE_ACCESS);

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
    err = H5Pset_cache(pListFileAccessId, 0, 521, 521 * chunkBytes, 0.75);
    if (err < 0) {
        throw std::runtime_error("H5Pset_cache error");
    }

    //herr_t H5Pset_alignment(hid_t plist, hsize_t threshold, hsize_t alignment )
    hsize_t threshold = 0;
    hsize_t alignment = 64 * 64 * 64 * 4;
    // Set alignment
    err = H5Pset_alignment(pListFileAccessId, threshold, alignment);
    if (err < 0) {
        throw std::runtime_error("H5Pset_alignment error");
    }

    // Open or create file
    if (flag == File::OPEN) {
        if (log)
            std::cout << "Opening file \"" << filename << "\" ";
        fileId = H5Fopen(filename.c_str(), H5F_ACC_RDWR, pListFileAccessId);
        if (fileId < 0) {
            if (log)
                std::cout << "... error" << std::endl;
            throw std::runtime_error("H5Fopen error");
        }
        if (log)
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


        try {
            // Load point spacing values
            float dataD;

            openDataset(DT_DATASET, false)->readDataset(dataD, false);
            dValues.w(dataD);
            closeDataset(DT_DATASET, false);

            openDataset(DX_DATASET, false)->readDataset(dataD, false);
            dValues.x(dataD);
            closeDataset(DX_DATASET, false);

            openDataset(DY_DATASET, false)->readDataset(dataD, false);
            dValues.y(dataD);
            closeDataset(DY_DATASET, false);

            openDataset(DZ_DATASET, false)->readDataset(dataD, false);
            dValues.z(dataD);
            closeDataset(DZ_DATASET, false);

        } catch(std::exception) {
            //closeFileAndObjects();
            //throw std::runtime_error("Wrong File");
            std::cout << "Point spacing values are not in the file \"" << filename << "\""<< std::endl;
        }
    } else if (flag == CREATE) {
        if (log)
            std::cout << "Creating file \"" << filename << "\" ";
        fileId = H5Fcreate(filename.c_str(), H5F_ACC_TRUNC, 0, pListFileAccessId);
        if (fileId < 0) {
            if (log)
                std::cout << "... error" << std::endl;
            throw std::runtime_error("H5Fcreate error");
        }
        if (log)
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
 * @brief Creates new 64-bit unsigned integer dataset in file
 * @param[in] name Name of dataset
 * @param[in] size Size of dataset
 * @param[in] chunkSize Chunk size of dataset
 * @param[in] rewrite Flag for rewriting existing dataset (optional)
 * @param[in] log Logging flag (optional)
 */
void File::createDatasetI(std::string name, Vector size, Vector chunkSize, bool rewrite, bool log)
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
void File::createDatasetF(std::string name, Vector size, Vector chunkSize, bool rewrite, bool log)
{
    File::createDataset(name, H5T_NATIVE_FLOAT, size, chunkSize, rewrite, log);
}

/**
 * @brief Creates new dataset of given type in file
 * @param[in] name Name of dataset
 * @param[in] datatypeId Datatype id (H5T_NATIVE_FLOAT | H5T_NATIVE_UINT64)
 * @param[in] size Size of dataset
 * @param[in] chunkSize Chunk size of dataset
 * @param[in] rewrite Flag for rewriting existing dataset (optional)
 * @param[in] log Logging flag (optional)
 * @throw std::runtime_error
 */
void File::createDataset(std::string name, hid_t datatypeId, Vector size, Vector chunkSize, bool rewrite, bool log)
{
    hid_t dataspaceId = H5Screate_simple(int(size.getLength()), size.getVectorPtr(), nullptr);
    if (dataspaceId < 0) {
        throw std::runtime_error("H5Screate_simple error");
    }
    if (datatypeId < 0) {
        throw std::runtime_error("HDF5 datatype error");
    }
    hid_t pListId = H5Pcreate(H5P_DATASET_CREATE);
    if (pListId < 0) {
        throw std::runtime_error("H5Pcreate error");
    }

    // Set chunking
    if (chunkSize.getLength() != size.getLength()) {
        throw std::runtime_error("Error - Chunk size length is not equal dataset size length");
    }

    if (!chunkSize.hasZeros()) {
        herr_t err = H5Pset_chunk(pListId, int(chunkSize.getLength()), chunkSize.getVectorPtr());
        if (err < 0) {
            throw std::runtime_error("H5Pset_chunk error");
        }
    } else {
        herr_t err = H5Pset_layout(pListId, H5D_CONTIGUOUS);
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
        token += s.substr(0, pos);
        if (token.length() > 0)
            createGroup(token, false, log);
        token += "/";
        s.erase(0, pos + delimiter.length());
    }

    if (log)
        std::cout << "Creating dataset \"" << name << "\" ";

    if (rewrite) {
        if (objExistsByName(name, fileId)) {
            Dataset *dataset = openDataset(name, false);
            if (H5Tequal(dataset->getDataType(), datatypeId) && dataset->getDims() == size && dataset->getChunkDims() == chunkSize) {
                if (log)
                    std::cout << "... rewriting original space ... OK" << std::endl;
                err = H5Sclose(dataspaceId);
                if (err < 0) {
                    throw std::runtime_error("H5Sclose error");
                }
                err = H5Pclose(pListId);
                if (err < 0) {
                    throw std::runtime_error("H5Pclose error");
                }
                closeDataset(dataset, false);
                return;
            }
            closeDataset(dataset, false);
            if (log)
                std::cout << "... deleting original link";
            H5Ldelete(fileId, name.c_str(), 0);
        }
    }

    hid_t datasetId = H5Dcreate(fileId, name.c_str(), datatypeId, dataspaceId, 0, pListId, 0);
    if (datasetId < 0) {
        if (log)
            std::cout << "... error" << std::endl;
        throw std::runtime_error("H5Dcreate error");
    }

    err = H5Sclose(dataspaceId);
    if (err < 0) {
        throw std::runtime_error("H5Sclose error");
    }
    err = H5Pclose(pListId);
    if (err < 0) {
        throw std::runtime_error("H5Pclose error");
    }
    err = H5Dclose(datasetId);
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
 */
void File::createDataset(const Dataset *dataset, bool rewrite, bool log)
{
    createDataset(dataset->getName(), dataset->getDataType(), dataset->getDims(), dataset->getChunkDims(), rewrite, log);
}

/**
 * @brief Opens dataset by name in HDF5 file
 * @param[in] name Name of dataset
 * @param[in] log Logging flag (optional)
 * @return Opened dataset
 */
Dataset *File::openDataset(std::string name, bool log)
{
    return dynamic_cast<Dataset *>(openObject(name, log));
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
bool File::isDatasetOpened(std::string name) const
{
    return isObjectOpened(name);
}

/**
 * @brief Is dataset opened?
 * @param[in] idx Index of dataset in file
 * @return True/False
 */
bool File::isDatasetOpened(hsize_t idx) const
{
    return isDatasetOpened(getObjNameByIdx(idx));
}

/**
 * @brief Closes dataset with given name in HDF5 file
 * @param[in] name Name of dataset
 * @param[in] log Logging flag (optional)
 */
void File::closeDataset(std::string name, bool log)
{
    closeObject(name, log);
}

/**
 * @brief Closes dataset with given index in HDF5 file
 * @param[in] idx Index of dataset in file
 * @param[in] log Logging flag (optional)
 */
void File::closeDataset(hsize_t idx, bool log)
{
    closeDataset(getObjNameByIdx(idx), log);
}

/**
 * @brief Closes dataset with same name as given dataset has
 * @param[in] dataset Dataset
 * @param[in] log Logging flag (optional)
 */
void File::closeDataset(const Dataset *dataset, bool log)
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
void File::createGroup(std::string name, bool rewrite, bool log) const
{
    if (log)
        std::cout << "Creating group \"" << name << "\" ";
    if (rewrite) {
        H5Ldelete(fileId, name.c_str(), 0);
        if (log)
            std::cout << "... rewrite ";
    }

    if (!objExistsByName(name, fileId)) {
        hid_t groupId = H5Gcreate(fileId, name.c_str(), 0, 0, 0);
        if (groupId < 0) {
            if (log)
                std::cout << "... error" << std::endl;
            throw std::runtime_error("H5Dcreate error");
        }
        H5Gclose(groupId);
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
Group *File::openGroup(std::string name, bool log)
{
    return dynamic_cast<Group *>(openObject(name, log));
}

/**
 * @brief Opens group with given index in HDF5 file
 * @param[in] idx Index of group in file
 * @param[in] log Logging flag (optional)
 * @return Opened group
 */
Group *File::openGroup(hsize_t idx, bool log)
{
    return openGroup(getObjNameByIdx(idx), log);
}

/**
 * @brief Is group opened?
 * @param[in] name Group name
 * @return True/False
 */
bool File::isGroupOpened(std::string name) const
{
    return isObjectOpened(name);
}

/**
 * @brief Is group opened?
 * @param[in] idx Group index
 * @return True/False
 */
bool File::isGroupOpened(hsize_t idx) const
{
    return isDatasetOpened(getObjNameByIdx(idx));
}

/**
 * @brief Closes group with given name in HDF5 file
 * @param[in] name Name of group
 * @param[in] log Logging flag (optional)
 */
void File::closeGroup(std::string name, bool log)
{
    closeObject(name, log);
}

/**
 * @brief Closes group with given index in HDF5 file
 * @param[in] idx Index of group in file
 * @param[in] log Logging flag (optional)
 */
void File::closeGroup(hsize_t idx, bool log)
{
    closeGroup(getObjNameByIdx(idx), log);
}

/**
 * @brief Closes group with same name as given group has
 * @param[in] group Group
 * @param[in] log Logging flag (optional)
 */
void File::closeGroup(const Group *group, bool log)
{
    closeGroup(group->getName(), log);
}

/**
 * @brief Opens object
 * @param[in] name Object name
 * @param[in] log Logging flag (optional)
 * @return Opened object
 */
Object *File::openObject(std::string name, bool log)
{
    std::string nameTmp = fixPath(name);
    if (objects.find(nameTmp) == objects.end()) {
        insertObject(nameTmp, log);
        return openObject(nameTmp, log);
    } else {
        objects.find(nameTmp)->second->setDeleteLog(log);
        return objects.find(nameTmp)->second;
    }
}

/**
 * @brief Opens object
 * @param[in] idx Object index
 * @param[in] log Logging flag (optional)
 * @return Opened object
 */
Object *File::openObject(hsize_t idx, bool log)
{
    return openObject(getObjNameByIdx(idx), log);
}

/**
 * @brief Is object opened?
 * @param[in] name Object name
 * @return True/False
 */
bool File::isObjectOpened(std::string name) const
{
    std::string nameTmp = fixPath(name);
    if (objects.find(nameTmp) == objects.end())
        return false;
    else
        return true;
}

/**
 * @brief Is object opened?
 * @param[in] idx Object index
 * @return True/False
 */
bool File::isObjectOpened(hsize_t idx) const
{
    return isObjectOpened(getObjNameByIdx(idx));
}

/**
 * @brief Closes object with given name in HDF5 file
 * @param[in] name Object name
 * @param[in] log Logging flag (optional)
 */
void File::closeObject(std::string name, bool log)
{
    std::string nameTmp = fixPath(name);
    if (objects.find(nameTmp) != objects.end()) {
        Object *object = objects.find(nameTmp)->second;
        object->setDeleteLog(log);
        if (object) {
            delete object;
            object = nullptr;
        }
        objects.erase(objects.find(nameTmp));
    }
}

/**
 * @brief Closes object with given index in HDF5 file
 * @param[in] idx Object index
 * @param[in] log Logging flag (optional)
 */
void File::closeObject(hsize_t idx, bool log)
{
    closeObject(getObjNameByIdx(idx), log);
}

/**
 * @brief Closes given object in HDF5 file
 * @param[in] object Object
 * @param[in] log Logging flag (optional)
 */
void File::closeObject(const Object *object, bool log)
{
    closeObject(object->getName(), log);
}

/**
 * @brief Returns number of objects in HDF5 file (root group)
 * @param[in] groupId Group id (optional)
 * @return Number of objects in HDF5 file
 * @throw std::runtime_error
 */
hsize_t File::getNumObjs(hid_t groupId) const
{
    hid_t groupIdTmp = groupId;
    if (groupId <= 0)
        groupIdTmp = fileId;

    H5G_info_t group_info;
    herr_t err = H5Gget_info(groupIdTmp, &group_info);
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
std::string File::getObjNameByIdx(hsize_t idx, hid_t groupId) const
{
    hid_t groupIdTmp = groupId;
    if (groupId <= 0)
        groupIdTmp = fileId;

    char *nameC = nullptr;
    size_t size = 0;
    ssize_t sizeR = 0;
    sizeR = H5Gget_objname_by_idx(groupIdTmp, idx, nameC, size);
    if (sizeR <= 0) {
        throw std::runtime_error("H5Gget_objname_by_idx error");
    }
    nameC = new char[size_t(sizeR) + 1]();
    H5Gget_objname_by_idx(groupIdTmp, idx, nameC, size_t(sizeR) + 1);
    std::string name(nameC);
    if (nameC) {
        delete [] nameC;
        nameC = nullptr;
    }
    return name;
}

/**
 * @brief Returns object type by index
 * @param[in] idx Index of object in file
 * @param[in] groupId Group id (optional)
 * @return Object type
 * @throw std::runtime_error
 */
H5G_obj_t File::getObjTypeByIdx(hsize_t idx, hid_t groupId) const
{
    hid_t groupIdTmp = groupId;
    if (groupId <= 0)
        groupIdTmp = fileId;

    int type = 0;
    type = H5Gget_objtype_by_idx(groupIdTmp, idx);
    if (type < 0) {
        throw std::runtime_error("H5Gget_objtype_by_idx error");
    }
    return H5G_obj_t(type);
}

/**
 * @brief Returns object type by name
 * @param[in] name Name of object in file
 * @param[in] groupId Group id (optional)
 * @return Object type
 * @throw std::runtime_error
 */
H5G_obj_t File::getObjTypeByName(std::string name, hid_t groupId) const
{
    hid_t groupIdTmp = groupId;
    if (groupId <= 0)
        groupIdTmp = fileId;
    H5G_stat_t statbuf;
    // H5Gget_objinfo is deprecated, but H5Gget_info_by_name is slow
    herr_t err = H5Gget_objinfo(groupIdTmp, name.c_str(), 1, &statbuf);
    if (err < 0) {
        throw std::runtime_error("H5Oget_info error");
    }
    return H5G_obj_t(statbuf.type);
}

/**
 * @brief Exists object with given name?
 * @param[in] name Object name
 * @param[in] groupId Group id (optional)
 * @return True/False
 */
bool File::objExistsByName(std::string name, hid_t groupId) const
{
    hid_t groupIdTmp = groupId;
    if (groupId <= 0)
        groupIdTmp = fileId;

    // Check every group
    std::string s = name;
    std::string delimiter = "/";
    size_t pos = 0;
    std::string token;
    while ((pos = s.find(delimiter)) != std::string::npos) {
        token += s.substr(0, pos);
        if (token.length() > 0) {
            if (H5Lexists(groupIdTmp, token.c_str(), 0) <= 0) {
                return false;
            } else {
                if (H5Oexists_by_name(groupIdTmp, token.c_str(), 0) <= 0) {
                    return false;
                }
            }
        }
        token += "/";
        s.erase(0, pos + delimiter.length());
    }
    if (H5Lexists(groupIdTmp, name.c_str(), 0) > 0) {
        return (H5Oexists_by_name(groupIdTmp, name.c_str(), 0) > 0);
    } else {
        return false;
    }
}

/**
 * @brief Renames object
 * @param[in] srcName Source object name
 * @param[in] dstName Destination object name
 * @throw std::runtime_error
 */
void File::objRename(std::string srcName, std::string dstName) const
{
    // TODO Check object is not opened
    herr_t err = H5Lmove(fileId, srcName.c_str(), fileId, dstName.c_str(), 0, 0);
    if (err < 0) {
        throw std::runtime_error("H5Lmove error");
    }
}

/**
 * @brief Renames attribute
 * @param[in] srcName Source attribute name
 * @param[in] dstName Destination attribute name
 * @param[in] objectId Object id
 * @throw std::runtime_error
 */
void File::renameAttribute(std::string srcName, std::string dstName, hid_t objectId) const
{
    hid_t objectIdTmp = objectId;
    if (objectId <= 0)
        objectIdTmp = fileId;

    herr_t err = H5Arename(objectIdTmp, srcName.c_str(), dstName.c_str());
    if (err < 0) {
        throw std::runtime_error("H5Arename error");
    }
}

/**
 * @brief Renames attribute
 * @param[in] srcName Source attribute name
 * @param[in] dstName Destination attribute name
 * @param[in] objName Object name
 * @throw std::runtime_error
 */
void File::renameAttribute(std::string srcName, std::string dstName, std::string objName) const
{
    hid_t objectId = H5Oopen(fileId, objName.c_str(), 0);
    if (objectId < 0) {
        throw std::runtime_error("H5Oopen error");
    }

    renameAttribute(srcName, dstName, objectId);

    herr_t err = H5Oclose(objectId);
    if (err < 0) {
        throw std::runtime_error("H5Oopen error");
    }
}

/**
 * @brief Returns filename
 * @return Filename
 */
std::string File::getFilename() const
{
    return filename;
}

/**
 * @brief Returns raw filename
 * @return Raw filename
 */
std::string File::getRawFilename() const
{
    std::string filename = getFilename();
    size_t lastindex = filename.find_last_of(".");
    return filename.substr(0, lastindex);
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
    //std::cout << "Number of elements to load: " << size << " (floats: " << size * 4 << " bytes, unsigned 64-bit integers: " << size * 8 << " bytes)" << std::endl;
    numberOfElementsToLoad = size;
}

/**
 * @brief Returns number of elements to load
 * @return Number of elements to load
 */
hsize_t File::getNumberOfElmsToLoad() const
{
    return numberOfElementsToLoad;
}

/**
 * @brief Returns N dimensions (Nt, Nz, Ny, Nx)
 * @return N dimensions
 */
Vector4D File::getNDims() const
{
    return nDims;
}

/**
 * @brief Returns time point spacing values
 * @return Time point spacing values
 */
Vector4DF File::getDValues() const
{
    return dValues;
}

/**
 * @brief Returns frequency, dt dataset value must be known.
 * @param[in] period Period
 * @return Frequency
 */
float File::getFrequency(float period) const
{
    if (dValues.t() != 0) {
        return 1.0f / (period * dValues.t());
    } else {
        std::cout << "Cannot compute real frequency from period. Time point spacing (dt) is unkown" << std::endl;
        return 0;
    }
}

/**
 * @brief Returns period, dt dataset value must be known.
 * @param[in] frequency Frequency
 * @return Period
 */
float File::getPeriod(float frequency) const
{
    if (dValues.t() != 0) {
        return 1.0f / (frequency * dValues.t());
    } else {
        std::cout << "Cannot compute period from real frequency. Time point spacing (dt) is unkown" << std::endl;
        return 0;
    }
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
 * @brief Sets delete logging flag
 * @param[in] value Delete logging flag
 */
void File::setDeleteLogging(bool value)
{
    deleteLogging = value;
}

/**
 * @brief Inserts object to opened object
 * @param[in] name Object name
 * @param[in] log Logging flag (optional)
 * @throw std::runtime_error
 */
void File::insertObject(std::string name, bool log)
{
    std::string nameTmp = fixPath(name);
    if (getObjTypeByName(nameTmp) == H5G_DATASET) {
        if (log)
            std::cout << "Opening dataset \"" << nameTmp << "\" ";
        hid_t datasetId = H5Dopen(fileId, nameTmp.c_str(), 0);
        if (datasetId < 0) {
            if (log)
                std::cout << "... error" << std::endl;
            throw std::runtime_error("H5Dopen error");
        }
        Dataset *dataset = new Dataset(datasetId, nameTmp, this);
        objects.insert(PairOfObjects(nameTmp, dataset));
        if (log)
            std::cout << "... OK" << std::endl;
    } else if (getObjTypeByName(nameTmp) == H5G_GROUP) {
        if (log)
            std::cout << "Opening group \"" << nameTmp << "\" ";
        hid_t groupId = H5Gopen(fileId, nameTmp.c_str(), 0);
        if (groupId < 0) {
            if (log)
                std::cout << "... error" << std::endl;
            throw std::runtime_error("H5Gopen error");
        }
        Group *group = new Group(groupId, nameTmp, this);
        objects.insert(PairOfObjects(nameTmp, group));
        if (log)
            std::cout << "... OK" << std::endl;
    } else {
        throw std::runtime_error("Object type is not supported");
    }
}

/**
 * @brief Closes file and objects
 */
void File::closeFileAndObjects()
{
    MapOfObjects objectsTmp = objects;
    // Delete all loaded objects
    for (MapOfObjects::iterator it = objectsTmp.begin(); it != objectsTmp.end(); ++it) {
        closeObject(it->second, it->second->getDeleteLog());
    }

    if (deleteLogging)
        std::cout << "Closing file \"" << filename << "\"";
    err = H5Pclose(pListFileAccessId);
    if (err < 0) {
        //throw std::runtime_error("H5Pclose error");
    }
    err = H5Fclose(fileId);
    if (err < 0) {
        //throw std::runtime_error("H5Fclose error");
    }
    if (deleteLogging)
        std::cout << " ... OK" << std::endl;
}

/**
 * @brief Trims slashes of path
 * @param[in] path Path to trim
 * @return Trimmed path
 */
std::string trimSlashes(std::string path)
{
    size_t first = path.find_first_not_of('/');
    if (std::string::npos == first) {
        if (path == "/")
            return "";
        return path;
    }
    size_t last = path.find_last_not_of('/');
    return path.substr(first, (last - first + 1));
}

/**
 * @brief Concatenates path with name
 * @param[in] path Path
 * @param[in] name Name
 * @return Concatenated path with name
 */
std::string concatenatePath(std::string path, std::string name)
{
    if (path == "/")
        return path + trimSlashes(name);
    else
        return path + fixPath(name);
}

/**
 * @brief Fixes path with starting slash and trimming
 * @param[in] path Path
 * @return Fixed path
 */
std::string fixPath(std::string path)
{
    return "/" + trimSlashes(path);
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
 * @brief Returns system physical memory currently used by process
 * @return System physical memory currently used by process
 */
size_t getSystemPhysicalMemoryCurrentlyUsedByProc()
{
#ifdef __unix
    // linux file contains this-process info
    FILE* file = fopen("/proc/self/status", "r");

    char buffer[1024] = "";

    int currRealMem;

    // read the entire file
    while (fscanf(file, " %1023s", buffer) == 1) {
        if (strcmp(buffer, "VmRSS:") == 0) { // kilobytes
            fscanf(file, " %d", &currRealMem);
        }
    }
    fclose(file);
    return size_t(currRealMem);
#endif

#ifdef _WIN32
    PROCESS_MEMORY_COUNTERS pmc;
    GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc));
    return size_t(pmc.WorkingSetSize); // bytes
#endif
}

/**
 * @brief Returns peak system physical memory currently used by process
 * @return Peak system physical memory currently used by process
 */
size_t getPeakSystemPhysicalMemoryCurrentlyUsedByProc()
{
#ifdef __unix
    // linux file contains this-process info
    FILE* file = fopen("/proc/self/status", "r");

    char buffer[1024] = "";

    //int currRealMem;
    int peakRealMem;
    //int currVirtMem;
    //int peakVirtMem;

    // read the entire file
    while (fscanf(file, " %1023s", buffer) == 1) {
        /*if (strcmp(buffer, "VmRSS:") == 0) { // kilobytes
                fscanf(file, " %d", &currRealMem);
            }*/
        if (strcmp(buffer, "VmHWM:") == 0) {
            fscanf(file, " %d", &peakRealMem);
        }
        /*if (strcmp(buffer, "VmSize:") == 0) {
                fscanf(file, " %d", &currVirtMem);
            }
            if (strcmp(buffer, "VmPeak:") == 0) {
                fscanf(file, " %d", &peakVirtMem);
            }*/
    }
    fclose(file);
    return size_t(peakRealMem);
#endif

#ifdef _WIN32
    PROCESS_MEMORY_COUNTERS pmc;
    GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc));
    return size_t(pmc.PeakWorkingSetSize); // bytes
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
 * @param[out] minV Minimal value
 * @param[out] maxV Maximal value
 * @param[in] value Input value
 * @param[out] minVIndex Minimal value index
 * @param[out] maxVIndex Maximal value index
 * @param[in] index Input value index
 *
 * This function exists due to OpenMP pragmas
 *
 * TODO use templates
 */
void checkOrSetMinMaxValue(float &minV, float &maxV, float value, hsize_t &minVIndex, hsize_t &maxVIndex, hsize_t index)
{
    if (minV > value) {
#pragma omp critical
        {
            if (minV > value) {
                minV = value;
                minVIndex = index;
            }
        }
    }
    if (maxV < value) {
#pragma omp critical
        {
            if (maxV < value) {
                maxV = value;
                maxVIndex = index;
            }
        }
    }
}

/**
 * @brief Checks or sets minimal and maximal 64-bit unsigned integer value
 * @param[out] minV Minimal value
 * @param[out] maxV Maximal value
 * @param[in] value Input value
 * @param[out] minVIndex Minimal value index
 * @param[out] maxVIndex Maximal value index
 * @param[in] index Input value index
 *
 * This function exists due to OpenMP pragmas
 *
 * TODO use templates
 */
void checkOrSetMinMaxValue(hsize_t &minV, hsize_t &maxV, hsize_t value, hsize_t &minVIndex, hsize_t &maxVIndex, hsize_t index)
{
    if (minV > value) {
#pragma omp critical
        {
            if (minV > value) {
                minV = value;
                minVIndex = index;
            }
        }
    }
    if (maxV < value) {
#pragma omp critical
        {
            if (maxV < value) {
                maxV = value;
                maxVIndex = index;
            }
        }
    }
}

/**
 * @brief Checks or sets minimal and maximal float value
 * @param[out] minV Minimal value
 * @param[out] maxV Maximal value
 * @param[in] minVI Input minimal value
 * @param[in] maxVI Input maximal value
 * @param[out] minVIndex Minimal value index
 * @param[out] maxVIndex Maximal value index
 * @param[in] minVIIndex Input minimal value index
 * @param[in] maxVIIndex Input maximal value index
 *
 * This function exists due to OpenMP pragmas
 *
 * TODO use templates
 */
void checkOrSetMinMaxValue(float &minV, float &maxV, float minVI, float maxVI, hsize_t &minVIndex, hsize_t &maxVIndex, hsize_t minVIIndex, hsize_t maxVIIndex)
{
    if (minV > minVI) {
#pragma omp critical
        {
            if (minV > minVI) {
                minV = minVI;
                minVIndex = minVIIndex;
            }
        }
    }
    if (maxV < maxVI) {
#pragma omp critical
        {
            if (maxV < maxVI) {
                maxV = maxVI;
                maxVIndex = maxVIIndex;
            }
        }
    }
}

/**
 * @brief Checks or sets minimal and maximal 64-bit unsigned integer value
 * @param[out] minV Minimal value
 * @param[out] maxV Maximal value
 * @param[in] minVI Input minimal value
 * @param[in] maxVI Input maximal value
 * @param[out] minVIndex Minimal value index
 * @param[out] maxVIndex Maximal value index
 * @param[in] minVIIndex Input minimal value index
 * @param[in] maxVIIndex Input maximal value index
 *
 * This function exists due to OpenMP pragmas
 *
 * TODO use templates
 */
void checkOrSetMinMaxValue(hsize_t &minV, hsize_t &maxV, hsize_t minVI, hsize_t maxVI, hsize_t &minVIndex, hsize_t &maxVIndex, hsize_t minVIIndex, hsize_t maxVIIndex)
{
    if (minV > minVI) {
#pragma omp critical
        {
            if (minV > minVI) {
                minV = minVI;
                minVIndex = minVIIndex;
            }
        }
    }
    if (maxV < maxVI) {
#pragma omp critical
        {
            if (maxV < maxVI) {
                maxV = maxVI;
                maxVIndex = maxVIIndex;
            }
        }
    }
}

/**
 * @brief File exists?
 * @param[in] name File name
 * @return True/False
 */
bool fileExists(const std::string& name)
{
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
        float *data = new float[srcDataset->getGeneralBlockDims().getSize()]();
        for (hsize_t i = 0; i < srcDataset->getNumberOfBlocks(); i++) {
            srcDataset->readBlock(i, offset, count, data, log);
            dstDataset->writeDataset(offset, count, data, log);
        }
        delete[] data;
        data = nullptr;
    } else if (H5Tequal(srcDataset->getDataType(), H5T_NATIVE_UINT64)) {
        hsize_t *data = new hsize_t[srcDataset->getGeneralBlockDims().getSize()]();
        for (hsize_t i = 0; i < srcDataset->getNumberOfBlocks(); i++) {
            srcDataset->readBlock(i, offset, count, data, log);
            dstDataset->writeDataset(offset, count, data, log);
        }
        delete[] data;
        data = nullptr;
    } else {
        throw std::runtime_error("Wrong data type of dataset (not float or 64-bit unsigned integer)");
    }

    // Copy attributes
    for (hsize_t i = 0; i < srcDataset->getNumAttrs(); i++) {
        Attribute *attribute = srcDataset->getAttribute(i);
        dstDataset->setAttribute(attribute, log);
        delete attribute;
        attribute = nullptr;
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