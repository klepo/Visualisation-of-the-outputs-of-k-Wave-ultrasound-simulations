#include "HDF5Dataset.h"

#include <time.h>   // clock()

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

    chunk_dims = NULL;
    chunk_dims = getChunkDims();

    dataspace.getSimpleExtentDims(dims, NULL);

    size = 1;
    for (hsize_t i = 0; i < rank; i++)
        size *= dims[i];

    maxVF = 0;
    minVF = 0;

    maxVI = 0;
    minVI = 0;

    dataI = NULL;
    dataF = NULL;

    blockInitialized = false;
    lastBlock = false;
    blockSize = 0;

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

H5std_string HDF5File::HDF5Dataset::getName()
{
    return name;
}

hsize_t HDF5File::HDF5Dataset::getId()
{
    return dataset.getId();
}

hsize_t HDF5File::HDF5Dataset::getRank()
{
    return rank;
}

hsize_t *HDF5File::HDF5Dataset::getDims()
{
    return dims;
}

hsize_t *HDF5File::HDF5Dataset::getChunkDims()
{
    H5::DSetCreatPropList cparms = dataset.getCreatePlist();
    delete [] chunk_dims;
    chunk_dims = NULL;
    chunk_dims = new hsize_t[rank]();
    if (H5D_CHUNKED == cparms.getLayout()) {
        cparms.getChunk((int) rank, chunk_dims);
        return chunk_dims;
    } else
        return NULL;
}

hsize_t HDF5File::HDF5Dataset::getSize()
{
    return size;
}

H5T_class_t HDF5File::HDF5Dataset::getDataType()
{
    return type_class;
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
        H5::DataSpace memspace((int) rank, dims);

        if (size > HDF5File::SIZE_OF_DATA_PART)
            throw std::runtime_error(std::string("Can not read the entire dataset, size: " + std::to_string(size) + " floats (max size: " + std::to_string(HDF5File::SIZE_OF_DATA_PART) + " floats)"));

        try {
            delete [] dataF;
            dataF = NULL;
            dataF = new float[size](); // TODO kontrola dostupné paměti
        } catch (std::bad_alloc e) {
            throw std::runtime_error(std::string("There is not enough memory to allocate dataset (dataset size: " + std::to_string(size) + " floats)").c_str());
        }

        try {

            int t4 = clock();
            dataset.read(dataF, typeF, memspace, dataspace);
            int t5 = clock();
            std::cout << name << " read time: " << (t5-t4) / (CLOCKS_PER_SEC / 1000) << " ms;" << std::endl;

        } catch(H5::DataSetIException error) {
            error.printError();
            throw std::runtime_error(error.getCDetailMsg());
        }
    } else
        throw std::runtime_error("Wrong data type of dataset (not float)");
    return dataF;
}

uint64_t *HDF5File::HDF5Dataset::readFullDatasetI()
{
    if (type_class == H5T_INTEGER) {
        H5::DataSpace memspace((int) rank, dims);

        if (size > HDF5File::SIZE_OF_DATA_PART)
            throw std::runtime_error(std::string("Can not read the entire dataset, size: " + std::to_string(size) + " unsigned 64-bit integers (max size: " + std::to_string(HDF5File::SIZE_OF_DATA_PART) + " unsigned 64-bit integers)"));

        try {
            delete [] dataI;
            dataI = NULL;
            dataI = new uint64_t[size]();
        } catch (std::bad_alloc e) {
            throw std::runtime_error(std::string("There is not enough memory to allocate dataset (dataset size: " + std::to_string(size) + " unsigned 64-bit integers)").c_str());
        }
        try {

            int t4 = clock();
            dataset.read(dataI, typeI, memspace, dataspace);
            int t5 = clock();
            std::cout << name << " read time: " << (t5-t4) / (CLOCKS_PER_SEC / 1000) << " ms;" << std::endl;

        } catch(H5::DataSetIException error) {
            error.printError();
            throw std::runtime_error(error.getCDetailMsg());
        }
    } else
        throw std::runtime_error("Wrong data type of dataset (not integer)");
    return dataI;
}

