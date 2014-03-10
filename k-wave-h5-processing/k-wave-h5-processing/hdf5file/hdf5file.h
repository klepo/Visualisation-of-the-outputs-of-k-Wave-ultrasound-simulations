#ifndef HDF5FILE_H
#define HDF5FILE_H

#include <H5Cpp.h>  // HDF5

#include <stdint.h> // int64_t
#include <string>
#include <iostream>
#include <map>

#ifdef __unix
#include <stdexcept>
#endif

class HDF5File
{
public:
    HDF5File(std::string filename, unsigned int flag = HDF5File::OPEN);
    ~HDF5File();

    class HDF5Dataset;
    class HDF5Group;

    HDF5Dataset *openDataset(const H5std_string datasetName);
    HDF5Dataset *openDataset(hsize_t idx);

    void closeDataset(const H5std_string datasetName);

    void createDatasetI(const H5std_string datasetName, hsize_t rank, hsize_t *size, hsize_t *chunk_size = NULL);
    void createDatasetF(const H5std_string datasetName, hsize_t rank, hsize_t *size, hsize_t *chunk_size = NULL);

    HDF5Group *getGroup(const H5std_string groupName);
    void createGroup(const H5std_string groupName);

    void unlinkLocation(const H5std_string name);

    hsize_t getNumObjs();

    void convertlinearTo3D(hsize_t index, hsize_t &x, hsize_t &y, hsize_t &z);
    void convert3DToLinear(hsize_t &x, hsize_t &y, hsize_t &z, hsize_t index);

    static const H5std_string NT;
    static const H5std_string NX;
    static const H5std_string NY;
    static const H5std_string NZ;
    static const unsigned int OPEN = 0;
    static const unsigned int CREATE = 1;
    static const uint64_t HDF5File::SIZE_OF_DATA_PART = 1024*1024*5;

    uint64_t getNT();
    uint64_t getNX();
    uint64_t getNY();
    uint64_t getNZ();

protected:
    uint64_t nT;
    uint64_t nX;
    uint64_t nY;
    uint64_t nZ;
    H5std_string filename;

    H5::H5File file;
    std::map<const H5std_string, HDF5Dataset *> datasets;
    std::map<const H5std_string, HDF5Group *> groups;

    void insertDataset(const H5std_string datasetName);
    void insertGroup(const H5std_string groupName);

private:
    class HDF5Object;

};

#endif // HDF5FILE_H
