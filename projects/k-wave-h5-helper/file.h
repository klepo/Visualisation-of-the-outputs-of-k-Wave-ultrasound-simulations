/**
 * @file        file.h
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        30 July      2014 (created) \n
 *              19 September 2017 (updated)
 *
 * @brief       The header file with H5Helper::File class declaration.
 *
 * @license     This file is part of the k-wave-h5-helper library for processing the HDF5 data
 *              created by the k-Wave toolbox - http://www.k-wave.org. This file may be used,
 *              distributed and modified under the terms of the LGPL version 3 open source
 *              license. A copy of the LGPL license should have been received with this file.
 *              Otherwise, it can be found at: http://www.gnu.org/copyleft/lesser.html.
 *
 * @copyright   Copyright © 2017, Petr Kleparnik, VUT FIT Brno. All Rights Reserved.
 *
 */

#ifndef FILE_H
#define FILE_H

#ifdef __unix
    #include <stdexcept>
    #include <sys/time.h>
    #include <unistd.h>
    #include <sys/mman.h>
#endif

#ifdef _WIN32
    #define NOMINMAX
    #include <Windows.h> // GetTickCount etc.
    #include "psapi.h"
#endif

#include <iostream>
#include <fstream>
#include <algorithm>
#include <time.h>

#ifdef PARALLEL_HDF5
    #include <mpi.h>
#endif

#include <vector3d.h>
#include <vector4d.h>
#include <group.h>
#include <dataset.h>

namespace H5Helper
{
/// Dataset name sensor_mask_type
const std::string SENSOR_MASK_TYPE_DATASET = "sensor_mask_type";
/// Dataset name sensor_mask_index
const std::string SENSOR_MASK_INDEX_DATASET = "sensor_mask_index";
/// Dataset name sensor_mask_corners
const std::string SENSOR_MASK_CORNERS_DATASET = "sensor_mask_corners";
/// Dataset name Nt
const std::string NT_DATASET = "Nt";
/// Dataset name Nx
const std::string NX_DATASET = "Nx";
/// Dataset name Ny
const std::string NY_DATASET = "Ny";
/// Dataset name Nz
const std::string NZ_DATASET = "Nz";
/// Dataset name p_source_input
const std::string P_SOURCE_INPUT_DATASET = "p_source_input";

/// Attribute name min
const std::string MIN_ATTR = "min";
/// Attribute name max
const std::string MAX_ATTR = "max";
/// Attribute name min_index
const std::string MIN_INDEX_ATTR = "min_index";
/// Attribute name max_index
const std::string MAX_INDEX_ATTR = "max_index";
/// Attribute name src_dataset_name
const std::string SRC_DATASET_NAME_ATTR = "src_dataset_name";
/// Attribute name c_type
const std::string C_TYPE_ATTR = "c_type";
/// Attribute name c_mos
const std::string C_MOS_ATTR = "c_mos";
/// Attribute name c_period
const std::string C_PERIOD_ATTR = "c_period";
/// Attribute name c_harmonic
const std::string C_HARMONIC_ATTR = "c_harmonic";
/// Attribute name c_harmonics
const std::string C_HARMONICS_ATTR = "c_harmonics";
/// Attribute name position_z
const std::string POSITION_Z_ATTR = "position_z";
/// Attribute name position_y
const std::string POSITION_Y_ATTR = "position_y";
/// Attribute name position_x
const std::string POSITION_X_ATTR = "position_x";
/// Attribute name src_position_z
const std::string SRC_POSITION_Z_ATTR = "src_position_z";
/// Attribute name src_position_y
const std::string SRC_POSITION_Y_ATTR = "src_position_y";
/// Attribute name src_position_x
const std::string SRC_POSITION_X_ATTR = "src_position_x";
/// Attribute name src_size_z
const std::string SRC_SIZE_Z_ATTR = "src_size_z";
/// Attribute name src_size_y
const std::string SRC_SIZE_Y_ATTR = "src_size_y";
/// Attribute name src_size_x
const std::string SRC_SIZE_X_ATTR = "src_size_x";

/**
 * @brief The File class represents wrapper for the HDF5 files
 */
class File
{
public:
    #ifdef PARALLEL_HDF5
        File(std::string filename, unsigned int flag, MPI_Comm comm, MPI_Info info, bool log = true);
    #else
        File(std::string filename, unsigned int flag, bool log = true);
    #endif

    ~File();

