#include "hdf5file.h"

#include <iostream>
#include <string>
#include <stdint.h> // int64_t
#include <cfloat>   // FLT_MAX
#include <climits>  // INT64_MAX
#include <time.h>   // clock()
#include <math.h>   // llround

#ifdef __unix
    #include <stdexcept>
#endif

double round(double number)
{
    return number < 0.0 ? ceil(number - 0.5) : floor(number + 0.5);
}

const H5std_string HDF5File::NT("Nt");
const H5std_string HDF5File::NX("Nx");
const H5std_string HDF5File::NY("Ny");
const H5std_string HDF5File::NZ("Nz");

float HDF5File::HDF5Dataset::FNULL = NULL;

HDF5File::HDF5Object::HDF5Object(H5::H5Object *_object)
{
    object = _object;
}

HDF5File::HDF5Object::~HDF5Object()
{

}

HDF5File::HDF5Group::HDF5Group(H5::Group _group) : HDF5Object(&_group) {
    group = _group;
    object = &group;
}

HDF5File::HDF5Group::~HDF5Group()
{
    group.close();
}

HDF5File::HDF5Dataset::HDF5Dataset(H5::DataSet _dataset, H5std_string _name) : HDF5Object(&_dataset)
{
    name = _name;
    dataset = _dataset;
    object = &dataset;
    dataspace = dataset.getSpace();
    type_class = dataset.getTypeClass();

    if (type_class == H5T_FLOAT)
        typeF = dataset.getFloatType();
    else if (type_class == H5T_INTEGER)
        typeI = dataset.getIntType();
    else
        throw std::runtime_error("Wrong data type of dataset");

    rank = dataspace.getSimpleExtentNdims();
    dims = new hsize_t[rank]();

    dataspace.getSimpleExtentDims(dims, NULL);

    size = 1;
    for (hsize_t i = 0; i < rank; i++)
        size *= dims[i];

    downsampling = 1;

    maxVF = 0;
    minVF = 0;

    maxVI = 0;
    minVI = 0;

    dataI = NULL;
    dataF = NULL;

    issetGlobalMinAndMaxValue = false;
}

HDF5File::HDF5Dataset::~HDF5Dataset()
{
    std::cout << "Closing dataset \"" << name << "\"";
    delete [] dataI;
    delete [] dataF;
    delete [] dims;
    dataset.close();
    std::cout << " ... OK" << std::endl;
}

