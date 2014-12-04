/*
 * @file        HDF5Object.cpp
 * @author      Petr Kleparnik, VUT FIT Brno, xklepa01@stud.fit.vutbr.cz
 * @version     0.0
 * @date        30 July 2014
 *
 * @brief       The implementation file containing HDF5Object class definition.
 *              This class is superclass of HDF5Group and HDF5Dataset and contains especially operations with attributes.
 *
 * @section     Licence
 * This file is part of hdf5file library for k-Wave h5 processing
 * for preprocessing HDF5 data created by the k-Wave toolbox - http://www.k-wave.org.
 * Copyright © 2014, Petr Kleparnik, VUT FIT Brno.
 * hdf5file library is free software.
 */

#include "HDF5Object.h"
#include "HDF5Attribute.h"

/**
 * @brief HDF5File::HDF5Object::HDF5Object
 * @param object
 */
HDF5File::HDF5Object::HDF5Object(const hid_t object)
{
    this->object = object;
}

/**
 * @brief HDF5File::HDF5Object::~HDF5Object
 */
HDF5File::HDF5Object::~HDF5Object()
{

}

/**
 * @brief HDF5File::HDF5Object::getNumAttrs Get number of attributes in object
 * @return number of attributes
 */
int HDF5File::HDF5Object::getNumAttrs()
{
    H5O_info_t object_info;
    herr_t err = H5Oget_info(object, &object_info);
    if (err < 0){
        throw std::runtime_error("H5Oget_info error");
        //MPI::COMM_WORLD.Abort(1);
    }
    return object_info.num_attrs;
}

/**
 * @brief HDF5File::HDF5Object::hasAttribute
 * @param name name of attribute
 * @return true/false
 * @throw std::runtime_error
 */
bool HDF5File::HDF5Object::hasAttribute(const std::string name)
{
    return H5Aexists(object, name.c_str());
}

/**
 * @brief HDF5File::HDF5Object::getAttribute Get attribute by name
 * @param name name of attribute
 * @return attribute (HDF5Attribute)
 * @throw std::runtime_error
 */
HDF5File::HDF5Object::HDF5Attribute *HDF5File::HDF5Object::getAttribute(const std::string name)
{
    hid_t attr = H5Aopen_name(object, name.c_str());
    if (attr < 0){
        throw std::runtime_error("H5Aopen_name error");
        //MPI::COMM_WORLD.Abort(1);
    }
    HDF5Attribute *at = new HDF5Attribute(attr);
    herr_t err = H5Aclose(attr);
    if (err < 0){
        throw std::runtime_error("H5Aclose error");
        //MPI::COMM_WORLD.Abort(1);
    }
    return at;
}

/**
 * @brief HDF5File::HDF5Object::getAttribute Get attribute by idx
 * @param idx idx of attribute
 * @return attribute (HDF5Attribute)
 * @throw std::runtime_error
 */
HDF5File::HDF5Object::HDF5Attribute *HDF5File::HDF5Object::getAttribute(const unsigned int idx)
{
    hid_t attr = H5Aopen_idx(object, idx);
    if (attr < 0){
        throw std::runtime_error("H5Aopen_idx error");
        //MPI::COMM_WORLD.Abort(1);
    }
    HDF5Attribute *at = new HDF5Attribute(attr);
    herr_t err = H5Aclose(attr);
    if (err < 0){
        throw std::runtime_error("H5Aclose error");
        //MPI::COMM_WORLD.Abort(1);
    }
    return at;
}

/**
 * @brief HDF5File::HDF5Object::removeAttribute Remove attribute by idx
 * @param idx
 * @throw std::runtime_error
 */
void HDF5File::HDF5Object::removeAttribute(const unsigned int idx)
{
    HDF5File::HDF5Object::removeAttribute(this->getAttribute(idx)->getName());
}

/**
 * @brief HDF5File::HDF5Object::removeAttribute Remove attribute by name
 * @param name
 * @throw std::runtime_error
 */
void HDF5File::HDF5Object::removeAttribute(const std::string name)
{
        std::cout << "Removing attribute \"" << name << "\"";
        if (HDF5File::HDF5Object::hasAttribute(name.c_str())) {
            herr_t err = H5Adelete(object, name.c_str());
            if (err < 0){
                std::cout << " ... error" << std::endl;
                throw std::runtime_error("H5Adelete error");
                //MPI::COMM_WORLD.Abort(1);
            }
        } else {
            std::cout << " ... attribute not exists";
        }
        std::cout << " ... OK" << std::endl;
}

/**
 * @brief HDF5File::HDF5Object::creatingAttributeMessage
 * @param name
 * @param type
 * @param value
 */