float *HDF5File::HDF5Dataset::read3DDataset(hsize_t zO, hsize_t yO, hsize_t xO, hsize_t zC, hsize_t yC, hsize_t xC, float &minVF, float &maxVF)
{
    if (type_class != H5T_FLOAT) throw std::runtime_error("Wrong data type of dataset (not float)");
    HDF5Dataset::checkOffsetAndCountParams(zO, yO, xO, zC, yC, xC);

    hsize_t offset[3];   // hyperslab offset in the file
    hsize_t count[3];    // size of the hyperslab in the file
    offset[0] = zO;
    offset[1] = yO;
    offset[2] = xO;
    count[0] = zC;
    count[1] = yC;
    count[2] = xC;
    hsize_t mem_offset[3];
    mem_offset[0] = 0;
    mem_offset[1] = 0;
    mem_offset[2] = 0;

    float *dataF = NULL;

    if (xC * yC * zC > HDF5File::SIZE_OF_DATA_PART)
        throw std::runtime_error(std::string("Can not read the entire dataset, size: " + std::to_string(xC * yC * zC) + " floats (max size: " + std::to_string(HDF5File::SIZE_OF_DATA_PART) + " floats)"));

    try {
        dataspace.selectHyperslab(H5S_SELECT_SET, count, offset);

        H5::DataSpace memspace(3, count);
        memspace.selectHyperslab(H5S_SELECT_SET, count, mem_offset);

        try {
            //delete [] dataF;
            //dataF = NULL;
            dataF = new float[xC * yC * zC]();
        } catch (std::bad_alloc e) {
            throw std::runtime_error(std::string("There is not enough memory to allocate dataset (dataset size: " + std::to_string(xC * yC * zC) + " floats)").c_str());
        }

        int t4 = clock();
        dataset.read(dataF, typeF, memspace, dataspace);
        int t5 = clock();
        std::cout << name << " read time: " << (t5-t4) / (CLOCKS_PER_SEC / 1000) << " ms; \t" << " offset: " << offset[0] << " x " << offset[1] << " x " << offset[2] << ";\tcount: " << count[0] << " x " << count[1] << " x " << count[2] << std::endl;

        HDF5Dataset::getMinAndMaxValue(dataF, xC * yC * zC, minVF, maxVF);
    } catch(H5::DataSpaceIException error) {
        error.printError();
        throw std::runtime_error(error.getCDetailMsg());
    } catch(H5::DataSetIException error) {
        error.printError();
        throw std::runtime_error(error.getCDetailMsg());
    }

    return dataF;
}

