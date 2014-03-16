#include "HDF5File.h"
#include "HDF5Dataset.h"
#include "HDF5Group.h"

#include <math.h>

const H5std_string HDF5File::NT("Nt");
const H5std_string HDF5File::NX("Nx");
const H5std_string HDF5File::NY("Ny");
const H5std_string HDF5File::NZ("Nz");
hsize_t HDF5File::ZERO_CHUNK[3];

HDF5File::HDF5File(std::string _filename, unsigned int flag)
{
    filename = _filename;
    // Try block to detect exceptions raised by any of the calls inside it
    try {
        H5::Exception::dontPrint();
        H5::FileAccPropList access_plist = H5::FileAccPropList::DEFAULT;
        H5::FileCreatPropList create_plist = H5::FileCreatPropList::DEFAULT;
        //list.setSieveBufSize(1048576);

        if (flag == HDF5File::OPEN) {
            std::cout << "Opening file \"" << filename << "\"";
            file = H5::H5File(filename, H5F_ACC_RDWR, create_plist, access_plist);
            std::cout << " ... OK " << std::endl;

            std::cout << "SieveBufSize: " << file.getAccessPlist().getSieveBufSize() << std::endl;
            int mdc_nelmts;
            size_t rdcc_nelmts;
            size_t rdcc_nbytes;
            double rdcc_w0;
            file.getAccessPlist().getCache(mdc_nelmts, rdcc_nelmts, rdcc_nbytes, rdcc_w0);
            std::cout << "mdc_nelmts: " << mdc_nelmts << std::endl;
            std::cout << "rdcc_nelmts: " << rdcc_nelmts << std::endl;
            std::cout << "rdcc_nbytes: " << rdcc_nbytes << std::endl;
            std::cout << "rdcc_w0: " << rdcc_w0 << std::endl;

            insertDataset(HDF5File::NT);
            insertDataset(HDF5File::NX);
            insertDataset(HDF5File::NY);
            insertDataset(HDF5File::NZ);

            //Set dimensions
            uint64_t *data = NULL;
            data = openDataset(HDF5File::NT)->readFullDatasetI();
            nT = data[0];
            data = openDataset(HDF5File::NX)->readFullDatasetI();
            nX = data[0];
            data = openDataset(HDF5File::NY)->readFullDatasetI();
            nY = data[0];
            data = openDataset(HDF5File::NZ)->readFullDatasetI();
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
    for (std::map<const H5std_string, HDF5Group *>::iterator it = groups.begin(); it != groups.end(); ++it) {
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
        datasets.insert(std::pair<const H5std_string, HDF5Dataset *>(datasetName, hDF5Dataset));
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
        HDF5Group *hDF5Group = new HDF5Group(file.openGroup(groupName), groupName);
        std::cout << " ... OK" << std::endl;
        groups.insert(std::pair<const H5std_string, HDF5Group *>(groupName, hDF5Group));
    } catch(H5::FileIException error) {
        std::cout << " ... error" << std::endl;
        //error.printError();
        throw std::runtime_error(std::string("Group \"" + groupName + "\" does not exist").c_str());
    }
}

void HDF5File::createDatasetI(const H5std_string datasetName, hsize_t rank, hsize_t *size, hsize_t *chunk_size, bool rewrite)
{
    try {
        H5::DataSpace dataspace((int) rank, size);
        H5::DataType datatype(H5::PredType::NATIVE_UINT64);

        H5::DSetCreatPropList list = H5::DSetCreatPropList::DEFAULT;
        if (chunk_size != HDF5File::ZERO_CHUNK)
            list.setChunk((int) rank, chunk_size);
        else
            list.setLayout(H5D_CONTIGUOUS);

        std::cout << "Creating dataset \"" << datasetName << "\"";
        if (rewrite) {
            try {
                file.unlink(datasetName);
                std::cout << " ... rewrite";
            } catch(H5::FileIException error) {
            }
        }
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

void HDF5File::createDatasetF(const H5std_string datasetName, hsize_t rank, hsize_t *size, hsize_t *chunk_size, bool rewrite)
{
    try {
        H5::DataSpace dataspace((int) rank, size);
        H5::DataType datatype(H5::PredType::NATIVE_FLOAT);

        H5::DSetCreatPropList list = H5::DSetCreatPropList::DEFAULT;
        if (chunk_size != HDF5File::ZERO_CHUNK)
            list.setChunk((int) rank, chunk_size);
        else
            list.setLayout(H5D_CONTIGUOUS);

        std::cout << "Creating dataset \"" << datasetName << "\"";
        if (rewrite) {
            try {
                file.unlink(datasetName);
                std::cout << " ... rewrite";
                closeDataset(datasetName);
            } catch(H5::FileIException error) {
            }
        }
        file.createDataSet(datasetName, datatype, dataspace, list);
        std::cout << " ... OK" << std::endl;
    }
    // catch failure caused by the H5File operations
    catch (H5::FileIException error) {
        std::cout << " ... error" << std::endl;
        error.printError();
        throw std::runtime_error(error.getCDetailMsg());
    } catch(H5::GroupIException error) {
        std::cout << " ... error" << std::endl;
        error.printError();
        throw std::runtime_error(error.getCDetailMsg());
    }
}

void HDF5File::createGroup(const H5std_string name, bool rewrite)
{
    try {
        std::cout << "Creating group \"" << name << "\"";
        if (rewrite) {
            try {
                file.unlink(name);
                std::cout << " ... rewrite";
                closeGroup(name);
            } catch(H5::FileIException error) {
            }
        }
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
        HDF5Dataset *dataset = datasets.find(datasetName)->second;
        delete dataset;
        datasets.erase(datasets.find(datasetName));
    }
}

HDF5File::HDF5Group *HDF5File::openGroup(const H5std_string groupName)
{
    if (groups.find(groupName) == groups.end()) {
        HDF5File::insertGroup(groupName);
        return HDF5File::openGroup(groupName);
    } else
        return groups.find(groupName)->second;
}

HDF5File::HDF5Group *HDF5File::openGroup(hsize_t idx)
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

    if (groups.find(name) == groups.end()) {
        HDF5File::insertGroup(name);
        return HDF5File::openGroup(name);
    } else
        return groups.find(name)->second;
}

void HDF5File::closeGroup(const H5std_string groupName)
{
    if (groups.find(groupName) != groups.end()){
        HDF5Group *group = groups.find(groupName)->second;
        delete group;
        groups.erase(groups.find(groupName));
    }
}

/*void HDF5File::unlinkLocation(const H5std_string name)
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
}*/

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

void HDF5File::convertlinearTo3D(hsize_t index, hsize_t &z, hsize_t &y, hsize_t &x)
{
    if (index > nX * nY * nZ) throw std::runtime_error("Wrong index - too big index");
    if (index == 0) throw std::runtime_error("Wrong index - too small index");

    z = (hsize_t) ceil((double) index / (nX * nY)) - 1;
    y = (hsize_t) fmod((double) index - 1, (nX * nY)) / nX;
    x = (hsize_t) fmod(fmod((double) index - 1, (nX * nY)), nX);
}

void HDF5File::convert3DToLinear(hsize_t z, hsize_t y, hsize_t x, hsize_t &index)
{
    if (x >= nX) throw std::runtime_error("Wrong x - too big x");
    if (y >= nY) throw std::runtime_error("Wrong y - too big y");
    if (z >= nZ) throw std::runtime_error("Wrong z - too big z");

    index = x + 1 + nX * (y) + (z) * nX * nY;

}