void HDF5File::HDF5Object::creatingAttributeMessage(const std::string name, const hid_t type, const void *value)
{
    if (H5Tequal(type, H5T_C_S1))
        std::cout << "Creating attribute (\"H5T_C_S1\") \"" << name << " = " << *(char *) value << "\"";
    else if (H5Tequal(type, H5T_NATIVE_INT))
        std::cout << "Creating attribute (\"H5T_NATIVE_INT\") \"" << name << " = " << *(int *) value << "\"";
    else if (H5Tequal(type, H5T_NATIVE_UINT64))
        std::cout << "Creating attribute (\"H5T_NATIVE_UINT64\") \"" << name << " = " << *(uint64_t *) value << "\"";
    else if (H5Tequal(type, H5T_NATIVE_DOUBLE))
        std::cout << "Creating attribute (\"H5T_NATIVE_DOUBLE\") \"" << name << " = " << *(double *) value << "\"";
    else if (H5Tequal(type, H5T_NATIVE_FLOAT))
        std::cout << "Creating attribute (\"H5T_NATIVE_FLOAT\") \"" << name << " = " << *(float *) value << "\"";
    else
        std::cout << "Creating attribute \"" << name << " = " << value << "\"";
}

/**
 * @brief HDF5File::HDF5Object::setAttribute Set attribute by HDF5Attribute
 * @param attribute
 * @throw std::runtime_error
 */
void HDF5File::HDF5Object::setAttribute(HDF5Attribute *attribute)
{
    HDF5File::HDF5Object::removeAttribute(attribute->getName());
    HDF5File::HDF5Object::creatingAttributeMessage(attribute->getName(), attribute->getDataType(), attribute->getData());
    // Copy attribute
    hid_t attr = H5Acreate(object, attribute->getName().c_str(), attribute->getDataType(), attribute->getSpace(), H5P_DEFAULT, H5P_DEFAULT);
    if (attr < 0){
        std::cout << " ... error" << std::endl;
        throw std::runtime_error("H5Acreate error");
        //MPI::COMM_WORLD.Abort(1);
    }
    herr_t err = H5Awrite(attr, attribute->getDataType() , attribute->getData());
    if (err < 0){
        std::cout << " ... error" << std::endl;
        throw std::runtime_error("H5Awrite error");
        //MPI::COMM_WORLD.Abort(1);
    }
    err = H5Aclose(attr);
    if (err < 0){
        std::cout << " ... error" << std::endl;
        throw std::runtime_error("H5Aclose error");
        //MPI::COMM_WORLD.Abort(1);
    }
    std::cout << " ... OK" << std::endl;
}

/**
 * @brief HDF5File::HDF5Object::setAttribute Set attribute by name type and value
 * @param name
 * @param type
 * @param value
 * @throw std::runtime_error
 */
void HDF5File::HDF5Object::setAttribute(const std::string name, const hid_t type, const void *value)
{
    HDF5File::HDF5Object::removeAttribute(name);
    HDF5File::HDF5Object::creatingAttributeMessage(name, type, value);
    hid_t datatype = H5Tcopy(type);
    if (datatype < 0){
        std::cout << " ... error" << std::endl;
        throw std::runtime_error("H5Tcopy error");
        //MPI::COMM_WORLD.Abort(1);
    }
    if (type == H5T_C_S1) {
        herr_t err = H5Tset_size(datatype, H5T_VARIABLE);
        if (err < 0){
            std::cout << " ... error" << std::endl;
            throw std::runtime_error("H5Tset_size error");
            //MPI::COMM_WORLD.Abort(1);
        }
    }
    hid_t dataspace = H5Screate(H5S_SCALAR);
    if (dataspace < 0){
        std::cout << " ... error" << std::endl;
        throw std::runtime_error("H5Screate error");
        //MPI::COMM_WORLD.Abort(1);
    }
    hid_t attr = H5Acreate(object, name.c_str(), datatype, dataspace, H5P_DEFAULT, H5P_DEFAULT);
    if (attr < 0){
        std::cout << " ... error" << std::endl;
        throw std::runtime_error("H5Acreate error");
        //MPI::COMM_WORLD.Abort(1);
    }
    herr_t err = H5Awrite(attr, datatype, value);
    if (err < 0){
        std::cout << " ... error" << std::endl;
        throw std::runtime_error("H5Awrite error");
        //MPI::COMM_WORLD.Abort(1);
    }
    err = H5Aclose(attr);
    if (err < 0){
        std::cout << " ... error" << std::endl;
        throw std::runtime_error("H5Aclose error");
        //MPI::COMM_WORLD.Abort(1);
    }
    std::cout << " ... OK" << std::endl;
}