uint64_t *HDF5File::HDF5Dataset::read3DDataset(hsize_t zO, hsize_t yO, hsize_t xO, hsize_t zC, hsize_t yC, hsize_t xC, uint64_t &minVI, uint64_t &maxVI)
{
    if (type_class != H5T_INTEGER) throw std::runtime_error("Wrong data type of dataset (not integer)");
    HDF5Dataset::checkOffsetAndCountParams(zO, yO, xO, zC, yC, xC);

    hsize_t offset[3];   // hyperslab offset in the file
    hsize_t count[3];    // size of the hyperslab in the file
    offset[0] = zO;
    offset[1] = yO;
    offset[2] = xO;
    count[0] = zC;
    count[1] = yC;
    count[2] = xC;
    hsize_t mem_offset[3];
    mem_offset[0] = 0;
    mem_offset[1] = 0;
    mem_offset[2] = 0;

    uint64_t *dataI = NULL;

    if (xC * yC * zC > HDF5File::SIZE_OF_DATA_PART)
        throw std::runtime_error(std::string("Can not read dataset, size: " + std::to_string(xC * yC * zC) + " unsigned 64-bit integers (max size: " + std::to_string(HDF5File::SIZE_OF_DATA_PART) + " unsigned 64-bit integers)"));

    try {
        dataspace.selectHyperslab(H5S_SELECT_SET, count, offset);

        H5::DataSpace memspace(3, count);
        memspace.selectHyperslab(H5S_SELECT_SET, count, mem_offset);

        try {
            //delete [] dataI;
            //dataI = NULL;
            dataI = new uint64_t[xC * yC * zC]();
        } catch (std::bad_alloc e) {
            throw std::runtime_error(std::string("There is not enough memory to allocate dataset (dataset size: " + std::to_string(xC * yC * zC) + " unsigned 64-bit integers)").c_str());
        }

        int t4 = clock();
        dataset.read(dataI, typeI, memspace, dataspace);
        int t5 = clock();
        std::cout << name << " read time: " << (t5-t4) / (CLOCKS_PER_SEC / 1000) << " ms; \t" << " offset: " << offset[0] << " x " << offset[1] << " x " << offset[2] << ";\tcount: " << count[0] << " x " << count[1] << " x " << count[2] << std::endl;

        HDF5Dataset::getMinAndMaxValue(dataI, xC * yC * zC, minVI, maxVI);
    } catch(H5::DataSpaceIException error) {
        error.printError();
        throw std::runtime_error(error.getCDetailMsg());
    } catch(H5::DataSetIException error) {
        error.printError();
        throw std::runtime_error(error.getCDetailMsg());
    }

    return dataI;
}

void HDF5File::HDF5Dataset::write3DDataset(hsize_t zO, hsize_t yO, hsize_t xO, hsize_t zC, hsize_t yC, hsize_t xC, float *data, bool log)
{
    if (type_class != H5T_FLOAT) throw std::runtime_error("Wrong data type of dataset (not float)");
    HDF5Dataset::checkOffsetAndCountParams(zO, yO, xO, zC, yC, xC);

    hsize_t offset[3];   // hyperslab offset in the file
    hsize_t count[3];    // size of the hyperslab in the file
    offset[0] = zO;
    offset[1] = yO;
    offset[2] = xO;
    count[0] = zC;
    count[1] = yC;
    count[2] = xC;
    hsize_t mem_offset[3];
    mem_offset[0] = 0;
    mem_offset[1] = 0;
    mem_offset[2] = 0;

    try {
        dataspace.selectHyperslab(H5S_SELECT_SET, count, offset);

        H5::DataSpace memspace(3, count);
        memspace.selectHyperslab(H5S_SELECT_SET, count, mem_offset);

        int t4, t5;
        if (log)
            t4 = clock();
        dataset.write(data, typeF, memspace, dataspace);
        if (log)
            t5 = clock();
        if (log)
            std::cout << name << " write time: " << (t5-t4) / (CLOCKS_PER_SEC / 1000) << " ms; \t" << " offset: " << offset[0] << " x " << offset[1] << " x " << offset[2] << ";\tcount: " << count[0] << " x " << count[1] << " x " << count[2] << std::endl;

    } catch(H5::DataSpaceIException error) {
        error.printError();
        throw std::runtime_error(error.getCDetailMsg());
    } catch(H5::DataSetIException error) {
        error.printError();
        throw std::runtime_error(error.getCDetailMsg());
    }
}

