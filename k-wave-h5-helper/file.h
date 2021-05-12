/**
 * @file        file.h
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        30 July      2014 (created) <br>
 *              27 March     2019 (updated)
 *
 * @brief       The header file with H5Helper::File class declaration.
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

#ifndef FILE_H
#define FILE_H

#ifdef __unix
#include <sys/time.h>
#include <unistd.h>
#include <sys/mman.h>
#endif

#ifdef _WIN32
#define NOMINMAX
#include <Windows.h> // GetTickCount etc.
#include <Psapi.h>
#endif

#include <stdexcept>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <time.h>
#include <map>

#ifdef PARALLEL_HDF5
#include <mpi.h>
#endif

#include <hdf5.h> // HDF5

#include "helper.h"
#include "vector4d.h"
#include "vector4d.cpp"

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
/// Dataset name dt
const std::string DT_DATASET = "dt";
/// Dataset name dx
const std::string DX_DATASET = "dx";
/// Dataset name dy
const std::string DY_DATASET = "dy";
/// Dataset name dz
const std::string DZ_DATASET = "dz";
/// Dataset name p_source_input
const std::string P_SOURCE_INPUT_DATASET = "p_source_input";
/// Dataset name p
const std::string P_INDEX_DATASET = "p";
/// Dataset name p/1
const std::string P_CUBOID_DATASET = "p/1";
/// Dataset name p_c
const std::string P_INDEX_DATASET_C = "p_c";
/// Dataset name p_c/1
const std::string P_CUBOID_DATASET_C = "p_c/1";

/// Attribute name min
const std::string MIN_ATTR = "min";
/// Attribute name max
const std::string MAX_ATTR = "max";
/// Attribute name min_index
const std::string MIN_INDEX_ATTR = "min_index";
/// Attribute name max_index
const std::string MAX_INDEX_ATTR = "max_index";
/// Attribute name period
const std::string PERIOD_ATTR = "period";
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

class Object;
class Dataset;
class Group;

/// Map of objects datatype
typedef std::map<std::string, Object *> MapOfObjects;

/// Pair of objects datatype
typedef std::pair<std::string, Object *> PairOfObjects;

/**
 * @brief The File class represents wrapper for the HDF5 files
 */
class File
{
public:
#ifdef PARALLEL_HDF5
    File(std::string filename, unsigned int flag, MPI_Comm comm, MPI_Info info);
#else
    File(std::string filename, unsigned int flag);
#endif

    ~File();

    void createDatasetI(std::string name, Vector size, Vector chunkSize, bool rewrite = false);
    void createDatasetF(std::string name, Vector size, Vector chunkSize, bool rewrite = false);
    void createDataset(std::string name, hid_t datatypeId, Vector size, Vector chunkSize, bool rewrite = false);
    void createDataset(const Dataset *dataset, bool rewrite = false);
    Dataset *openDataset(std::string name);
    Dataset *openDataset(hsize_t idx);
    bool isDatasetOpened(std::string name) const;
    bool isDatasetOpened(hsize_t idx) const;
    void closeDataset(std::string name);
    void closeDataset(hsize_t idx);
    void closeDataset(const Dataset *dataset);

    void createGroup(std::string name, bool rewrite = false) const;
    Group *openGroup(std::string name);
    Group *openGroup(hsize_t idx);
    bool isGroupOpened(std::string name) const;
    bool isGroupOpened(hsize_t idx) const;
    void closeGroup(std::string name);
    void closeGroup(hsize_t idx);
    void closeGroup(const Group *group);

    Object *openObject(std::string name);
    Object *openObject(hsize_t idx);
    bool isObjectOpened(std::string name) const;
    bool isObjectOpened(hsize_t idx) const;
    void closeObject(std::string name);
    void closeObject(hsize_t idx);
    void closeObject(const Object *object);

    hsize_t getNumObjs(hid_t groupId = -1) const;
    std::string getObjNameByIdx(hsize_t idx, hid_t groupId = -1) const;
    H5G_obj_t getObjTypeByIdx(hsize_t idx, hid_t groupId = -1) const;
    H5G_obj_t getObjTypeByName(std::string name, hid_t groupId = -1) const;
    bool objExistsByName(std::string name, hid_t groupId = -1) const;

    void objRename(std::string srcName, std::string dstName) const;

    void renameAttribute(std::string srcName, std::string dstName, hid_t objectId = -1) const;
    void renameAttribute(std::string srcName, std::string dstName, std::string objName) const;

    std::string getFilename() const;
    std::string getRawFilename() const;

    void setNumberOfElmsToLoad(hsize_t size);
    hsize_t getNumberOfElmsToLoad() const;

    Vector4D getNDims() const;
    Vector4DF getDValues() const;

    float getFrequency(float period) const;
    float getPeriod(float frequency) const;

    int getMPISize() const;

    /// Open file flag
    static const unsigned int OPEN = 0;
    /// Create file flag
    static const unsigned int CREATE = 1;

private:
    /// Disable copy constructor
    File(const File &);
    /// Disable assignment operator
    /// \return File
    File &operator=(const File &);

    void insertObject(std::string name);
    void closeFileAndObjects();

    /// Domain dimensions
    Vector4D nDims;
    /// Grid point spacing
    Vector4DF dValues;
    /// Filename
    std::string filename = "";
    /// Number of elements to load for block reading
    hsize_t numberOfElementsToLoad = 512 * 512 * 512;
    /// File access property list id
    hid_t pListFileAccessId = -1;
    /// File id
    hid_t fileId = -1;
    /// Map of objects
    MapOfObjects objects;
    /// Error handle
    herr_t err = -1;
    /// MPI size
    int mPISize = 0;
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
void copyDataset(Dataset *srcDataset, File *dstFile, bool rewrite = true);
void copyDataset(File *srcFile, File *dstFile, std::string name, bool rewrite = true);
}

#endif // FILE_H
