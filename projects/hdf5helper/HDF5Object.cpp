/**
 * @file        HDF5Object.cpp
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        30 July      2014 (created) \n
 *              27 February  2017 (updated)
 *
 * @brief       The implementation file containing HDF5Object class definition.
 *              This class is superclass of HDF5Group and HDF5Dataset and contains especially operations with attributes.
 *
 * @license     This file is part of the hdf5helper library for k-Wave h5 processing
 *              for preprocessing the HDF5 data created by the k-Wave toolbox - http://www.k-wave.org.
 *              The hdf5helper library is free software.
 *
 * @copyright   Copyright © 2017, Petr Kleparnik, VUT FIT Brno. All Rights Reserved.
 *
 */

#include "HDF5Object.h"
#include "HDF5Attribute.h"

namespace HDF5Helper {

/**
 * @brief HDF5Object::HDF5Object
 * @param object
 */
HDF5Object::HDF5Object(const hid_t object, std::string name)
{
    this->object = object;
    this->name = name;
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
 * @throw std::runtime_error
 */
hsize_t HDF5Object::getNumAttrs() const
{
    H5O_info_t object_info;
    herr_t err = H5Oget_info(object, &object_info);
    if (err < 0) {
        throw std::runtime_error("H5Oget_info error");
    }
    return object_info.num_attrs;
}

/**
 * @brief HDF5Object::getName
 * @return
 */
std::string HDF5Object::getName()
{
    return name;
}

/**
 * @brief HDF5Object::hasAttribute
 * @param name name of attribute
 * @return true/false
 */
bool HDF5Object::hasAttribute(const std::string name) const
{
    return H5Aexists(object, name.c_str()) != 0;
}

/**
 * @brief HDF5Object::getAttribute Get attribute by name
 * @param name name of attribute
 * @return attribute (HDF5Attribute)
 */
HDF5Attribute *HDF5Object::getAttribute(const std::string name) const
{
    return new HDF5Attribute(object, name);
}

/**
 * @brief HDF5Object::getAttribute Get attribute by idx
 * @param idx idx of attribute
 * @return attribute (HDF5Attribute)
 */
HDF5Attribute *HDF5Object::getAttribute(const hsize_t idx) const
{
    return new HDF5Attribute(object, idx);
}

/**
 * @brief HDF5Object::removeAttribute Remove attribute by idx
 * @param idx
 */
void HDF5Object::removeAttribute(const unsigned int idx, bool log)
{
    HDF5Object::removeAttribute(this->getAttribute(idx)->getName(), log);
}

/**
 * @brief HDF5Object::removeAttribute Remove attribute by name
 * @param name
 * @throw std::runtime_error
 */
void HDF5Object::removeAttribute(const std::string name, bool log)
{
    if (log)
        std::cout << "Removing attribute \"" << name << "\"";
    if (HDF5Object::hasAttribute(name.c_str())) {
        err = H5Adelete(object, name.c_str());
        if (err < 0) {
            if (log)
                std::cout << " ... error" << std::endl;
            throw std::runtime_error("H5Adelete error");
        }
    } else {
        if (log)
            std::cout << " ... attribute not exists";
    }
    if (log)
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
std::string HDF5Object::getStringValueByType(const hid_t type, const void *value) const
{
    if (H5Tget_class(type) == H5Tget_class(H5T_C_S1)) {
        if (H5Tis_variable_str(type)) {
            void *voidValue = const_cast<void *>(value);
            //return *(char **)(value);
            return std::string(*static_cast<char **>(voidValue));
        } else {
             return static_cast<const char *>(value);
        }
    } else if (H5Tequal(type, H5T_NATIVE_INT))
        return std::to_string(*static_cast<const int *>(value));
    else if (H5Tequal(type, H5T_NATIVE_UINT64))
        return std::to_string(*static_cast<const hsize_t *>(value));
    else if (H5Tequal(type, H5T_NATIVE_INT64))
        return std::to_string(*static_cast<const hssize_t *>(value));
    else if (H5Tequal(type, H5T_NATIVE_DOUBLE))
        return std::to_string(*static_cast<const double *>(value));
    else if (H5Tequal(type, H5T_NATIVE_FLOAT))
        return std::to_string(*static_cast<const float *>(value));
    else
        return static_cast<const char *>(value);
}

/**
 * @brief HDF5Object::getStringValueByType
 * @param type
 * @param value
 * @return
 */
std::string HDF5Object::getStringTypeByType(const hid_t type) const
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
    else if (H5Tequal(type, H5T_NATIVE_INT64))
        return "H5T_NATIVE_INT64";
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
 * @param log
 * @throw std::runtime_error
 */
void HDF5Object::createAttribute(const std::string name, const hid_t type, const hid_t space, const void *value, bool log)
{
    HDF5Object::removeAttribute(name, false);
    if (log)
        HDF5Object::creatingAttributeMessage(name, type, value);

    // Copy attribute
    hid_t attr = H5Acreate(object, name.c_str(), type, space, H5P_DEFAULT, H5P_DEFAULT);
    if (attr < 0) {
        if (log)
            std::cout << " ... error" << std::endl;
        throw std::runtime_error("H5Acreate error");
    }
    err = H5Awrite(attr, type, value);
    if (err < 0) {
        if (log)
            std::cout << " ... error" << std::endl;
        throw std::runtime_error("H5Awrite error");
    }
    err = H5Aclose(attr);
    if (err < 0) {
        if (log)
            std::cout << " ... error" << std::endl;
        throw std::runtime_error("H5Aclose error");
    }
    if (log)
        std::cout << " ... OK" << std::endl;
}

/**
 * @brief HDF5Object::setAttribute Set attribute by HDF5Attribute
 * @param attribute
 */
void HDF5Object::setAttribute(HDF5Attribute *attribute, bool log)
{
    createAttribute(attribute->getName(), attribute->getDatatype(), attribute->getDataspace(), attribute->getData(), log);
}

/**
 * @brief HDF5Object::setAttribute Set attribute by name type and value
 * @param name
 * @param type
 * @param value
 * @param log
 * @throw std::runtime_error
 */
void HDF5Object::setAttribute(const std::string name, const hid_t type, const void *value, bool log)
{
    hid_t datatype = H5Tcopy(type);
    if (datatype < 0) {
        std::cout << " ... error" << std::endl;
        throw std::runtime_error("H5Tcopy error");
    }
    if (type == H5T_C_S1) {
        err = H5Tset_size(datatype, size_t(-1));
        if (err < 0) {
            std::cout << " ... error" << std::endl;
            throw std::runtime_error("H5Tset_size error");
        }
        err = H5Tset_cset(datatype, H5T_CSET_UTF8);
        if (err < 0) {
            std::cout << " ... error" << std::endl;
            throw std::runtime_error("H5Tset_cset error");
        }
    }
    hid_t dataspace = H5Screate(H5S_SCALAR);
    if (dataspace < 0) {
        std::cout << " ... error" << std::endl;
        throw std::runtime_error("H5Screate error");
    }
    createAttribute(name, datatype, dataspace, value, log);
    err = H5Tclose(datatype);
    if (err < 0) {
        throw std::runtime_error("H5Tclose error");
    }
    err = H5Sclose(dataspace);
    if (err < 0) {
        throw std::runtime_error("H5Sclose error");
    }
}

/**
 * @brief HDF5Object::setAttribute Set attribute by name and integer value
 * @param name
 * @param value
 */
void HDF5Object::setAttribute(const std::string name, const int value, bool log)
{
    HDF5Object::setAttribute(name, H5T_NATIVE_INT, &value, log);
}

/**
 * @brief HDF5Object::setAttribute Set attribute by name and hsize_t value
 * @param name
 * @param value
 */
void HDF5Object::setAttribute(const std::string name, const hsize_t value, bool log)
{
    HDF5Object::setAttribute(name, H5T_NATIVE_UINT64, &value, log);
}

/**
 * @brief HDF5Object::setAttribute Set attribute by name and double value
 * @param name
 * @param value
 */
void HDF5Object::setAttribute(const std::string name, const double value, bool log)
{
    HDF5Object::setAttribute(name, H5T_NATIVE_DOUBLE, &value, log);
}

/**
 * @brief HDF5Object::setAttribute Set attribute by name and float value
 * @param name
 * @param value
 */
void HDF5Object::setAttribute(const std::string name, const float value, bool log)
{
    HDF5Object::setAttribute(name, H5T_NATIVE_FLOAT, &value, log);
}

/**
 * @brief HDF5Object::setAttribute Set attribute by name and string value
 * @param name
 * @param value
 */
void HDF5Object::setAttribute(const std::string name, const std::string value, bool log)
{
    const char *str = value.c_str();
    HDF5Object::setAttribute(name, H5T_C_S1, &str, log);
}

/**
 * @brief HDF5Object::readAttributeF Read float attribute
 * @param name
 * @return float value
 */
float HDF5Object::readAttributeF(const std::string name, bool log) const
{
    float value;
    if (log)
        std::cout << "Reading attribute \"" << name << "\" ";
    HDF5Attribute *attr = getAttribute(name);
    if (log)
        std::cout << "(" << getStringTypeByType(attr->getDatatype()) << ")";
    value = *static_cast<float *>(attr->getData());
    delete attr;
    if (log)
        std::cout << " = \"" << value << "\"";
    if (log)
        std::cout << " ... OK" << std::endl;
    return value;
}

/**
 * @brief HDF5Object::readAttributeI Read hsize_t attribute
 * @param name
 * @return hsize_t value
 */
hsize_t HDF5Object::readAttributeI(const std::string name, bool log) const
{
    hsize_t value;
    if (log)
        std::cout << "Reading attribute \"" << name << "\" ";
    HDF5Attribute *attr = getAttribute(name);
    if (log)
        std::cout << "(" << getStringTypeByType(attr->getDatatype()) << ")";
    value = *static_cast<hsize_t *>(attr->getData());
    delete attr;
    if (log)
        std::cout << " = \"" << value << "\"";
    if (log)
        std::cout << " ... OK" << std::endl;
    return value;
}

/**
 * @brief HDF5Object::readAttributeS Read string attribute
 * @param name
 * @return string value
 */
std::string HDF5Object::readAttributeS(const std::string name, bool log) const
{
    std::string value;
    if (log)
        std::cout << "Reading attribute \"" << name << "\" ";
    HDF5Attribute *attr = getAttribute(name);
    if (log)
        std::cout << "(" << getStringTypeByType(attr->getDatatype()) << ")";
    if (H5Tget_class(attr->getDatatype()) == H5Tget_class(H5T_C_S1) && H5Tis_variable_str(attr->getDatatype()))
        //value = std::string(*(char **) attr->getData());
        value = std::string(*static_cast<char **>(attr->getData()));
    else
        value = std::string(static_cast<char *>(attr->getData()));
    delete attr;
    if (log)
        std::cout << " = \"" << value << "\"";
    if (log)
        std::cout << " ... OK" << std::endl;
    return value;
}

/**
 * @brief HDF5Object::getFile
 * @return
 */
File *HDF5Object::getFile()
{
    return hDF5File;
}

/**
 * @brief HDF5Object::setDeleteLog
 * @param value
 */
void HDF5Object::setDeleteLog(bool value)
{
    deleteLog = value;
}

/**
 * @brief HDF5Helper::HDF5Object::operator std::string
 */
HDF5Helper::HDF5Object::operator std::string() const
{
    return this->name;
}
}
