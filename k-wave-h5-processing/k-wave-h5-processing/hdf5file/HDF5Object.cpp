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
HDF5File::HDF5Object::HDF5Object(H5::H5Object *object)
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
    //mutex.lock();
    int num = (int) object->getNumAttrs();
    //mutex.unlock();
    return num;
}

/**
 * @brief HDF5File::HDF5Object::hasAttribute
 * @param name name of attribute
 * @return true/false
 * @throw std::runtime_error
 */
bool HDF5File::HDF5Object::hasAttribute(H5std_string name)
{
    //return ((H5::H5Location *) object)->attrExists(name);
    try {
        object->openAttribute(name);
        return true;
    } catch(H5::Exception) {
        return false;
    }
}

/**
 * @brief HDF5File::HDF5Object::getAttribute Get attribute by name
 * @param name name of attribute
 * @return attribute (HDF5Attribute)
 * @throw std::runtime_error
 */
HDF5File::HDF5Object::HDF5Attribute *HDF5File::HDF5Object::getAttribute(H5std_string name)
{
    H5::Attribute attr;
    try {
        //mutex.lock();
        std::cout << "Getting attribute \"" << name << "\"";
        attr = object->openAttribute(name);
        std::cout << " ... OK" << std::endl;
        //mutex.unlock();
    } catch(H5::AttributeIException error) {
        std::cout << " ... error" << std::endl;
        error.printError();
        //mutex.unlock();
        throw std::runtime_error(error.getCDetailMsg());
    }
    HDF5Attribute *at = new HDF5Attribute(attr);
    attr.close();
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
    H5::Attribute attr;
    try {
        //mutex.lock();
        std::cout << "Getting attribute " << idx;
        attr = object->openAttribute(idx);
        std::cout << " \"" << attr.getName() << "\"";
        std::cout << " ... OK" << std::endl;
        //mutex.unlock();
    } catch(H5::AttributeIException error) {
        std::cout << " ... error" << std::endl;
        error.printError();
        //mutex.unlock();
        throw std::runtime_error(error.getCDetailMsg());
    }
    HDF5Attribute *at = new HDF5Attribute(attr);
    attr.close();
    return at;
}

/**
 * @brief HDF5File::HDF5Object::removeAttribute Remove attribute by idx
 * @param idx
 * @throw std::runtime_error
 */
void HDF5File::HDF5Object::removeAttribute(const unsigned int idx)
{
    //mutex.lock();
    try {
        std::cout << "Removing attribute " << idx;
        object->removeAttr(object->openAttribute(idx).getName());
        std::cout << " ... OK" << std::endl;
    } catch(H5::AttributeIException error) {
        std::cout << " ... error" << std::endl;
    }
    //mutex.unlock();
}

/**
 * @brief HDF5File::HDF5Object::removeAttribute Remove attribute by name
 * @param name
 * @throw std::runtime_error
 */
void HDF5File::HDF5Object::removeAttribute(const H5std_string name)
{
    //mutex.lock();
    try {
        std::cout << "Removing attribute \"" << name << "\"";
        object->removeAttr(name);
        std::cout << " ... OK" << std::endl;
    } catch(H5::AttributeIException error) {
        std::cout << " ... error" << std::endl;
    }
    //mutex.unlock();
}

/**
 * @brief HDF5File::HDF5Object::setAttribute Set attribute by HDF5Attribute
 * @param attribute
 * @throw std::runtime_error
 */
void HDF5File::HDF5Object::setAttribute(HDF5Attribute *attribute)
{
    //mutex.lock();
    try {
        object->removeAttr(attribute->getName());
    } catch(H5::AttributeIException error) {

    }
    //mutex.unlock();

    try {
        //mutex.lock();
        std::cout << "Creating attribute \"" << attribute->getName() << "\"";
        // Copy attribute
        H5::Attribute att = object->createAttribute(attribute->getName(), attribute->getDataType(), attribute->getSpace());
        att.write(attribute->getDataType() , attribute->getData());
        att.close();
        std::cout << " ... OK" << std::endl;
        //mutex.unlock();
    } catch(H5::AttributeIException error) {
        std::cout << " ... error" << std::endl;
        error.printError();
        //mutex.unlock();
        throw std::runtime_error(error.getCDetailMsg());
    }
}

