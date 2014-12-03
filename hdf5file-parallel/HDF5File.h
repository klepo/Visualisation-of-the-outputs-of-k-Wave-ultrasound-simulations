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

#ifdef __unix
#include <stdexcept>
#include <sys/time.h>
#endif

#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#endif

#define SIZE_OF_DATA_PART 4096*4096*2

#include <mutex>

class HDF5File
{
public:
    HDF5File(std::string filename, unsigned int flag = HDF5File::OPEN, bool log = false);
    ~HDF5File();

    class HDF5Dataset;
    class HDF5Group;

    HDF5Dataset *openDataset(const std::string datasetName);
    HDF5Dataset *openDataset(hsize_t idx);

    void closeDataset(const std::string datasetName);

    void createDatasetI(const std::string datasetName, hsize_t rank, hsize_t *size, hsize_t *chunk_size = ZERO_CHUNK, bool rewrite = false);
    void createDatasetF(const std::string datasetName, hsize_t rank, hsize_t *size, hsize_t *chunk_size = ZERO_CHUNK, bool rewrite = false);

    HDF5Group *openGroup(const std::string groupName);
    HDF5Group *openGroup(hsize_t idx);

    void closeGroup(const std::string groupName);

    void createGroup(const std::string groupName, bool rewrite = false);

    //void unlinkLocation(const std::string name);

    hsize_t getNumObjs();
    std::string getObjNameById(hsize_t id);
    H5G_obj_t getObjTypeById(hsize_t id);

    std::string getFilename();

    void convertlinearTo3D(hsize_t index, hsize_t &z, hsize_t &y, hsize_t &x);
    void convert3DToLinear(hsize_t z, hsize_t y, hsize_t x, hsize_t &index);

    static double getTime();

    void setSizeOfDataPart(uint64_t size);
    uint64_t getSizeOfDataPart();

    static const std::string NT;
    static const std::string NX;
    static const std::string NY;
    static const std::string NZ;
    static const unsigned int OPEN = 0;
    static const unsigned int CREATE = 1;
    static hsize_t ZERO_CHUNK[3];

    uint64_t getNT();
    uint64_t getNX();
    uint64_t getNY();
    uint64_t getNZ();

    std::ofstream *getLogFileStream();

protected:
    uint64_t nT;
    uint64_t nX;
    uint64_t nY;
    uint64_t nZ;
    std::string filename;

    std::ofstream logFileStream;

    static std::mutex mutex;

    hid_t file; // HDF file handle
    std::map<const std::string, HDF5Dataset *> datasets;
    std::map<const std::string, HDF5Group *> groups;

    void insertDataset(const std::string datasetName);
    void insertGroup(const std::string groupName);

private:
    class HDF5Object;
    uint64_t sizeOfDataPart;

    void createDataset(const std::string datasetName, hid_t type, hsize_t rank, hsize_t *size, hsize_t *chunk_size = ZERO_CHUNK, bool rewrite = false);

};

#endif // HDF5FILE_H