    void createDatasetI(std::string name, Vector size, Vector chunkSize, bool rewrite = false, bool log = true);
    void createDatasetF(std::string name, Vector size, Vector chunkSize, bool rewrite = false, bool log = true);
    void createDataset(std::string name, hid_t type, Vector size, Vector chunkSize, bool rewrite = false, bool log = true);
    void createDataset(Dataset *dataset, bool rewrite = false, bool log = true);
    Dataset *openDataset(std::string name, bool log = true);
    Dataset *openDataset(hsize_t idx, bool log = true);
    bool isDatasetOpened(std::string name) const;
    bool isDatasetOpened(hsize_t idx) const;
    void closeDataset(std::string name, bool log = true);
    void closeDataset(hsize_t idx, bool log = true);
    void closeDataset(Dataset *dataset, bool log = true);

    void createGroup(std::string name, bool rewrite = false, bool log = true) const;
    Group *openGroup(std::string name, bool log = true);
    Group *openGroup(hsize_t idx, bool log = true);
    bool isGroupOpened(std::string name) const;
    bool isGroupOpened(hsize_t idx) const;
    void closeGroup(std::string name, bool log = true);
    void closeGroup(hsize_t idx, bool log = true);
    void closeGroup(Group *group, bool log = true);

    Object *openObject(std::string name, bool log = true);
    Object *openObject(hsize_t idx, bool log = true);
    bool isObjectOpened(std::string name) const;
    bool isObjectOpened(hsize_t idx) const;
    void closeObject(std::string name, bool log = true);
    void closeObject(hsize_t idx, bool log = true);
    void closeObject(Object *object, bool log = true);

    hsize_t getNumObjs(hid_t groupId = -1) const;
    std::string getObjNameByIdx(hsize_t idx, hid_t groupId = -1) const;
    H5G_obj_t getObjTypeByIdx(hsize_t idx, hid_t groupId = -1) const;
    H5G_obj_t getObjTypeByName(std::string name) const;

    bool objExistsByName(std::string name) const;

    void objRename(std::string srcName, std::string dstName) const;

    void renameAttribute(std::string srcName, std::string dstName, hid_t groupId = -1) const;
    void renameAttribute(std::string srcName, std::string dstName, std::string objName) const;

    std::string getFilename() const;

    void setNumberOfElmsToLoad(hsize_t size);
    hsize_t getNumberOfElmsToLoad() const;

    Vector4D getNdims() const;

    int getMPISize() const;

    void setDeleteLog(bool value);

    /// Open file flag
    static const unsigned int OPEN = 0;
    /// Create file flag
    static const unsigned int CREATE = 1;

private:
    File(const File &);
    File &operator=(const File &);
    void insertObject(std::string name, bool log = true);
    void closeFileAndObjects();

    Vector4D nDims;
    std::string filename;
    hsize_t numberOfElementsToLoad = 512 * 512 * 512;
    hid_t plist_FILE_ACCESS;

    /// HDF5 file handle
    hid_t file;
    /// Map of objects
    MapOfObjects objects;

    /// Delete log flag
    bool deleteLog = true;

    herr_t err;
    int mPISize;
};

// General functions
std::string trimSlashes(std::string path);
std::string concatenatePath(std::string path, std::string name);
std::string fixPath(std::string path);
double getTime();
size_t getTotalSystemPhysicalMemory();
size_t getAvailableSystemPhysicalMemory();
size_t getSystemPhysicalMemoryCurrentlyUsedByProc();
size_t getPeakSystemPhysicalMemoryCurrentlyUsedByProc();
void convertlinearToMultiDim(hsize_t index, Vector &position, Vector size);
void convertMultiDimToLinear(Vector position, hsize_t &index, Vector size);
void checkOrSetMinMaxValue(float &minV, float &maxV, float value, hsize_t &minVIndex, hsize_t &maxVIndex, hsize_t index);
void checkOrSetMinMaxValue(hsize_t &minV, hsize_t &maxV, hsize_t value, hsize_t &minVIndex, hsize_t &maxVIndex, hsize_t index);
void checkOrSetMinMaxValue(float &minV, float &maxV, float minVI, float maxVI, hsize_t &minVIndex, hsize_t &maxVIndex, hsize_t minVIIndex, hsize_t maxVIIndex);
void checkOrSetMinMaxValue(hsize_t &minV, hsize_t &maxV, hsize_t minVI, hsize_t maxVI, hsize_t &minVIndex, hsize_t &maxVIndex, hsize_t minVIIndex, hsize_t maxVIIndex);
bool fileExists(const std::string& name);
void copyDataset(Dataset *srcDataset, File *dstFile, bool rewrite = true, bool log = true);
void copyDataset(File *srcFile, File *dstFile, std::string name, bool rewrite = true, bool log = true);
}

#endif // FILE_H