HDF5File::HDF5File(std::string _filename, unsigned int flag)
{
    filename = _filename;
    // Try block to detect exceptions raised by any of the calls inside it
    try {
        H5::Exception::dontPrint();
        H5::FileAccPropList access_plist = H5::FileAccPropList::DEFAULT;
        H5::FileCreatPropList create_plist = H5::FileCreatPropList::DEFAULT;
        //list.setSieveBufSize(1048576);
        //std::cout << "SieveBufSize: " << file.getAccessPlist().getSieveBufSize() << std::endl;

        if (flag == HDF5File::OPEN) {
            std::cout << "Opening file \"" << filename << "\"";
            file = H5::H5File(filename, H5F_ACC_RDWR, create_plist, access_plist);
            std::cout << " ... OK " << std::endl;

            insertDataset(HDF5File::NT);
            insertDataset(HDF5File::NX);
            insertDataset(HDF5File::NY);
            insertDataset(HDF5File::NZ);

            //Set dimensions
            uint64_t *data = NULL;
            data = HDF5File::openDataset(HDF5File::NT)->readFullDatasetI();
            nT = data[0];
            data = HDF5File::openDataset(HDF5File::NX)->readFullDatasetI();
            nX = data[0];
            data = HDF5File::openDataset(HDF5File::NY)->readFullDatasetI();
            nY = data[0];
            data = HDF5File::openDataset(HDF5File::NZ)->readFullDatasetI();
            nZ = data[0];

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

HDF5File::~HDF5File()
{
    for (std::map<const H5std_string, HDF5Dataset *>::iterator it = datasets.begin(); it != datasets.end(); ++it) {
        delete it->second;
    }
    std::cout << "Closing file \"" << filename << "\"";
    file.close();
    std::cout << " ... OK" << std::endl;

}

void HDF5File::insertDataset(const H5std_string datasetName)
{
    try {
        std::cout << "Opening dataset \"" << datasetName << "\"";
        HDF5Dataset *hDF5Dataset = new HDF5Dataset(file.openDataSet(datasetName), datasetName);
        std::cout << " ... OK" << std::endl;
        datasets.insert(std::pair<const H5std_string, HDF5Dataset*>(datasetName, hDF5Dataset));
    } catch(H5::FileIException error) {
        std::cout << " ... error" << std::endl;
        //error.printError();
        throw std::runtime_error(std::string("Dataset \"" + datasetName + "\" does not exist").c_str());
    }
}

void HDF5File::insertGroup(const H5std_string groupName)
{
    try {
        std::cout << "Opening group \"" << groupName << "\"";
        HDF5Group *hDF5Group = new HDF5Group(file.openGroup(groupName));
        std::cout << " ... OK" << std::endl;
        groups.insert(std::pair<const H5std_string, HDF5Group*>(groupName, hDF5Group));
    } catch(H5::FileIException error) {
        std::cout << " ... error" << std::endl;
        //error.printError();
        throw std::runtime_error(std::string("Group \"" + groupName + "\" does not exist").c_str());
    }
}

void HDF5File::createDatasetI(const H5std_string datasetName, hsize_t rank, hsize_t *size, hsize_t *chunk_size)
{
    try {
        const H5::DataSpace dataspace((int) rank, size);
        const H5::DataType datatype(H5::PredType::NATIVE_UINT64);

        const H5::DSetCreatPropList list = H5::DSetCreatPropList::DEFAULT;
        if (chunk_size != NULL)
            list.setChunk((int) rank, chunk_size);

        std::cout << "Creating dataset \"" << datasetName << "\"";
        file.createDataSet(datasetName, datatype, dataspace, list);
        std::cout << " ... OK" << std::endl;
    }
    // catch failure caused by the H5File operations
    catch(H5::FileIException error) {
        std::cout << " ... error" << std::endl;
        error.printError();
        throw std::runtime_error(error.getCDetailMsg());
    } catch(H5::GroupIException error) {
        std::cout << " ... error" << std::endl;
        error.printError();
        throw std::runtime_error(error.getCDetailMsg());
    }
}

void HDF5File::createDatasetF(const H5std_string datasetName, hsize_t rank, hsize_t *size, hsize_t *chunk_size)
{
    try {
        const H5::DataSpace dataspace((int) rank, size);
        const H5::DataType datatype(H5::PredType::NATIVE_FLOAT);

        const H5::DSetCreatPropList list = H5::DSetCreatPropList::DEFAULT;
        if (chunk_size != NULL)
            list.setChunk((int) rank, chunk_size);

        std::cout << "Creating dataset \"" << datasetName << "\"";
        file.createDataSet(datasetName, datatype, dataspace, list);
        std::cout << " ... OK" << std::endl;
    }
    // catch failure caused by the H5File operations
    catch(H5::FileIException error) {
        std::cout << " ... error" << std::endl;
        error.printError();
        throw std::runtime_error(error.getCDetailMsg());
    } catch(H5::GroupIException error) {
        std::cout << " ... error" << std::endl;
        error.printError();
        throw std::runtime_error(error.getCDetailMsg());
    }
}

void HDF5File::createGroup(const H5std_string name)
{
    try {
        std::cout << "Creating group \"" << name << "\"";
        file.createGroup(name);
        std::cout << " ... OK" << std::endl;
    }
    // catch failure caused by the H5File operations
    catch(H5::FileIException error) {
        std::cout << " ... error" << std::endl;
        error.printError();
        throw std::runtime_error(error.getCDetailMsg());
    } catch(H5::GroupIException error) {
        std::cout << " ... error" << std::endl;
        error.printError();
        throw std::runtime_error(error.getCDetailMsg());
    }
}

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

HDF5File::HDF5Dataset *HDF5File::openDataset(const H5std_string datasetName)
{
    if (datasets.find(datasetName) == datasets.end()) {
        HDF5File::insertDataset(datasetName);
        return HDF5File::openDataset(datasetName);
    } else
        return datasets.find(datasetName)->second;
}

void HDF5File::closeDataset(const H5std_string datasetName)
{
    if (datasets.find(datasetName) != datasets.end()){
        delete datasets.find(datasetName)->second;
        datasets.erase(datasets.find(datasetName));
    }
}

HDF5File::HDF5Group *HDF5File::getGroup(const H5std_string groupName)
{
    if (groups.find(groupName) == groups.end()) {
        HDF5File::insertGroup(groupName);
        return HDF5File::getGroup(groupName);
    } else
        return groups.find(groupName)->second;
}

void HDF5File::unlinkLocation(const H5std_string name)
{
    try {
        std::cout << "Unlink location \"" << name << "\"";
        file.unlink(name);
        std::cout << " ... OK" << std::endl;
    } catch(H5::FileIException error) {
        std::cout << " ... error" << std::endl;
        error.printError();
        throw std::runtime_error(error.getCDetailMsg());
    }
}

hsize_t HDF5File::getNumObjs()
{
    return file.getNumObjs();
}

uint64_t HDF5File::getNT()
{
    return nT;
}

uint64_t HDF5File::getNX()
{
    return nX;
}

uint64_t HDF5File::getNY()
{
    return nY;
}

uint64_t HDF5File::getNZ()
{
    return nZ;
}

H5std_string HDF5File::HDF5Dataset::getName()
{
    return name;
}

hsize_t HDF5File::HDF5Dataset::getRank()
{
    return rank;
}

hsize_t *HDF5File::HDF5Dataset::getDims()
{
    return dims;
}

H5T_class_t HDF5File::HDF5Dataset::getDataType()
{
    return type_class;
}

hsize_t *HDF5File::HDF5Dataset::getChunkDims()
{
    H5::DSetCreatPropList cparms = dataset.getCreatePlist();
    hsize_t *chunk_dims = new hsize_t[rank]();
    if (H5D_CHUNKED == cparms.getLayout()) {
        cparms.getChunk((int) rank, chunk_dims);
        return chunk_dims;
    } else
        return NULL;
}

hsize_t HDF5File::HDF5Dataset::getDownsampling()
{
    return downsampling;
}

void HDF5File::HDF5Dataset::setDownsampling(hsize_t _downsampling)
{
    downsampling = _downsampling;
    issetGlobalMinAndMaxValue = false;
}

uint64_t HDF5File::HDF5Dataset::getGlobalMaxValueI(bool reset)
{
    if (type_class == H5T_FLOAT)
        throw std::runtime_error("Wrong data type of dataset (not integer)");
    if (issetGlobalMinAndMaxValue != true)
        HDF5Dataset::findAndSetGlobalMinAndMaxValue(reset);
    return maxVI;
}

uint64_t HDF5File::HDF5Dataset::getGlobalMinValueI(bool reset)
{
    if (type_class == H5T_FLOAT)
        throw std::runtime_error("Wrong data type of dataset (not integer)");
    if (issetGlobalMinAndMaxValue != true)
        HDF5Dataset::findAndSetGlobalMinAndMaxValue(reset);
    return minVI;
}

float HDF5File::HDF5Dataset::getGlobalMaxValueF(bool reset)
{
    if (type_class == H5T_INTEGER)
        throw std::runtime_error("Wrong data type of dataset (not float)");
    if (issetGlobalMinAndMaxValue != true)
        HDF5Dataset::findAndSetGlobalMinAndMaxValue(reset);
    return maxVF;
}

float HDF5File::HDF5Dataset::getGlobalMinValueF(bool reset)
{
    if (type_class == H5T_INTEGER)
        throw std::runtime_error("Wrong data type of dataset (not float)");
    if (issetGlobalMinAndMaxValue != true)
        HDF5Dataset::findAndSetGlobalMinAndMaxValue(reset);
    return minVF;
}

float *HDF5File::HDF5Dataset::readFullDatasetF()
{
    if (type_class == H5T_FLOAT) {
        dataspace = dataset.getSpace();
        hsize_t *stride = new hsize_t[rank]();
        hsize_t *block = new hsize_t[rank]();
        hsize_t *offset = new hsize_t[rank]();
        hsize_t *count = new hsize_t[rank]();
        size = 1;
        for (hsize_t i = 0; i < rank; i++) {
            stride[i] = downsampling;
            block[i] = 1;
            offset[i] = 0;
            count[i] = (hsize_t) round((double) dims[i] / downsampling);
            size *= count[i];
        }

        try {
            dataspace.selectHyperslab(H5S_SELECT_SET, count, offset, stride, block);
            H5::DataSpace memspace((int) rank, count);
            memspace.selectHyperslab(H5S_SELECT_SET, count, offset);

            try {
                dataF = new float[size](); // TODO
            } catch (std::bad_alloc e) {
                throw std::runtime_error(std::string("There is not enough memory to allocate dataset (dataset size: " + std::to_string(size) + " floats)").c_str());
            }

            int t4 = clock();
            //std::cout << "memory before dataset.read    " << ShowMemoryUsageInMB() << " MB" << std::endl;
            dataset.read(dataF, typeF, memspace, dataspace);
            //std::cout << "memory after dataset.read     " << ShowMemoryUsageInMB() << " MB" << std::endl;
            int t5 = clock();
            std::cout << "dwnsmpl: " << downsampling << " time: " << (t5-t4) / (CLOCKS_PER_SEC / 1000) << " ms;" << std::endl;
        } catch(H5::DataSpaceIException error) {
            error.printError();
            throw std::runtime_error(error.getCDetailMsg());
        } catch(H5::DataSetIException error) {
            error.printError();
            throw std::runtime_error(error.getCDetailMsg());
        }

        delete [] stride;
        delete [] block;
        delete [] offset;
        delete [] count;

    } else
        throw std::runtime_error("Wrong data type of dataset (not float)");
    return dataF;
}

float *HDF5File::HDF5Dataset::readSlabOfDatasetF(int dimension, hsize_t index, hsize_t &Y, hsize_t &X)
{
    return HDF5Dataset::readSlabOfDatasetF(dimension, index, Y, X, FNULL, FNULL);
}

// TODO asi smazat
float *HDF5File::HDF5Dataset::readSlabOfDatasetF(int dimension, hsize_t index, hsize_t &Y, hsize_t &X, float &minVF, float &maxVF)
{
    if (type_class != H5T_FLOAT) throw std::runtime_error("Wrong data type of dataset (not float)");
    if (rank != 3) throw std::runtime_error("Wrong rank - dataset is not 3D matrix");
    //if (index < 0) throw std::runtime_error("Wrong index - index is less than 0");
    hsize_t offset[3];   // hyperslab offset in the file
    hsize_t count[3];    // size of the hyperslab in the file
    if (dimension == 0) {
        if (index > dims[0]) throw std::runtime_error("Wrong index - index is greater than size");
        offset[0] = index;
        offset[1] = 0;
        offset[2] = 0;
        count[0] = 1;
        count[1] = (hsize_t) round((double) dims[1] / downsampling);
        count[2] = (hsize_t) round((double) dims[2] / downsampling);
        X = count[1];
        Y = count[2];
    } else if (dimension == 1) {
        if (index > dims[1]) throw std::runtime_error("Wrong index - index is greater than size");
        offset[0] = 0;
        offset[1] = index;
        offset[2] = 0;
        count[0] = (hsize_t) round((double) dims[0] / downsampling);
        count[1] = 1;
        count[2] = (hsize_t) round((double) dims[2] / downsampling);
        X = count[0];
        Y = count[2];
    } else if (dimension == 2) {
        if (index > dims[2]) throw std::runtime_error("Wrong index - index is greater than size");
        offset[0] = 0;
        offset[1] = 0;
        offset[2] = index;
        count[0] = (hsize_t) round((double) dims[0] / downsampling);
        count[1] = (hsize_t) round((double) dims[1] / downsampling);
        count[2] = 1;
        X = count[0];
        Y = count[1];
    } else throw std::runtime_error("Wrong dimension");

    hsize_t stride[3] = {downsampling, downsampling, downsampling};
    hsize_t block[3] = {1, 1, 1};
    float *data_out;

    try {
        dataspace.selectHyperslab(H5S_SELECT_SET, count, offset, stride, block);
        hsize_t offset_out[2];   // hyperslab offset in memory
        hsize_t count_out[2];    // size of the hyperslab in memory
        offset_out[0] = 0;
        offset_out[1] = 0;
        count_out[0]  = X;
        count_out[1]  = Y;

        H5::DataSpace memspace(2, count_out);
        memspace.selectHyperslab(H5S_SELECT_SET, count_out, offset_out);

        try {
            data_out = new float[X * Y]();
        } catch (std::bad_alloc e) {
            throw std::runtime_error(std::string("There is not enough memory to allocate dataset (dataset size: " + std::to_string(X * Y) + " floats)").c_str());
        }

        int t4 = clock();
        //std::cout << "memory before dataset.read    " << ShowMemoryUsageInMB() << " MB" << std::endl;
        dataset.read(data_out, typeF, memspace, dataspace);
        //std::cout << "memory after dataset.read     " << ShowMemoryUsageInMB() << " MB" << std::endl;
        int t5 = clock();
        std::cout << "dwnsmpl: " << downsampling << "; time: " << (t5-t4) / (CLOCKS_PER_SEC / 1000) << " ms; \t" << " dim: " << dimension << "; index: " << index << std::endl;
        //if (minVF != FNULL && maxVF != FNULL)
        HDF5Dataset::getMinAndMaxValueF(data_out, X * Y, minVF, maxVF);
    } catch(H5::DataSpaceIException error) {
        error.printError();
        throw std::runtime_error(error.getCDetailMsg());
    } catch(H5::DataSetIException error) {
        error.printError();
        throw std::runtime_error(error.getCDetailMsg());
    }

    return data_out;
}

float *HDF5File::HDF5Dataset::readDatasetF(hsize_t xO, hsize_t yO, hsize_t zO, hsize_t xC, hsize_t yC, hsize_t zC, float &minVF, float &maxVF)
{
    float *data_out;
    if (type_class != H5T_FLOAT) throw std::runtime_error("Wrong data type of dataset (not float)");
    HDF5Dataset::checkOffsetAndCountParams(xO, yO, zO, xC, yC, zC);

    hsize_t offset[3];   // hyperslab offset in the file
    hsize_t count[3];    // size of the hyperslab in the file
    offset[0] = xO;
    offset[1] = yO;
    offset[2] = zO;
    count[0] = xC;
    count[1] = yC;
    count[2] = zC;
    hsize_t mem_offset[3];
    mem_offset[0] = 0;
    mem_offset[1] = 0;
    mem_offset[2] = 0;

    try {
        dataspace.selectHyperslab(H5S_SELECT_SET, count, offset);

        H5::DataSpace memspace(3, count);
        memspace.selectHyperslab(H5S_SELECT_SET, count, mem_offset);

        try {
            data_out = new float[xC * yC * zC]();
        } catch (std::bad_alloc e) {
            throw std::runtime_error(std::string("There is not enough memory to allocate dataset (dataset size: " + std::to_string(xC * yC * zC) + " floats)").c_str());
        }

        int t4 = clock();
        //std::cout << "memory before dataset.read    " << ShowMemoryUsageInMB() << " MB" << std::endl;
        dataset.read(data_out, typeF, memspace, dataspace);
        //std::cout << "memory after dataset.read     " << ShowMemoryUsageInMB() << " MB" << std::endl;
        int t5 = clock();
        std::cout << "time: " << (t5-t4) / (CLOCKS_PER_SEC / 1000) << " ms; \t" << " offset: " << offset[0] << " x " << offset[1] << " x " << offset[2] << "; count: " << count[0] << " x " << count[1] << " x " << count[2] << std::endl;
        //if (minVF != FNULL && maxVF != FNULL)
        HDF5Dataset::getMinAndMaxValueF(data_out, xC * yC * zC, minVF, maxVF);
    } catch(H5::DataSpaceIException error) {
        error.printError();
        throw std::runtime_error(error.getCDetailMsg());
    } catch(H5::DataSetIException error) {
        error.printError();
        throw std::runtime_error(error.getCDetailMsg());
    }

    return data_out;
}

void HDF5File::HDF5Dataset::writeDataset(hsize_t xO, hsize_t yO, hsize_t zO, hsize_t xC, hsize_t yC, hsize_t zC, float *data)
{
    if (type_class != H5T_FLOAT) throw std::runtime_error("Wrong data type of dataset (not float)");
    HDF5Dataset::checkOffsetAndCountParams(xO, yO, zO, xC, yC, zC);

    hsize_t offset[3];   // hyperslab offset in the file
    hsize_t count[3];    // size of the hyperslab in the file
    offset[0] = xO;
    offset[1] = yO;
    offset[2] = zO;
    count[0] = xC;
    count[1] = yC;
    count[2] = zC;
    hsize_t mem_offset[3];
    mem_offset[0] = 0;
    mem_offset[1] = 0;
    mem_offset[2] = 0;

    try {
        dataspace.selectHyperslab(H5S_SELECT_SET, count, offset);

        H5::DataSpace memspace(3, count);
        memspace.selectHyperslab(H5S_SELECT_SET, count, mem_offset);

        int t4 = clock();
        //std::cout << "memory before dataset.read    " << ShowMemoryUsageInMB() << " MB" << std::endl;
        dataset.write(data, typeF, memspace, dataspace);
        //std::cout << "memory after dataset.read     " << ShowMemoryUsageInMB() << " MB" << std::endl;
        int t5 = clock();
        std::cout << "time: " << (t5-t4) / (CLOCKS_PER_SEC / 1000) << " ms; \t" << " offset: " << offset[0] << " x " << offset[1] << " x " << offset[2] << "; count: " << count[0] << " x " << count[1] << " x " << count[2] << std::endl;
    } catch(H5::DataSpaceIException error) {
        error.printError();
        throw std::runtime_error(error.getCDetailMsg());
    } catch(H5::DataSetIException error) {
        error.printError();
        throw std::runtime_error(error.getCDetailMsg());
    }
}

uint64_t *HDF5File::HDF5Dataset::readFullDatasetI()
{
    if (type_class == H5T_INTEGER) {
        H5::DataSpace memspace((int) rank, dims);

        try {
            dataI = new uint64_t[size]();
        } catch (std::bad_alloc e) {
            throw std::runtime_error(std::string("There is not enough memory to allocate dataset (dataset size: " + std::to_string(size) + " unsigned 64-bit integers)").c_str());
        }
        try {
            dataset.read(dataI, typeI, memspace, dataspace);
        } catch(H5::DataSetIException error) {
            error.printError();
            throw std::runtime_error(error.getCDetailMsg());
        }

    } else
        throw std::runtime_error("Wrong data type of dataset (not integer)");
    return dataI;
}

uint64_t *HDF5File::HDF5Dataset::readDatasetI(hsize_t xO, hsize_t yO, hsize_t zO, hsize_t xC, hsize_t yC, hsize_t zC, uint64_t &minVI, uint64_t &maxVI)
{
    uint64_t *data_out;
    if (type_class != H5T_INTEGER) throw std::runtime_error("Wrong data type of dataset (not integer)");
    HDF5Dataset::checkOffsetAndCountParams(xO, yO, zO, xC, yC, zC);

    hsize_t offset[3];   // hyperslab offset in the file
    hsize_t count[3];    // size of the hyperslab in the file
    offset[0] = xO;
    offset[1] = yO;
    offset[2] = zO;
    count[0] = xC;
    count[1] = yC;
    count[2] = zC;
    hsize_t mem_offset[3];
    mem_offset[0] = 0;
    mem_offset[1] = 0;
    mem_offset[2] = 0;

    try {
        dataspace.selectHyperslab(H5S_SELECT_SET, count, offset);

        H5::DataSpace memspace(3, count);
        memspace.selectHyperslab(H5S_SELECT_SET, count, mem_offset);

        try {
            data_out = new uint64_t[xC * yC * zC]();
        } catch (std::bad_alloc e) {
            throw std::runtime_error(std::string("There is not enough memory to allocate dataset (dataset size: " + std::to_string(xC * yC * zC) + " integers)").c_str());
        }

        int t4 = clock();
        //std::cout << "memory before dataset.read    " << ShowMemoryUsageInMB() << " MB" << std::endl;
        dataset.read(data_out, typeI, memspace, dataspace);
        //std::cout << "memory after dataset.read     " << ShowMemoryUsageInMB() << " MB" << std::endl;
        int t5 = clock();
        std::cout << "time: " << (t5-t4) / (CLOCKS_PER_SEC / 1000) << " ms; \t" << " offset: " << offset[0] << " x " << offset[1] << " x " << offset[2] << "; count: " << count[0] << " x " << count[1] << " x " << count[2] << std::endl;
        //if (minVF != FNULL && maxVF != FNULL)
        HDF5Dataset::getMinAndMaxValueI(data_out, xC * yC * zC, minVI, maxVI);
    } catch(H5::DataSpaceIException error) {
        error.printError();
        throw std::runtime_error(error.getCDetailMsg());
    } catch(H5::DataSetIException error) {
        error.printError();
        throw std::runtime_error(error.getCDetailMsg());
    }

    return data_out;
}

void HDF5File::HDF5Dataset::checkOffsetAndCountParams(hsize_t xO, hsize_t yO, hsize_t zO, hsize_t xC, hsize_t yC, hsize_t zC)
{
    if (rank != 3) throw std::runtime_error("Wrong rank - dataset is not 3D matrix");
    //if (xO < 0) throw std::runtime_error("Wrong offset - x offset is less than 0");
    //if (yO < 0) throw std::runtime_error("Wrong offset - y offset is less than 0");
    //if (zO < 0) throw std::runtime_error("Wrong offset - z offset is less than 0");
    if (xO >= dims[0]) throw std::runtime_error("Wrong offset - too big x offset");
    if (yO >= dims[1]) throw std::runtime_error("Wrong offset - too big y offset");
    if (zO >= dims[2]) throw std::runtime_error("Wrong offset - too big z offset");
    if (xC <= 0) throw std::runtime_error("Wrong count - too small x count");
    if (yC <= 0) throw std::runtime_error("Wrong count - too small y count");
    if (zC <= 0) throw std::runtime_error("Wrong count - too small z count");
    if (xO + xC > dims[0]) throw std::runtime_error("Wrong count - sum of x offset and x count is too big");
    if (yO + yC > dims[1]) throw std::runtime_error("Wrong count - sum of y offset and y count is too big");
    if (zO + zC > dims[2]) throw std::runtime_error("Wrong count - sum of z offset and z count is too big");
}

void HDF5File::HDF5Dataset::findGlobalMinAndMaxValueF()
{
    HDF5Dataset::readFullDatasetF();
    minVF = maxVF = dataF[0];
    for (hsize_t x = 0; x < size; x++) {
        float value = dataF[x];
        //std::cout << value << std::endl;
        if (value < minVF) minVF = value;
        if (value > maxVF) maxVF = value;
    }
    issetGlobalMinAndMaxValue = true;
}

void HDF5File::HDF5Dataset::findGlobalMinAndMaxValueI()
{
    HDF5Dataset::readFullDatasetI();
    minVI = maxVI = dataI[0];
    for (hsize_t x = 0; x < size; x++) {
        uint64_t value = dataI[x];
        //std::cout << value << std::endl;
        if (value < minVI) minVI = value;
        if (value > maxVI) maxVI = value;
    }
    issetGlobalMinAndMaxValue = true;
}

HDF5File::HDF5Object::HDF5Attribute::HDF5Attribute(H5::Attribute attribute)
{
    type = attribute.getDataType();
    size = attribute.getInMemDataSize();

    try {
        name = attribute.getName();
    } catch(H5::AttributeIException error) {
        error.printError();
        throw std::runtime_error(error.getCDetailMsg());
    }
    space = attribute.getSpace();
    buffer = malloc(size);
    attribute.read(type, buffer);
}

HDF5File::HDF5Object::HDF5Attribute::~HDF5Attribute()
{
    free(buffer);
}

H5::DataType HDF5File::HDF5Object::HDF5Attribute::getDataType()
{
    return type;
}

hsize_t HDF5File::HDF5Object::HDF5Attribute::getSize()
{
    return size;
}

H5std_string HDF5File::HDF5Object::HDF5Attribute::getName()
{
    return name;
}

H5::DataSpace HDF5File::HDF5Object::HDF5Attribute::getSpace()
{
    return space;
}

void *HDF5File::HDF5Object::HDF5Attribute::getData()
{
    return buffer;
}


int HDF5File::HDF5Object::getNumAttrs()
{
    return object->getNumAttrs();
}

HDF5File::HDF5Object::HDF5Attribute HDF5File::HDF5Object::getAttribute(H5std_string name)
{
    H5::Attribute attr;
    try {
        std::cout << "Getting attribute \"" << name << "\"";
        attr = object->openAttribute(name);
        std::cout << " ... OK" << std::endl;
    } catch(H5::AttributeIException error) {
        std::cout << " ... error" << std::endl;
        error.printError();
        throw std::runtime_error(error.getCDetailMsg());
    }
    return HDF5Attribute(attr);
}

HDF5File::HDF5Object::HDF5Attribute HDF5File::HDF5Object::getAttribute(const unsigned int idx)
{
    H5::Attribute attr;
    try {
        std::cout << "Getting attribute " << idx;
        attr = object->openAttribute(idx);
        std::cout << " ... OK" << std::endl;
    } catch(H5::AttributeIException error) {
        std::cout << " ... error" << std::endl;
        error.printError();
        throw std::runtime_error(error.getCDetailMsg());
    }
    return HDF5Attribute(attr);
}

void HDF5File::HDF5Object::removeAttribute(const unsigned int idx)
{
    try {
        std::cout << "Removing attribute " << idx;
        object->removeAttr(object->openAttribute(idx).getName());
        std::cout << " ... OK" << std::endl;
    } catch(H5::AttributeIException error) {
        std::cout << " ... error" << std::endl;
    }
}

void HDF5File::HDF5Object::removeAttribute(const H5std_string name)
{
    try {
        std::cout << "Removing attribute \"" << name << "\"";
        object->removeAttr(name);
        std::cout << " ... OK" << std::endl;
    } catch(H5::AttributeIException error) {
        std::cout << " ... error" << std::endl;
    }
}



void HDF5File::HDF5Object::setAttribute(HDF5Attribute attribute)
{
    try {
        std::cout << "Creating attribute \"" << attribute.getName() << "\"";
        object->createAttribute(attribute.getName(), attribute.getDataType(), attribute.getSpace()).write(attribute.getDataType() , attribute.getData());
        std::cout << " ... OK" << std::endl;
    } catch(H5::AttributeIException error) {
        std::cout << " ... error" << std::endl;
        error.printError();
        throw std::runtime_error(error.getCDetailMsg());
    }
}


void HDF5File::HDF5Object::setAttribute(const H5std_string name, int value)
{
    try {
        object->removeAttr(name);
    } catch(H5::AttributeIException error) {

    }

    try {
        std::cout << "Creating int attribute \"" << name << " = " << value << "\"";
        H5::IntType type(H5::PredType::NATIVE_INT);
        H5::DataSpace att_space(H5S_SCALAR);
        H5::Attribute att = object->createAttribute(name, type, att_space);
        att.write(type, &value);
        att.close();
        std::cout << " ... OK" << std::endl;
    } catch(H5::AttributeIException error) {
        std::cout << " ... error" << std::endl;
        error.printError();
        throw std::runtime_error(error.getCDetailMsg());
    }
}

void HDF5File::HDF5Object::setAttribute(const H5std_string name, uint64_t value)
{
    try {
        object->removeAttr(name);
    } catch(H5::AttributeIException error) {

    }

    try {
        std::cout << "Creating uint64_t attribute \"" << name << " = " << value << "\"";
        H5::IntType type(H5::PredType::NATIVE_UINT64);
        H5::DataSpace att_space(H5S_SCALAR);
        H5::Attribute att = object->createAttribute(name, type, att_space);
        att.write(type, &value);
        att.close();
        std::cout << " ... OK" << std::endl;
    } catch(H5::AttributeIException error) {
        std::cout << " ... error" << std::endl;
        error.printError();
        throw std::runtime_error(error.getCDetailMsg());
    }
}

void HDF5File::HDF5Object::setAttribute(const H5std_string name, double value)
{
    try {
        object->removeAttr(name);
    } catch(H5::AttributeIException error) {

    }

    try {
        std::cout << "Creating double attribute \"" << name << " = " << value << "\"";
        H5::IntType type(H5::PredType::NATIVE_DOUBLE);
        H5::DataSpace att_space(H5S_SCALAR);
        H5::Attribute att = object->createAttribute(name, type, att_space);
        att.write(type, &value);
        att.close();
        std::cout << " ... OK" << std::endl;
    } catch(H5::AttributeIException error) {
        std::cout << " ... error" << std::endl;
        error.printError();
        throw std::runtime_error(error.getCDetailMsg());
    }
}

void HDF5File::HDF5Object::setAttribute(const H5std_string name, float value)
{
    try {
        object->removeAttr(name);
    } catch(H5::AttributeIException error) {

    }

    try {
        std::cout << "Creating float attribute \"" << name << " = " << value << "\"";
        H5::IntType type(H5::PredType::NATIVE_FLOAT);
        H5::DataSpace att_space(H5S_SCALAR);
        H5::Attribute att = object->createAttribute(name, type, att_space);
        att.write(type, &value);
        att.close();
        std::cout << " ... OK" << std::endl;
    } catch(H5::AttributeIException error) {
        std::cout << " ... error" << std::endl;
        error.printError();
        throw std::runtime_error(error.getCDetailMsg());
    }
}

void HDF5File::HDF5Object::setAttribute(const H5std_string name, const H5std_string value)
{
    try {
        object->removeAttr(name);
    } catch(H5::AttributeIException error) {

    }

    try {
        std::cout << "Creating string attribute \"" << name << " = " << value << "\"";
        H5::StrType type(0, H5T_VARIABLE);
        H5::DataSpace att_space(H5S_SCALAR);
        H5::Attribute att = object->createAttribute(name, type, att_space);
        const char *str = value.c_str();
        att.write(type, &str);
        att.close();
        std::cout << " ... OK" << std::endl;
    } catch(H5::AttributeIException error) {
        std::cout << " ... error" << std::endl;
        error.printError();
        throw std::runtime_error(error.getCDetailMsg());
    }
}

float HDF5File::HDF5Object::readAttributeF(const H5std_string name)
{
    float value;
    try {
        std::cout << "Reading float attribute \"" << name << "\"";
        H5::FloatType type(H5::PredType::NATIVE_FLOAT);
        object->openAttribute(name).read(type, &value);
        std::cout << " ... OK" << std::endl;
    } catch(H5::AttributeIException error) {
        std::cout << " ... error" << std::endl;
        error.printError();
        throw std::runtime_error(error.getCDetailMsg());
    }
    return value;
}

uint64_t HDF5File::HDF5Object::readAttributeI(const H5std_string name)
{
    uint64_t value;
    try {
        std::cout << "Reading uint64_t attribute \"" << name << "\"";
        H5::IntType type(H5::PredType::NATIVE_UINT64);
        object->openAttribute(name).read(type, &value);
        std::cout << " ... OK" << std::endl;
    } catch(H5::AttributeIException error) {
        std::cout << " ... error" << std::endl;
        error.printError();
        throw std::runtime_error(error.getCDetailMsg());
    }
    return value;
}

H5std_string HDF5File::HDF5Object::readAttributeS(const H5std_string name)
{
    char *value;
    H5std_string retValue;
    try {
        std::cout << "Reading string attribute \"" << name << "\"";
        H5::StrType type = object->openAttribute(name).getStrType();
        if (type.isVariableStr()) {
            object->openAttribute(name).read(type, &value);
            retValue = H5std_string(value);
        } else {
            value = new char[type.getSize()]();
            object->openAttribute(name).read(type, value);
            retValue = H5std_string(value);
            delete [] value;
        }
        std::cout << " ... OK" << std::endl;
    } catch(H5::DataTypeIException error) {
        std::cout << " ... error" << std::endl;
        error.printError();
        throw std::runtime_error(error.getCDetailMsg());
    } catch(H5::AttributeIException error) {
        std::cout << " ... error" << std::endl;
        error.printError();
        throw std::runtime_error(error.getCDetailMsg());
    }
    return retValue;
}

void HDF5File::HDF5Dataset::findAndSetGlobalMinAndMaxValue(bool reset)
{
    if (type_class == H5T_FLOAT) {
        if (reset) {
            HDF5Dataset::findGlobalMinAndMaxValueF();
            HDF5Dataset::setAttribute("min", minVF);
            HDF5Dataset::setAttribute("max", maxVF);
        } else {
            try {
                H5::FloatType type(H5::PredType::NATIVE_FLOAT);
                dataset.openAttribute("min").read(type, &minVF);
                dataset.openAttribute("max").read(type, &maxVF);
                issetGlobalMinAndMaxValue = true;
            } catch(H5::AttributeIException error) {
                HDF5Dataset::findGlobalMinAndMaxValueF();
                HDF5Dataset::setAttribute("min", minVF);
                HDF5Dataset::setAttribute("max", maxVF);
            }
        }
    } else {
        if (reset) {
            HDF5Dataset::findGlobalMinAndMaxValueI();
            HDF5Dataset::setAttribute("min", minVI);
            HDF5Dataset::setAttribute("max", maxVI);
        } else {
            try {
                H5::IntType type(H5::PredType::NATIVE_UINT64);
                dataset.openAttribute("min").read(type, &minVI);
                dataset.openAttribute("max").read(type, &maxVI);
                issetGlobalMinAndMaxValue = true;
            } catch(H5::AttributeIException error) {
                HDF5Dataset::findGlobalMinAndMaxValueI();
                HDF5Dataset::setAttribute("min", minVI);
                HDF5Dataset::setAttribute("max", maxVI);
            }
        }
    }
}

void HDF5File::HDF5Dataset::getMinAndMaxValueF(float *data, hsize_t size, float &minVF, float &maxVF)
{
    maxVF = minVF = data[0];
    for (hsize_t i = 0; i < size; i++) {
        float value = data[i];
        if (value < minVF) minVF = value;
        if (value > maxVF) maxVF = value;
    }
}

/**
 * @brief HDF5File::HDF5Dataset::getMinAndMaxValueI
 * @param data
 * @param size
 * @param minVI
 * @param maxVI
 */
void HDF5File::HDF5Dataset::getMinAndMaxValueI(uint64_t *data, hsize_t size, uint64_t &minVI, uint64_t &maxVI)
{
    maxVI = minVI = data[0];
    for (hsize_t i = 0; i < size; i++) {
        uint64_t value = data[i];
        if (value < minVI) minVI = value;
        if (value > maxVI) maxVI = value;
    }
}