/**
 * @brief HDF5File::HDF5Object::setAttribute Set attribute by name and integer value
 * @param name
 * @param value
 * @throw std::runtime_error
 */
void HDF5File::HDF5Object::setAttribute(const H5std_string name, int value)
{
    //mutex.lock();
    try {
        object->removeAttr(name);
    } catch(H5::AttributeIException error) {

    }
    //mutex.unlock();

    try {
        //mutex.lock();
        std::cout << "Creating int attribute \"" << name << " = " << value << "\"";
        H5::IntType type(H5::PredType::NATIVE_INT);
        H5::DataSpace att_space(H5S_SCALAR);
        H5::Attribute att = object->createAttribute(name, type, att_space);
        att.write(type, &value);
        att.close();
        std::cout << " ... OK" << std::endl;
        //mutex.unlock();
    } catch(H5::AttributeIException error) {
        std::cout << " ... error" << std::endl;
        error.printError();
        //mutex.unlock();
        throw std::runtime_error(error.getCDetailMsg());
    }
}

/**
 * @brief HDF5File::HDF5Object::setAttribute Set attribute by name and uint64_t value
 * @param name
 * @param value
 * @throw std::runtime_error
 */
void HDF5File::HDF5Object::setAttribute(const H5std_string name, uint64_t value)
{
    //mutex.lock();
    try {
        object->removeAttr(name);
    } catch(H5::AttributeIException error) {

    }
    //mutex.unlock();

    try {
        //mutex.lock();
        std::cout << "Creating uint64_t attribute \"" << name << " = " << value << "\"";
        H5::IntType type(H5::PredType::NATIVE_UINT64);
        H5::DataSpace att_space(H5S_SCALAR);
        H5::Attribute att = object->createAttribute(name, type, att_space);
        att.write(type, &value);
        att.close();
        std::cout << " ... OK" << std::endl;
        //mutex.unlock();
    } catch(H5::AttributeIException error) {
        std::cout << " ... error" << std::endl;
        error.printError();
        //mutex.unlock();
        throw std::runtime_error(error.getCDetailMsg());
    }
}

/**
 * @brief HDF5File::HDF5Object::setAttribute Set attribute by name and double value
 * @param name
 * @param value
 * @throw std::runtime_error
 */
void HDF5File::HDF5Object::setAttribute(const H5std_string name, double value)
{
    //mutex.lock();
    try {
        object->removeAttr(name);
    } catch(H5::AttributeIException error) {

    }
    //mutex.unlock();

    try {
        //mutex.lock();
        std::cout << "Creating double attribute \"" << name << " = " << value << "\"";
        H5::IntType type(H5::PredType::NATIVE_DOUBLE);
        H5::DataSpace att_space(H5S_SCALAR);
        H5::Attribute att = object->createAttribute(name, type, att_space);
        att.write(type, &value);
        att.close();
        std::cout << " ... OK" << std::endl;
        //mutex.unlock();
    } catch(H5::AttributeIException error) {
        std::cout << " ... error" << std::endl;
        error.printError();
        //mutex.unlock();
        throw std::runtime_error(error.getCDetailMsg());
    }
}

/**
 * @brief HDF5File::HDF5Object::setAttribute Set attribute by name and float value
 * @param name
 * @param value
 * @throw std::runtime_error
 */
