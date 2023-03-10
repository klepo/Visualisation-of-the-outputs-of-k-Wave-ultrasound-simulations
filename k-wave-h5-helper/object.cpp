/**
 * @file        object.cpp
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        30 July      2014 (created) <br>
 *              10 February  2023 (updated)
 *
 * @brief       The implementation file containing H5Helper::Object class definition.
 *
 * This is superclass of H5Helper::Group and H5Helper::Dataset and contains
 * especially operations with attributes.
 *
 * @license     This file is part of the k-wave-h5-helper library for processing the HDF5 data
 *              created by the k-Wave toolbox - http://www.k-wave.org. This file may be used,
 *              distributed and modified under the terms of the LGPL version 3 open source
 *              license. A copy of the LGPL license should have been received with this file.
 *              Otherwise, it can be found at: http://www.gnu.org/copyleft/lesser.html.
 *
 * @copyright   Copyright © 2019, Petr Kleparnik, VUT FIT Brno. All Rights Reserved.
 *
 */

#include "object.h"

namespace H5Helper
{

/**
 * @brief Creates Object with given object and name
 * @param[in] objectId Object id
 * @param[in] name Name of object
 * @param[in] file HDF5 File
 */
Object::Object(hid_t objectId, std::string name, File *file)
    : file(file)
    , objectId(objectId)
{
    this->name = "/" + trimSlashes(name);
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
 */
void Object::setAttribute(const Attribute *attribute)
{
    createAttribute(attribute->getName(), attribute->getDatatype(), attribute->getDataspace(), attribute->getData());
}

/**
 * @brief Sets attribute by given name to integer value
 * @param[in] name Name of attribute
 * @param[in] value Attribute value
 */
void Object::setAttribute(std::string name, int value)
{
    Object::setAttribute(name, H5T_NATIVE_INT, &value);
}

/**
 * @brief Sets attribute by given name to unsigned integer value
 * @param[in] name Name of attribute
 * @param[in] value Attribute value
 */
void Object::setAttribute(std::string name, unsigned int value)
{
    Object::setAttribute(name, H5T_NATIVE_UINT, &value);
}

/**
 * @brief Sets attribute by given name to 64-bit integer value
 * @param[in] name Name of attribute
 * @param[in] value Attribute value
 */
void Object::setAttribute(std::string name, hssize_t value)
{
    Object::setAttribute(name, H5T_NATIVE_INT64, &value);
}

/**
 * @brief Sets attribute by given name to 64-bit unsigned integer value
 * @param[in] name Name of attribute
 * @param[in] value Attribute value
 */
void Object::setAttribute(std::string name, hsize_t value)
{
    Object::setAttribute(name, H5T_NATIVE_UINT64, &value);
}

/**
 * @brief Sets attribute by given name to float value
 * @param[in] name Name of attribute
 * @param[in] value Attribute value
 */
void Object::setAttribute(std::string name, float value)
{
    Object::setAttribute(name, H5T_NATIVE_FLOAT, &value);
}

/**
 * @brief Sets attribute by given name to double value
 * @param[in] name Name of attribute
 * @param[in] value Attribute value
 */
void Object::setAttribute(std::string name, double value)
{
    Object::setAttribute(name, H5T_NATIVE_LDOUBLE, &value);
}

/**
 * @brief Sets attribute by given name to long double value
 * @param[in] name Name of attribute
 * @param[in] value Attribute value
 */
void Object::setAttribute(std::string name, long double value)
{
    Object::setAttribute(name, H5T_NATIVE_LDOUBLE, &value);
}

/**
 * @brief Sets attribute by given name to string value
 * @param[in] name Name of attribute
 * @param[in] value Attribute value
 */
void Object::setAttribute(std::string name, std::string value)
{
    const char *str = value.c_str();
    Object::setAttribute(name, H5T_C_S1, &str);
}

/**
 * @brief Returns float attribute value
 * @param[in] name Name of attribute
 * @return Float attribute value
 */
float Object::readAttributeF(std::string name) const
{
    float value;
    Helper::printDebugMsgStart("Reading attribute \"" + name + "\"");
    Attribute *attribute = getAttribute(name);
    // attribute->getStringDatatype();
    if (H5Tequal(attribute->getDatatype(), H5T_NATIVE_INT) || H5Tequal(attribute->getDatatype(), H5T_NATIVE_UINT)
        || H5Tequal(attribute->getDatatype(), H5T_NATIVE_INT64)
        || H5Tequal(attribute->getDatatype(), H5T_NATIVE_UINT64)) {
        ssize_t valueI = *static_cast<ssize_t *>(attribute->getData());
        value          = float(valueI);
    } else {
        value = *static_cast<float *>(attribute->getData());
    }
    delete attribute;
    attribute = nullptr;
    Helper::printDebugMsgEnd(std::to_string(value));
    return value;
}

/**
 * @brief Returns 64-bit unsigned integer attribute value
 * @param[in] name Name of attribute
 * @return Integer attribute value
 */
hsize_t Object::readAttributeI(std::string name) const
{
    hsize_t value;
    Helper::printDebugMsgStart("Reading attribute \"" + name + "\"");
    Attribute *attribute = getAttribute(name);
    // attribute->getStringDatatype();
    if (H5Tequal(attribute->getDatatype(), H5T_NATIVE_FLOAT)) {
        float valueF = *static_cast<float *>(attribute->getData());
        value        = hsize_t(valueF);
    } else if (H5Tequal(attribute->getDatatype(), H5T_NATIVE_DOUBLE)) {
        double valueD = *static_cast<double *>(attribute->getData());
        value         = hsize_t(valueD);
    } else if (H5Tequal(attribute->getDatatype(), H5T_NATIVE_LDOUBLE)) {
        long double valueLD = *static_cast<long double *>(attribute->getData());
        value               = hsize_t(valueLD);
    } else {
        value = *static_cast<hsize_t *>(attribute->getData());
    }
    delete attribute;
    attribute = nullptr;
    Helper::printDebugMsgEnd(std::to_string(value));
    return value;
}

/**
 * @brief Returns string attribute value
 * @param[in] name Name of attribute
 * @return String attribute value
 */
std::string Object::readAttributeS(std::string name) const
{
    std::string value;
    Helper::printDebugMsgStart("Reading attribute \"" + name + "\"");
    Attribute *attribute = getAttribute(name);
    // attribute->getStringDatatype();
    value = attribute->getStringValue();
    delete attribute;
    attribute = nullptr;
    Helper::printDebugMsgEnd("\"" + value + "\"");
    return value;
}

/**
 * @brief Returns Attribute object by given name
 * @param[in] name Name of attribute
 * @return Attribute object
 */
Attribute *Object::getAttribute(std::string name) const
{
    return new Attribute(objectId, name);
}

/**
 * @brief Returns Attribute object by given idx
 * @param[in] idx Index of attribute
 * @return Attribute object
 */
Attribute *Object::getAttribute(hsize_t idx) const
{
    return new Attribute(objectId, idx);
}

/**
 * @brief Removes Attribute object by given name
 * @param[in] name Name of attribute
 * @throw std::runtime_error
 */
void Object::removeAttribute(std::string name) const
{
    Helper::printDebugMsgStart("Removing attribute \"" + name + "\"");
    if (Object::hasAttribute(name.c_str())) {
        herr_t err = H5Adelete(objectId, name.c_str());
        if (err < 0) {
            Helper::printDebugMsgEnd("error");
            throw std::runtime_error("H5Adelete error");
        }
    } else {
        Helper::printDebugMsgEnd("attribute not exists");
    }
    Helper::printDebugMsgEnd("OK");
}

/**
 * @brief Removes Attribute object by given idx
 * @param[in] idx Index of attribute
 */
void Object::removeAttribute(const unsigned int idx) const
{
    Object::removeAttribute(this->getAttribute(idx)->getName());
}

/**
 * @brief Has object the attribute?
 * @param[in] name Name of attribute
 * @return True/False
 */
bool Object::hasAttribute(std::string name) const
{
    return H5Aexists(objectId, name.c_str()) != 0;
}

/**
 * @brief Renames attribute
 * @param[in] srcName Source attribute name
 * @param[in] dstName Destination attribute name
 */
void Object::renameAttribute(std::string srcName, std::string dstName) const
{
    file->renameAttribute(srcName, dstName, objectId);
}

/**
 * @brief Returns number of attributes in object
 * @return Number of attributes
 */
hsize_t Object::getNumAttrs() const
{
    hsize_t count = 0;
    hid_t attributeId;
    while (1) {
        attributeId = H5Aopen_by_idx(objectId, ".", H5_INDEX_NAME, H5_ITER_INC, count, 0, 0);
        if (attributeId >= 0) {
            count++;
            H5Aclose(attributeId);
        } else {
            H5Aclose(attributeId);
            break;
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
    std::string s         = name;
    std::string delimiter = "/";
    std::string token     = s.substr(s.rfind(delimiter) + 1);
    if (!token.empty())
        return token;
    else
        return name;
}

/**
 * @brief Returns the object group name
 * @return Object group name
 */
std::string Object::getGroupName() const
{
    std::string str = getName().substr(0, getName().find(getOnlyName()) - 1);
    return str.size() > 0 ? str : "/";
}

/**
 * @brief Returns the name of object with underscores
 * @return Name of object
 */
std::string Object::getSuffixName(std::string suffix) const
{
    return (getGroupName() == "/") ? getName() + suffix : getGroupName() + suffix + "/" + getOnlyName();
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
 * @brief Operator std::string
 */
Object::operator std::string() const
{
    return this->name;
}

/**
 * @brief Creates attribute by name, type, dataspace, and value
 * @param[in] name Name of attribute
 * @param[in] datatypeId Datatype of attribute
 * @param[in] dataspaceId Attribute dataspace
 * @param[in] value Attribute value
 * @throw std::runtime_error
 */
void Object::createAttribute(std::string name, hid_t datatypeId, hid_t dataspaceId, const void *value) const
{
    Object::removeAttribute(name);
    Object::creatingAttributeMessage(name, datatypeId, value);

    // Create attribute
    hid_t attributeId = H5Acreate(objectId, name.c_str(), datatypeId, dataspaceId, 0, 0);
    if (attributeId < 0) {
        Helper::printDebugMsgEnd("error");
        throw std::runtime_error("H5Acreate error");
    }
    herr_t err = H5Awrite(attributeId, datatypeId, value);
    if (err < 0) {
        Helper::printDebugMsgEnd("error");
        throw std::runtime_error("H5Awrite error");
    }
    err = H5Aclose(attributeId);
    if (err < 0) {
        Helper::printDebugMsgEnd("error");
        throw std::runtime_error("H5Aclose error");
    }
    Helper::printDebugMsgEnd("OK");
}

/**
 * @brief Sets attribute by name, type, and value
 * @param[in] name Name of attribute
 * @param[in] datatypeId Datatype of attribute
 * @param[in] value Attribute value
 * @throw std::runtime_error
 */
void Object::setAttribute(std::string name, hid_t datatypeId, const void *value) const
{
    hid_t datatypeIdTmp = H5Tcopy(datatypeId);
    if (datatypeIdTmp < 0) {
        Helper::printDebugMsgEnd("error");
        throw std::runtime_error("H5Tcopy error");
    }
    if (datatypeId == H5T_C_S1) {
        herr_t err = H5Tset_size(datatypeIdTmp, size_t(-1));
        if (err < 0) {
            Helper::printDebugMsgEnd("error");
            throw std::runtime_error("H5Tset_size error");
        }
        err = H5Tset_cset(datatypeIdTmp, H5T_CSET_UTF8);
        if (err < 0) {
            Helper::printDebugMsgEnd("error");
            throw std::runtime_error("H5Tset_cset error");
        }
    }
    hid_t dataspaceId = H5Screate(H5S_SCALAR);
    if (dataspaceId < 0) {
        Helper::printDebugMsgEnd("error");
        throw std::runtime_error("H5Screate error");
    }
    createAttribute(name, datatypeIdTmp, dataspaceId, value);
    herr_t err = H5Tclose(datatypeIdTmp);
    if (err < 0) {
        throw std::runtime_error("H5Tclose error");
    }
    err = H5Sclose(dataspaceId);
    if (err < 0) {
        throw std::runtime_error("H5Sclose error");
    }
}

/**
 * @brief Prints creating attribute message
 * @param[in] name Name of attribute
 * @param[in] datatypeId Type of attribute
 * @param[in] value Attribute value
 */
void Object::creatingAttributeMessage(std::string name, hid_t datatypeId, const void *value) const
{
    std::string valueStr = Attribute::getStringValue(datatypeId, value);
    if (H5Tget_class(datatypeId) == H5Tget_class(H5T_C_S1))
        valueStr = "\"" + valueStr + "\"";
    // std::string nameStr = "\"" + name + "\" (" + Attribute::getStringDatatype(datatypeId) + ") ";
    // std::cout << "Creating attribute " << std::left << std::setw(40) << nameStr << valueStr;
    Helper::printDebugMsgStart("Creating attribute \"" + name + "\"");
    Helper::printDebugMsgEnd(valueStr);
}
} // namespace H5Helper
