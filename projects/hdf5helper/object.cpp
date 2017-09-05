/**
 * @file        object.cpp
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        30 July      2014 (created) \n
 *              28 August    2017 (updated)
 *
 * @brief       The implementation file containing HDF5Helper::Object class definition.
 *
 * This is superclass of HDF5Helper::Group and HDF5Helper::Dataset and contains
 * especially operations with attributes.
 *
 * @license     This file is part of the hdf5helper library for processing the HDF5 data
 *              created by the k-Wave toolbox - http://www.k-wave.org. This file may be used,
 *              distributed and modified under the terms of the LGPL version 3 open source
 *              license. A copy of the LGPL license should have been recieved with this file.
 *              Otherwise, it can be found at: http://www.gnu.org/copyleft/lesser.html.
 *
 * @copyright   Copyright © 2017, Petr Kleparnik, VUT FIT Brno. All Rights Reserved.
 *
 */

#include <object.h>

namespace HDF5Helper {

/**
 * @brief Creates Object with given object and name
 * @param[in] object Object id
 * @param[in] name Name of object
 * @param[in] file HDF5 File
 */
Object::Object(const hid_t object, std::string name, File *file)
{
    this->object = object;
    this->name = name;
    this->file = file;
}

/**
 * @brief Destructor of Object object
 *
 * Does nothing.
 */
Object::~Object()
{

}

/**
 * @brief Sets attribute by attribute
 * @param[in] attribute Attribute
 * @param[in] log Logging flag (optional)
 */
void Object::setAttribute(Attribute *attribute, bool log)
{
    createAttribute(attribute->getName(), attribute->getDatatype(), attribute->getDataspace(), attribute->getData(), log);
}

/**
 * @brief Sets attribute by given name to integer value
 * @param[in] name Name of attribute
 * @param[in] value Attribute value
 * @param[in] log Logging flag (optional)
 */
void Object::setAttribute(const std::string name, const int value, bool log)
{
    Object::setAttribute(name, H5T_NATIVE_INT, &value, log);
}

/**
 * @brief Sets attribute by given name to 64-bit unsigned integer value
 * @param[in] name Name of attribute
 * @param[in] value Attribute value
 * @param[in] log Logging flag (optional)
 */
void Object::setAttribute(const std::string name, const hsize_t value, bool log)
{
    Object::setAttribute(name, H5T_NATIVE_UINT64, &value, log);
}

/**
 * @brief Sets attribute by given name to double value
 * @param[in] name Name of attribute
 * @param[in] value Attribute value
 * @param[in] log Logging flag (optional)
 */
void Object::setAttribute(const std::string name, const double value, bool log)
{
    Object::setAttribute(name, H5T_NATIVE_DOUBLE, &value, log);
}

/**
 * @brief Sets attribute by given name to float value
 * @param[in] name Name of attribute
 * @param[in] value Attribute value
 * @param[in] log Logging flag (optional)
 */
void Object::setAttribute(const std::string name, const float value, bool log)
{
    Object::setAttribute(name, H5T_NATIVE_FLOAT, &value, log);
}

/**
 * @brief Sets attribute by given name to string value
 * @param[in] name Name of attribute
 * @param[in] value Attribute value
 * @param[in] log Logging flag (optional)
 */
void Object::setAttribute(const std::string name, const std::string value, bool log)
{
    const char *str = value.c_str();
    Object::setAttribute(name, H5T_C_S1, &str, log);
}

/**
 * @brief Returns float attribute value
 * @param[in] name Name of attribute
 * @param[in] log Logging flag (optional)
 * @return Float attribute value
 */
float Object::readAttributeF(const std::string name, bool log) const
{
    float value;
    if (log)
        std::cout << "Reading attribute \"" << name << "\" ";
    Attribute *attr = getAttribute(name);
    if (log)
        std::cout << "(" << attr->getStringDatatype() << ")";
    value = *static_cast<float *>(attr->getData());
    delete attr;
    if (log)
        std::cout << " = \"" << value << "\"";
    if (log)
        std::cout << " ... OK" << std::endl;
    return value;
}

/**
 * @brief Returns 64-bit unsigned integer attribute value
 * @param[in] name Name of attribute
 * @param[in] log Logging flag (optional)
 * @return Integer attribute value
 */
hsize_t Object::readAttributeI(const std::string name, bool log) const
{
    hsize_t value;
    if (log)
        std::cout << "Reading attribute \"" << name << "\" ";
    Attribute *attr = getAttribute(name);
    if (log)
        std::cout << "(" << attr->getStringDatatype() << ")";
    value = *static_cast<hsize_t *>(attr->getData());
    delete attr;
    if (log)
        std::cout << " = \"" << value << "\"";
    if (log)
        std::cout << " ... OK" << std::endl;
    return value;
}

/**
 * @brief Returns string attribute value
 * @param[in] name Name of attribute
 * @param[in] log Logging flag (optional)
 * @return String attribute value
 */
std::string Object::readAttributeS(const std::string name, bool log) const
{
    std::string value;
    if (log)
        std::cout << "Reading attribute \"" << name << "\" ";
    Attribute *attr = getAttribute(name);
    if (log)
        std::cout << "(" << attr->getStringDatatype() << ")";
    value = attr->getStringValue();
    delete attr;
    if (log)
        std::cout << " = \"" << value << "\"";
    if (log)
        std::cout << " ... OK" << std::endl;
    return value;
}

/**
 * @brief Returns Attribute object by given name
 * @param[in] name Name of attribute
 * @return Attribute object
 */
Attribute *Object::getAttribute(const std::string name) const
{
    return new Attribute(object, name);
}

/**
 * @brief Returns Attribute object by given idx
 * @param[in] idx Index of attribute
 * @return Attribute object
 */
Attribute *Object::getAttribute(const hsize_t idx) const
{
    return new Attribute(object, idx);
}

/**
 * @brief Removes Attribute object by given name
 * @param[in] name Name of attribute
 * @param[in] log Logging flag (optional)
 * @throw std::runtime_error
 */
void Object::removeAttribute(const std::string name, bool log)
{
    if (log)
        std::cout << "Removing attribute \"" << name << "\"";
    if (Object::hasAttribute(name.c_str())) {
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
 * @brief Removes Attribute object by given idx
 * @param[in] idx Index of attribute
 * @param[in] log Logging flag (optional)
 */
void Object::removeAttribute(const unsigned int idx, bool log)
{
    Object::removeAttribute(this->getAttribute(idx)->getName(), log);
}

/**
 * @brief Has object the attribute?
 * @param[in] name Name of attribute
 * @return True/False
 */
bool Object::hasAttribute(const std::string name) const
{
    return H5Aexists(object, name.c_str()) != 0;
}

/**
 * @brief Returns number of attributes in object
 * @return Number of attributes
 */
hsize_t Object::getNumAttrs() const
{
    hsize_t count = 0;
    hid_t attribute;
    while (1) {
        attribute = H5Aopen_by_idx(object, ".", H5_INDEX_NAME, H5_ITER_INC, count, 0, 0);
        if (attribute >= 0) {
            count++;
            H5Aclose(attribute);
        } else {
            break;
            H5Aclose(attribute);
        }
    }
    return count;
}

/**
 * @brief Returns object name
 * @return Object name
 */
std::string Object::getName() const
{
    return name;
}

/**
 * @brief Returns the name of object without any path
 * @return Name of object
 */
std::string Object::getOnlyName() const
{
    std::string s = name;
    std::string delimiter = "/";
    std::string token = s.substr(s.rfind(delimiter) + 1);
    if (!token.empty())
        return token;
    else
        return name;
}

/**
 * @brief Returns file where object is opened
 * @return File
 */
File *Object::getFile() const
{
    return file;
}

/**
 * @brief Sets delete logging
 * @param[in] value True/False for delete logging
 */
void Object::setDeleteLog(bool value)
{
    deleteLog = value;
}

/**
 * @brief Operator std::string
 */
HDF5Helper::Object::operator std::string() const
{
    return this->name;
}

/**
 * @brief Creates attribute by name, type, dataspace, and value
 * @param[in] name Name of attribute
 * @param[in] datatype Datatype of attribute
 * @param[in] dataspace Attribute dataspace
 * @param[in] value Attribute value
 * @param[in] log Logging flag (optional)
 * @throw std::runtime_error
 */
void Object::createAttribute(const std::string name, const hid_t datatype, const hid_t dataspace, const void *value, bool log)
{
    Object::removeAttribute(name, false);
    if (log)
        Object::creatingAttributeMessage(name, datatype, value);

    // Create attribute
    hid_t attr = H5Acreate(object, name.c_str(), datatype, dataspace, H5P_DEFAULT, H5P_DEFAULT);
    if (attr < 0) {
        if (log)
            std::cout << " ... error" << std::endl;
        throw std::runtime_error("H5Acreate error");
    }
    err = H5Awrite(attr, datatype, value);
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
 * @brief Sets attribute by name, type, and value
 * @param[in] name Name of attribute
 * @param[in] datatype Datatype of attribute
 * @param[in] value Attribute value
 * @param[in] log Logging flag (optional)
 * @throw std::runtime_error
 */
void Object::setAttribute(const std::string name, const hid_t datatype, const void *value, bool log)
{
    hid_t datatypeTmp = H5Tcopy(datatype);
    if (datatypeTmp < 0) {
        std::cout << " ... error" << std::endl;
        throw std::runtime_error("H5Tcopy error");
    }
    if (datatype == H5T_C_S1) {
        err = H5Tset_size(datatypeTmp, size_t(-1));
        if (err < 0) {
            std::cout << " ... error" << std::endl;
            throw std::runtime_error("H5Tset_size error");
        }
        err = H5Tset_cset(datatypeTmp, H5T_CSET_UTF8);
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
    createAttribute(name, datatypeTmp, dataspace, value, log);
    err = H5Tclose(datatypeTmp);
    if (err < 0) {
        throw std::runtime_error("H5Tclose error");
    }
    err = H5Sclose(dataspace);
    if (err < 0) {
        throw std::runtime_error("H5Sclose error");
    }
}

/**
 * @brief Prints creating attribute message
 * @param[in] name Name of attribute
 * @param[in] type Type of attribute
 * @param[in] value Attribute value
 */
void Object::creatingAttributeMessage(const std::string name, const hid_t type, const void *value)
{
    std::cout << "Creating attribute \"" << name << "\" (" << Attribute::getStringDatatype(type) << ") = \"" << Attribute::getStringValue(type, value) << "\"";
}
}
