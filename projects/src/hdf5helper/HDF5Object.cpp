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

namespace HDF5Helper {

/**
 * @brief HDF5Object::HDF5Object
 * @param object
 */
HDF5Object::HDF5Object(const hid_t object)
{
    this->object = object;
}

/**
 * @brief HDF5Object::~HDF5Object
 */
HDF5Object::~HDF5Object()
{

}

/**
 * @brief HDF5Object::getNumAttrs Get number of attributes in object
 * @return number of attributes
 */
unsigned int HDF5Object::getNumAttrs()
{
    H5O_info_t object_info;
    err = H5Oget_info(object, &object_info);
    if (err < 0){
        throw std::runtime_error("H5Oget_info error");
        //MPI::COMM_WORLD.Abort(1);
    }
    return (unsigned int) object_info.num_attrs;
}

/**
 * @brief HDF5Object::hasAttribute
 * @param name name of attribute
 * @return true/false
 * @throw std::runtime_error
 */
bool HDF5Object::hasAttribute(const std::string name)
{
    return H5Aexists(object, name.c_str()) != 0;
}

/**
 * @brief HDF5Object::getAttribute Get attribute by name
 * @param name name of attribute
 * @return attribute (HDF5Attribute)
 * @throw std::runtime_error
 */
HDF5Attribute *HDF5Object::getAttribute(const std::string name)
{
    HDF5Attribute *at = new HDF5Attribute(object, name);
    return at;
}

/**
 * @brief HDF5Object::getAttribute Get attribute by idx
 * @param idx idx of attribute
 * @return attribute (HDF5Attribute)
 * @throw std::runtime_error
 */
HDF5Attribute *HDF5Object::getAttribute(const unsigned int idx)
{
    HDF5Attribute *at = new HDF5Attribute(object, idx);
    return at;
}

/**
 * @brief HDF5Object::removeAttribute Remove attribute by idx
 * @param idx
 * @throw std::runtime_error
 */
void HDF5Object::removeAttribute(const unsigned int idx, bool flag)
{
    HDF5Object::removeAttribute(this->getAttribute(idx)->getName(), flag);
}

/**
 * @brief HDF5Object::removeAttribute Remove attribute by name
 * @param name
 * @throw std::runtime_error
 */
void HDF5Object::removeAttribute(const std::string name, bool flag)
{
    if (flag)
        std::cout << "Removing attribute \"" << name << "\"";
    if (HDF5Object::hasAttribute(name.c_str())) {
        err = H5Adelete(object, name.c_str());
        if (err < 0){
            if (flag)
                std::cout << " ... error" << std::endl;
            throw std::runtime_error("H5Adelete error");
            //MPI::COMM_WORLD.Abort(1);
        }
    } else {
        if (flag)
            std::cout << " ... attribute not exists";
    }
    if (flag)
        std::cout << " ... OK" << std::endl;
}

/**
 * @brief HDF5Object::creatingAttributeMessage
 * @param name
 * @param type
 * @param value
 */
void HDF5Object::creatingAttributeMessage(const std::string name, const hid_t type, const void *value)
{
    std::cout << "Creating attribute \"" << name << "\" (" << getStringTypeByType(type) << ") = \"" << getStringValueByType(type, value) << "\"";
}

/**
 * @brief HDF5Object::getStringValueByType
 * @param type
 * @param value
 * @return
 */
std::string HDF5Object::getStringValueByType(const hid_t type, const void *value)
{
    if (H5Tget_class(type) == H5Tget_class(H5T_C_S1)) {
        if (H5Tis_variable_str(type))
            return (*(char **) value);
        else
            return ((char *) value);
    } else if (H5Tequal(type, H5T_NATIVE_INT))
        return std::to_string(*(int *) value);
    else if (H5Tequal(type, H5T_NATIVE_UINT64))
        return std::to_string(*(hsize_t *) value);
    else if (H5Tequal(type, H5T_NATIVE_DOUBLE))
        return std::to_string(*(double *) value);
    else if (H5Tequal(type, H5T_NATIVE_FLOAT))
        return std::to_string(*(float *) value);
    else
        return ((char *) value);
}

/**
 * @brief HDF5Object::getStringValueByType
 * @param type
 * @param value
 * @return
 */
std::string HDF5Object::getStringTypeByType(const hid_t type)
{
    if (H5Tget_class(type) == H5Tget_class(H5T_C_S1)) {
        if (H5Tis_variable_str(type))
            return "H5T_C_S1, H5T_VARIABLE";
        else
            return "H5T_C_S1";
    } else if (H5Tequal(type, H5T_NATIVE_INT))
        return "H5T_NATIVE_INT";
    else if (H5Tequal(type, H5T_NATIVE_UINT64))
        return "H5T_NATIVE_UINT64";
    else if (H5Tequal(type, H5T_NATIVE_DOUBLE))
        return "H5T_NATIVE_DOUBLE";
    else if (H5Tequal(type, H5T_NATIVE_FLOAT))
        return "H5T_NATIVE_FLOAT";
    else
        return std::to_string(type);
}

/**
 * @brief HDF5Object::createAttribute
 * @param name
 * @param type
 * @param space
 * @param value
 */
void HDF5Object::createAttribute(const std::string name, const hid_t type, const hid_t space, const void *value, bool flag)
{
    HDF5Object::removeAttribute(name, false);
    if (flag)
        HDF5Object::creatingAttributeMessage(name, type, value);

    // Copy attribute
    hid_t attr = H5Acreate(object, name.c_str(), type, space, H5P_DEFAULT, H5P_DEFAULT);
    if (attr < 0){
        if (flag)
            std::cout << " ... error" << std::endl;
        throw std::runtime_error("H5Acreate error");
        //MPI::COMM_WORLD.Abort(1);
    }
    err = H5Awrite(attr, type, value);
    if (err < 0){
        if (flag)
            std::cout << " ... error" << std::endl;
        throw std::runtime_error("H5Awrite error");
        //MPI::COMM_WORLD.Abort(1);
    }
    err = H5Aclose(attr);
    if (err < 0){
        if (flag)
            std::cout << " ... error" << std::endl;
        throw std::runtime_error("H5Aclose error");
        //MPI::COMM_WORLD.Abort(1);
    }
    if (flag)
        std::cout << " ... OK" << std::endl;
}

/**
 * @brief HDF5Object::setAttribute Set attribute by HDF5Attribute
 * @param attribute
 * @throw std::runtime_error
 */
void HDF5Object::setAttribute(HDF5Attribute *attribute, bool flag)
{
    createAttribute(attribute->getName(), attribute->getDatatype(), attribute->getDataspace(), attribute->getData(), flag);
}

/**
 * @brief HDF5Object::setAttribute Set attribute by name type and value
 * @param name
 * @param type
 * @param value
 * @throw std::runtime_error
 */
void HDF5Object::setAttribute(const std::string name, const hid_t type, const void *value, bool flag)
{
    hid_t datatype = H5Tcopy(type);
    if (datatype < 0){
        std::cout << " ... error" << std::endl;
        throw std::runtime_error("H5Tcopy error");
        //MPI::COMM_WORLD.Abort(1);
    }
    if (type == H5T_C_S1) {
        err = H5Tset_size(datatype, H5T_VARIABLE);
        if (err < 0){
            std::cout << " ... error" << std::endl;
            throw std::runtime_error("H5Tset_size error");
            //MPI::COMM_WORLD.Abort(1);
        }
        err = H5Tset_cset(datatype, H5T_CSET_UTF8);
        if (err < 0){
            std::cout << " ... error" << std::endl;
            throw std::runtime_error("H5Tset_cset error");
            //MPI::COMM_WORLD.Abort(1);
        }
    }
    hid_t dataspace = H5Screate(H5S_SCALAR);
    if (dataspace < 0){
        std::cout << " ... error" << std::endl;
        throw std::runtime_error("H5Screate error");
        //MPI::COMM_WORLD.Abort(1);
    }
    createAttribute(name, datatype, dataspace, value, flag);
    H5Tclose(datatype);
    H5Sclose(dataspace);
}

/**
 * @brief HDF5Object::setAttribute Set attribute by name and integer value
 * @param name
 * @param value
 * @throw std::runtime_error
 */
void HDF5Object::setAttribute(const std::string name, const int value)
{
    HDF5Object::setAttribute(name, H5T_NATIVE_INT, &value);
}

/**
 * @brief HDF5Object::setAttribute Set attribute by name and hsize_t value
 * @param name
 * @param value
 * @throw std::runtime_error
 */
void HDF5Object::setAttribute(const std::string name, const hsize_t value)
{
    HDF5Object::setAttribute(name, H5T_NATIVE_UINT64, &value);
}

/**
 * @brief HDF5Object::setAttribute Set attribute by name and double value
 * @param name
 * @param value
 * @throw std::runtime_error
 */
void HDF5Object::setAttribute(const std::string name, const double value)
{
    HDF5Object::setAttribute(name, H5T_NATIVE_DOUBLE, &value);
}

/**
 * @brief HDF5Object::setAttribute Set attribute by name and float value
 * @param name
 * @param value
 * @throw std::runtime_error
 */
void HDF5Object::setAttribute(const std::string name, const float value)
{
    HDF5Object::setAttribute(name, H5T_NATIVE_FLOAT, &value);
}

/**
 * @brief HDF5Object::setAttribute Set attribute by name and string value
 * @param name
 * @param value
 * @throw std::runtime_error
 */
void HDF5Object::setAttribute(const std::string name, const std::string value)
{
    const char *str = value.c_str();
    HDF5Object::setAttribute(name, H5T_C_S1, &str);
}

/**
 * @brief HDF5Object::readAttributeF Read float attribute
 * @param name
 * @return float value
 */
float HDF5Object::readAttributeF(const std::string name)
{
    float value;
    std::cout << "Reading attribute \"" << name << "\" ";
    HDF5Attribute *attr = getAttribute(name);
    std::cout << "(" << getStringTypeByType(attr->getDatatype()) << ")";
    value = *(float *) attr->getData();
    delete attr;
    std::cout << " = \"" << value << "\"";
    std::cout << " ... OK" << std::endl;
    return value;
}

/**
 * @brief HDF5Object::readAttributeI Read hsize_t attribute
 * @param name
 * @return hsize_t value
 */
hsize_t HDF5Object::readAttributeI(const std::string name)
{
    hsize_t value;
    std::cout << "Reading attribute \"" << name << "\" ";
    HDF5Attribute *attr = getAttribute(name);
    std::cout << "(" << getStringTypeByType(attr->getDatatype()) << ")";
    value = *(hsize_t *) attr->getData();
    delete attr;
    std::cout << " = \"" << value << "\"";
    std::cout << " ... OK" << std::endl;
    return value;
}

/**
 * @brief HDF5Object::readAttributeS Read string attribute
 * @param name
 * @return string value
 */
std::string HDF5Object::readAttributeS(const std::string name)
{
    std::string value;
    std::cout << "Reading attribute \"" << name << "\" ";
    HDF5Attribute *attr = getAttribute(name);
    std::cout << "(" << getStringTypeByType(attr->getDatatype()) << ")";
    if (H5Tget_class(attr->getDatatype()) == H5Tget_class(H5T_C_S1) && H5Tis_variable_str(attr->getDatatype()))
        value = std::string(*(char **) attr->getData());
    else
        value = std::string((char *) attr->getData());
    delete attr;
    std::cout << " = \"" << value << "\"";
    std::cout << " ... OK" << std::endl;
    return value;
}

File *HDF5Object::getFile()
{
    return hDF5File;
}
}
