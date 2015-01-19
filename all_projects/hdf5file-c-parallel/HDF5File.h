/*
 * @file        HDF5File.h
 * @author      Petr Kleparnik, VUT FIT Brno, xklepa01@stud.fit.vutbr.cz
 * @version     0.0
 * @date        30 July 2014
 *
 * @brief       The header file with HDF5File class declaration.
 *
 * @section     Licence
 * This file is part of hdf5file library for k-Wave h5 processing
 * for preprocessing HDF5 data created by the k-Wave toolbox - http://www.k-wave.org.
 * Copyright © 2014, Petr Kleparnik, VUT FIT Brno.
 * hdf5file library is free software.
 */

#ifndef HDF5FILE_H
#define HDF5FILE_H

#include <hdf5.h>  // HDF5

#include <stdint.h> // int64_t
#include <string>
#include <iostream>
#include <fstream>
#include <map>
#include <algorithm>
#include <limits>

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <mpi.h>

#ifdef __unix
#include <stdexcept>
#include <sys/time.h>
#include <unistd.h>
#include <sys/mman.h>
#endif

#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#endif

#define NUMBER_OF_ELEMENTS_TO_LOAD 256 * 256 * 256

#include <mutex>

namespace HDF5Helper
{
    double getTime();
    size_t getTotalSystemPhysicalMemory();
    size_t getAvailableSystemPhysicalMemory();
}

class HDF5File
{
public:
    HDF5File(std::string filename, MPI_Comm comm, MPI_Info info, unsigned int flag = HDF5File::OPEN, bool log = false);
    ~HDF5File();

    class HDF5Vector3D;

    class HDF5Dataset;
    class HDF5Group;

    typedef std::map<const std::string, HDF5Dataset *> MapOfDatasets;
    typedef std::map<const std::string, HDF5Group *> MapOfGroups;

    HDF5Dataset *openDataset(const std::string datasetName);
    HDF5Dataset *openDataset(hsize_t idx);

    void closeDataset(const std::string datasetName);

    void createDatasetI(const std::string datasetName, hsize_t rank, HDF5Vector3D size, HDF5Vector3D chunk_size, bool rewrite = false);
    void createDatasetF(const std::string datasetName, hsize_t rank, HDF5Vector3D size, HDF5Vector3D chunk_size, bool rewrite = false);

    HDF5Group *openGroup(const std::string groupName);
    HDF5Group *openGroup(hsize_t idx);

    void closeGroup(const std::string groupName);

    void createGroup(const std::string groupName, bool rewrite = false);

    hsize_t getNumObjs();
    std::string getObjNameById(hsize_t id);
    H5G_obj_t getObjTypeById(hsize_t id);

    bool objExistsByName(const std::string name);

    std::string getFilename();

    void convertlinearTo3D(hsize_t index, HDF5File::HDF5Vector3D &position);
    void convert3DToLinear(HDF5File::HDF5Vector3D position, hsize_t &index);

    void setNumberOfElmsToLoad(hsize_t size);
    hsize_t getNumberOfElmsToLoad();

    static const std::string NT;
    static const std::string NX;
    static const std::string NY;
    static const std::string NZ;
    static const unsigned int OPEN = 0;
    static const unsigned int CREATE = 1;

    hsize_t getNT();
    hsize_t getNX();
    hsize_t getNY();
    hsize_t getNZ();

    HDF5Vector3D getNdims();

    std::ofstream *getLogFileStream();

private:
    hsize_t nT;
    hsize_t nX;
    hsize_t nY;
    hsize_t nZ;
    std::string filename;

    hsize_t numberOfElementsToLoad;

    hid_t plist_FILE_ACCESS;

    std::ofstream logFileStream;

    static std::mutex mutex;

    hid_t file; // HDF file handle
    MapOfDatasets datasets;
    MapOfGroups groups;

    void insertDataset(const std::string datasetName);
    void insertGroup(const std::string groupName);

    class HDF5Object;
    herr_t err;
    int mPISize;

    void createDataset(const std::string datasetName, hid_t type, hsize_t rank, HDF5Vector3D size, HDF5Vector3D chunk_size, bool rewrite = false);

};

#endif // HDF5FILE_H