void HDF5File::HDF5Object::setAttribute(const H5std_string name, float value)
{
    //mutex.lock();
    try {
        object->removeAttr(name);
    } catch(H5::AttributeIException error) {

    }
    //mutex.unlock();

    try {
        //mutex.lock();
        std::cout << "Creating float attribute \"" << name << " = " << value << "\"";
        H5::IntType type(H5::PredType::NATIVE_FLOAT);
        H5::DataSpace att_space(H5S_SCALAR);
        H5::Attribute att = object->createAttribute(name, type, att_space);
        att.write(type, &value);
        att.close();
        std::cout << " ... OK" << std::endl;
        //mutex.unlock();
    } catch(H5::AttributeIException error) {
        std::cout << " ... error" << std::endl;
        error.printError();
        //mutex.unlock();
        throw std::runtime_error(error.getCDetailMsg());
    }
}

/**
 * @brief HDF5File::HDF5Object::setAttribute Set attribute by name and string value
 * @param name
 * @param value
 * @throw std::runtime_error
 */
void HDF5File::HDF5Object::setAttribute(const H5std_string name, const H5std_string value)
{
    //mutex.lock();
    try {
        object->removeAttr(name);
    } catch(H5::AttributeIException error) {

    }
    //mutex.unlock();

    try {
        //mutex.lock();
        std::cout << "Creating string attribute \"" << name << " = " << value << "\"";
        H5::StrType type(0, H5T_VARIABLE);
        H5::DataSpace att_space(H5S_SCALAR);
        H5::Attribute att = object->createAttribute(name, type, att_space);
        const char *str = value.c_str();
        att.write(type, &str);
        att.close();
        std::cout << " ... OK" << std::endl;
        //mutex.unlock();
    } catch(H5::AttributeIException error) {
        std::cout << " ... error" << std::endl;
        error.printError();
        //mutex.unlock();
        throw std::runtime_error(error.getCDetailMsg());
    }
}

/**
 * @brief HDF5File::HDF5Object::readAttributeF Read float attribute
 * @param name
 * @return float value
 */
float HDF5File::HDF5Object::readAttributeF(const H5std_string name)
{
    float value;
    try {
        mutex.lock();
        std::cout << "Reading float attribute \"" << name << "\"";
        H5::FloatType type(H5::PredType::NATIVE_FLOAT);
        object->openAttribute(name).read(type, &value);
        std::cout << " ... OK" << std::endl;
        mutex.unlock();
    } catch(H5::AttributeIException error) {
        std::cout << " ... error" << std::endl;
        error.printError();
        mutex.unlock();
        throw std::runtime_error(error.getCDetailMsg());
    }
    return value;
}

/**
 * @brief HDF5File::HDF5Object::readAttributeI Read uint64_t attribute
 * @param name
 * @return uint64_t value
 */
uint64_t HDF5File::HDF5Object::readAttributeI(const H5std_string name)
{
    uint64_t value;
    try {
        mutex.lock();
        std::cout << "Reading uint64_t attribute \"" << name << "\"";
        H5::IntType type(H5::PredType::NATIVE_UINT64);
        object->openAttribute(name).read(type, &value);
        std::cout << " ... OK" << std::endl;
        mutex.unlock();
    } catch(H5::AttributeIException error) {
        std::cout << " ... error" << std::endl;
        error.printError();
        mutex.unlock();
        throw std::runtime_error(error.getCDetailMsg());
    }
    return value;
}

/**
 * @brief HDF5File::HDF5Object::readAttributeS Read string attribute
 * @param name
 * @return string value
 */
H5std_string HDF5File::HDF5Object::readAttributeS(const H5std_string name)
{
    char *value;
    H5std_string retValue;
    try {
        mutex.lock();
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
        mutex.unlock();
    } catch(H5::DataTypeIException error) {
        std::cout << " ... error" << std::endl;
        error.printError();
        mutex.unlock();
        throw std::runtime_error(error.getCDetailMsg());
    } catch(H5::AttributeIException error) {
        std::cout << " ... error" << std::endl;
        error.printError();
        mutex.unlock();
        throw std::runtime_error(error.getCDetailMsg());
    }
    return retValue;
}