void HDF5File::HDF5Dataset::write3DDataset(hsize_t zO, hsize_t yO, hsize_t xO, hsize_t zC, hsize_t yC, hsize_t xC, uint64_t *data, bool log)
{
    if (type_class != H5T_INTEGER) throw std::runtime_error("Wrong data type of dataset (not integer)");
    HDF5Dataset::checkOffsetAndCountParams(zO, yO, xO, zC, yC, xC);

    hsize_t offset[3];   // hyperslab offset in the file
    hsize_t count[3];    // size of the hyperslab in the file
    offset[0] = zO;
    offset[1] = yO;
    offset[2] = xO;
    count[0] = zC;
    count[1] = yC;
    count[2] = xC;
    hsize_t mem_offset[3];
    mem_offset[0] = 0;
    mem_offset[1] = 0;
    mem_offset[2] = 0;

    try {
        dataspace.selectHyperslab(H5S_SELECT_SET, count, offset);

        H5::DataSpace memspace(3, count);
        memspace.selectHyperslab(H5S_SELECT_SET, count, mem_offset);

        int t4, t5;
        if (log)
            t4 = clock();
        dataset.write(data, typeI, memspace, dataspace);
        if (log)
            t5 = clock();
        if (log)
            std::cout << name << " write time: " << (t5-t4) / (CLOCKS_PER_SEC / 1000) << " ms; \t" << " offset: " << offset[0] << " x " << offset[1] << " x " << offset[2] << ";\tcount: " << count[0] << " x " << count[1] << " x " << count[2] << std::endl;

    } catch(H5::DataSpaceIException error) {
        error.printError();
        throw std::runtime_error(error.getCDetailMsg());
    } catch(H5::DataSetIException error) {
        error.printError();
        throw std::runtime_error(error.getCDetailMsg());
    }
}