/**
 * @brief HDF5File::HDF5Object::setAttribute Set attribute by name and integer value
 * @param name
 * @param value
 * @throw std::runtime_error
 */
void HDF5File::HDF5Object::setAttribute(const std::string name, const int value)
{
    HDF5File::HDF5Object::setAttribute(name, H5T_NATIVE_INT, &value);
}

/**
 * @brief HDF5File::HDF5Object::setAttribute Set attribute by name and uint64_t value
 * @param name
 * @param value
 * @throw std::runtime_error
 */
void HDF5File::HDF5Object::setAttribute(const std::string name, const uint64_t value)
{
    HDF5File::HDF5Object::setAttribute(name, H5T_NATIVE_UINT64, &value);
}

/**
 * @brief HDF5File::HDF5Object::setAttribute Set attribute by name and double value
 * @param name
 * @param value
 * @throw std::runtime_error
 */
void HDF5File::HDF5Object::setAttribute(const std::string name, const double value)
{
    HDF5File::HDF5Object::setAttribute(name, H5T_NATIVE_DOUBLE, &value);
}

/**
 * @brief HDF5File::HDF5Object::setAttribute Set attribute by name and float value
 * @param name
 * @param value
 * @throw std::runtime_error
 */
void HDF5File::HDF5Object::setAttribute(const std::string name, const float value)
{
    HDF5File::HDF5Object::setAttribute(name, H5T_NATIVE_FLOAT, &value);
}

/**
 * @brief HDF5File::HDF5Object::setAttribute Set attribute by name and string value
 * @param name
 * @param value
 * @throw std::runtime_error
 */
void HDF5File::HDF5Object::setAttribute(const std::string name, const std::string value)
{
    const char *str = value.c_str();
    HDF5File::HDF5Object::setAttribute(name, H5T_C_S1, str);
}


/**
 * @brief HDF5File::HDF5Object::readAttributeF Read float attribute
 * @param name
 * @return float value
 */
void HDF5File::HDF5Object::readAttribute(const std::string name, void *value)
{
    std::cout << "Reading attribute \"" << name << "\"";
    hid_t attr = H5Aopen_name(object, name.c_str());
    if (attr < 0){
        std::cout << " ... error" << std::endl;
        throw std::runtime_error("H5Aopen_name error");
        //MPI::COMM_WORLD.Abort(1);
    }
    hid_t datatype = H5Aget_type(attr);
    if (datatype < 0){
        std::cout << " ... error" << std::endl;
        throw std::runtime_error("H5Tcopy error");
        //MPI::COMM_WORLD.Abort(1);
    }
    if (H5Tis_variable_str(datatype)) {
        herr_t err = H5Aread(attr, datatype, value);
        if (err < 0){
            std::cout << " ... error" << std::endl;
            throw std::runtime_error("H5Aread error");
            //MPI::COMM_WORLD.Abort(1);
        }
    } else {
        char *valueC = new char[H5Tget_size(datatype)]();
        herr_t err = H5Aread(attr, datatype, value);
        if (err < 0){
            std::cout << " ... error" << std::endl;
            throw std::runtime_error("H5Aread error");
            //MPI::COMM_WORLD.Abort(1);
        }
        value = valueC;
    }
    herr_t err = H5Aclose(attr);
    if (err < 0){
        std::cout << " ... error" << std::endl;
        throw std::runtime_error("H5Aclose error");
        //MPI::COMM_WORLD.Abort(1);
    }
    std::cout << " ... OK" << std::endl;
}

/**
 * @brief HDF5File::HDF5Object::readAttributeF Read float attribute
 * @param name
 * @return float value
 */
float HDF5File::HDF5Object::readAttributeF(const std::string name)
{
    float value;
    HDF5File::HDF5Object::readAttribute(name, &value);
    return value;
}

/**
 * @brief HDF5File::HDF5Object::readAttributeI Read uint64_t attribute
 * @param name
 * @return uint64_t value
 */
uint64_t HDF5File::HDF5Object::readAttributeI(const std::string name)
{
    uint64_t value;
    HDF5File::HDF5Object::readAttribute(name, &value);
    return value;
}

/**
 * @brief HDF5File::HDF5Object::readAttributeS Read string attribute
 * @param name
 * @return string value
 */
std::string HDF5File::HDF5Object::readAttributeS(const std::string name)
{
    char *value = NULL;
    std::string retValue;
    HDF5File::HDF5Object::readAttribute(name, value);
    retValue = std::string(value);
    delete [] value;
    return retValue;
}