void HDF5File::HDF5Dataset::checkOffsetAndCountParams(hsize_t zO, hsize_t yO, hsize_t xO, hsize_t zC, hsize_t yC, hsize_t xC)
{
    if (rank != 3) throw std::runtime_error("Wrong rank - dataset is not 3D matrix");
    if (zO >= dims[0]) throw std::runtime_error("Wrong offset - too big z offset");
    if (yO >= dims[1]) throw std::runtime_error("Wrong offset - too big y offset");
    if (xO >= dims[2]) throw std::runtime_error("Wrong offset - too big x offset");
    if (zC <= 0) throw std::runtime_error("Wrong count - too small z count");
    if (yC <= 0) throw std::runtime_error("Wrong count - too small y count");
    if (xC <= 0) throw std::runtime_error("Wrong count - too small x count");
    if (zO + zC > dims[0]) throw std::runtime_error("Wrong count - sum of z offset and z count is too big");
    if (yO + yC > dims[1]) throw std::runtime_error("Wrong count - sum of y offset and y count is too big");
    if (xO + xC > dims[2]) throw std::runtime_error("Wrong count - sum of x offset and x count is too big");
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

bool HDF5File::HDF5Dataset::isLastBlock()
{
    return lastBlock;
}

hsize_t HDF5File::HDF5Dataset::getBlockSize()
{
    return blockSize;
}

void HDF5File::HDF5Dataset::initBlockReading(hsize_t maxSize)
{
    // Compute maximal block size to read
    z = maxSize / (dims[2] * dims[1]);
    y = (maxSize % (dims[2] * dims[1])) / dims[2];
    x = (maxSize % (dims[2] * dims[1])) % dims[2];

    z = std::min(z, dims[0]);

    xO = 0;
    yO = 0;
    zO = 0;

    if (z > 0) { // Minimal size is slab xy
        zC = z;
        yC = dims[1];
        xC = dims[2];
        blockSize = z * dims[2] * dims[1];
    } else if (y > 0) { // Minimal size is part of slab xy
        zC = 1;
        yC = y;
        xC = dims[2];
        blockSize = y * dims[2];
    } else { // Minimal size is smaller than x size
        zC = 1;
        yC = 1;
        xC = blockSize = x;
    }

    blockInitialized = true;
    lastBlock = false;
}

void HDF5File::HDF5Dataset::recomputeBlock()
{
    if (z > 0) { // Minimal size is slab xy
        zO += zC;
        if (zO >= dims[0]) {
            lastBlock = true;
            return;
        }
        if (zO + zC > dims[0] - 1) {
            zC = dims[0] - zO;
        }
    } else if (y > 0) { // Minimal size is part of slab xy
        yO += yC;
        if (yO >= dims[1]) {
            zO += 1;
            if (zO >= dims[0]) {
                lastBlock = true;
                return;
            }
            yO = 0;
            yC = y;
            return;
        }
        if (yO + yC > dims[1] - 1) {
            yC = dims[1] - yO;
        }
    } else { // Minimal size is smaller than x size
        xO += xC;
        if (xO >= dims[2]) {
            yO += 1;
            if (yO >= dims[1]) {
                zO += 1;
                if (zO >= dims[0]) {
                    lastBlock = true;
                    return;
                }
                yO = 0;
            }
            xO = 0;
            xC = x;
            return;
        }
        if (xO + xC > dims[2] - 1) {
            xC = dims[2] - xO;
        }
    }
}

float *HDF5File::HDF5Dataset::readBlock(hsize_t &zO_, hsize_t &yO_, hsize_t &xO_, hsize_t &zC_, hsize_t &yC_, hsize_t &xC_, float &minVFTmp, float &maxVFTmp)
{
    if (!blockInitialized) {
        initBlockReading();
    }

    float *data = read3DDataset(zO, yO, xO, zC, yC, xC, minVFTmp, maxVFTmp);

    zO_ = zO;
    yO_ = yO;
    xO_ = xO;
    zC_ = zC;
    yC_ = yC;
    xC_ = xC;

    recomputeBlock();

    return data;
}

uint64_t *HDF5File::HDF5Dataset::readBlock(hsize_t &zO_, hsize_t &yO_, hsize_t &xO_, hsize_t &zC_, hsize_t &yC_, hsize_t &xC_, uint64_t &minVITmp, uint64_t &maxVITmp)
{
    if (!blockInitialized) {
        initBlockReading();
    }

    uint64_t *data = read3DDataset(zO, yO, xO, zC, yC, xC, minVITmp, maxVITmp);

    zO_ = zO;
    yO_ = yO;
    xO_ = xO;
    zC_ = zC;
    yC_ = yC;
    xC_ = xC;

    recomputeBlock();

    return data;
}

void HDF5File::HDF5Dataset::findGlobalMinAndMaxValueF()
{
    float *data;

    hsize_t xO, yO, zO;
    hsize_t xC, yC, zC;

    float minVFTmp;
    float maxVFTmp;

    bool first = true;

    do {

        data = readBlock(zO, yO, xO, zC, yC, xC, minVFTmp, maxVFTmp);
        if (first)
            minVF = maxVF = data[0];
        first = false;

        if (minVFTmp < minVF) minVF = minVFTmp;
        if (maxVFTmp > maxVF) maxVF = maxVFTmp;

    } while (lastBlock == false);

    issetGlobalMinAndMaxValue = true;
}

void HDF5File::HDF5Dataset::findGlobalMinAndMaxValueI()
{
    uint64_t *data;

    hsize_t xO, yO, zO;
    hsize_t xC, yC, zC;

    uint64_t minVITmp;
    uint64_t maxVITmp;

    bool first = true;

    do {

        data = readBlock(zO, yO, xO, zC, yC, xC, minVITmp, maxVITmp);
        if (first)
            minVI = maxVI = data[0];
        first = false;

        if (minVITmp < minVI) minVI = minVITmp;
        if (maxVITmp > maxVI) maxVI = maxVITmp;

    } while (lastBlock == false);

    issetGlobalMinAndMaxValue = true;
}

void HDF5File::HDF5Dataset::getMinAndMaxValue(float *data, hsize_t size, float &minVF, float &maxVF)
{
    maxVF = minVF = data[0];
    for (hsize_t i = 0; i < size; i++) {
        float value = data[i];
        if (value < minVF) minVF = value;
        if (value > maxVF) maxVF = value;
    }
}

void HDF5File::HDF5Dataset::getMinAndMaxValue(uint64_t *data, hsize_t size, uint64_t &minVI, uint64_t &maxVI)
{
    maxVI = minVI = data[0];
    for (hsize_t i = 0; i < size; i++) {
        uint64_t value = data[i];
        if (value < minVI) minVI = value;
        if (value > maxVI) maxVI = value;
    }
}
