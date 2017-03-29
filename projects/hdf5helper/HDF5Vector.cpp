/**
 * @file        HDF5Vector.cpp
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@stud.fit.vutbr.cz
 * @version     1.1
 * @date        16 June      2016 (created) \n
 *              27 February  2017 (updated)
 *
 * @brief       The implementation file containing HDF5Vector class definition.
 *              This class is for multidimensional point representation.
 *
 * @license     This file is part of the hdf5helper library for k-Wave h5 processing
 *              for preprocessing the HDF5 data created by the k-Wave toolbox - http://www.k-wave.org.
 *              The hdf5helper library is free software.
 *
 * @copyright   Copyright © 2017, Petr Kleparnik, VUT FIT Brno. All Rights Reserved.
 *
 */

#include <HDF5Vector.h>

namespace HDF5Helper {

/**
 * @brief HDF5Vector::HDF5Vector
 */
HDF5Vector::HDF5Vector()
    : vector(0)
    , length(0)
{

}

/**
 * @brief HDF5Vector::HDF5Vector
 * @param length
 * @param value
 */
HDF5Vector::HDF5Vector(hsize_t length, hsize_t value)
{
    this->length = length;
    vector = new hsize_t[length]();
    //std::fill_n(vector, length, value);
    for (hsize_t i = 0; i < length; i++)
        vector[i] = value;
}

/**
 * @brief HDF5Vector::HDF5Vector
 * @param length
 * @param value
 * @throw std::runtime_error
 */
HDF5Vector::HDF5Vector(int length, hsize_t value)
{
    if (length >= 0) {
        this->length = static_cast<hsize_t>(length);
        vector = new hsize_t[static_cast<hsize_t>(length)]();
        //std::fill_n(vector, length, value);
        for (int i = 0; i < length; i++)
            vector[i] = value;
    } else {
        throw std::runtime_error("Length of HDF5Vector is less than zero");
    }
}

/**
 * @brief HDF5Vector::HDF5Vector
 * @param hDF5Vector
 */
HDF5Vector::HDF5Vector(const HDF5Vector &hDF5Vector)
{
    assign(hDF5Vector, false);
}

/**
 * @brief HDF5Vector::operator =
 * @param hDF5Vector
 * @return
 */
HDF5Vector &HDF5Vector::operator =(const HDF5Vector &hDF5Vector)
{
    if (this != &hDF5Vector) {
        assign(hDF5Vector, true);
    }
    return *this;
}

/**
 * @brief HDF5Vector::~HDF5Vector
 */
HDF5Vector::~HDF5Vector()
{
    delete[] vector;
    vector = 0;
}

/**
 * @brief HDF5Vector::operator ==
 * @param hDF5Vector
 * @return
 */
bool HDF5Vector::operator ==(const HDF5Vector &hDF5Vector) const
{
    if (length != hDF5Vector.length)
        return false;
    for (hsize_t i = 0; i < length; i++) {
        if (vector[i] != hDF5Vector.vector[i])
            return false;
    }
    return true;
}

/**
 * @brief HDF5Vector::operator !=
 * @param hDF5Vector
 * @return
 */
bool HDF5Vector::operator !=(const HDF5Vector &hDF5Vector) const
{
    if (hDF5Vector == *this)
        return false;
    return true;
}

/**
 * @brief HDF5Vector::operator []
 * @param i
 * @return
 * @throw std::runtime_error
 */
hsize_t &HDF5Vector::operator [](hsize_t i)
{
    if (i >= length) {
        throw std::runtime_error("Index to HDF5Vector is too big");
    }
    return vector[i];
}

/**
 * @brief HDF5Vector::operator []
 * @param i
 * @return
 * @throw std::runtime_error
 */
hsize_t &HDF5Vector::operator [](hssize_t i)
{
    if (static_cast<hsize_t>(i) >= length) {
        throw std::runtime_error("Index to HDF5Vector is too big");
    }
    return vector[static_cast<hsize_t>(i)];
}

/**
 * @brief HDF5Vector::operator []
 * @param i
 * @return
 * @throw std::runtime_error
 */
hsize_t &HDF5Vector::operator [](int i)
{
    if (static_cast<hsize_t>(i) >= length) {
        throw std::runtime_error("Index to HDF5Vector is too big");
    }
    return vector[static_cast<hsize_t>(i)];
}

/**
 * @brief HDF5Vector::operator []
 * @param i
 * @return
 * @throw std::runtime_error
 */
hsize_t &HDF5Vector::operator [](unsigned int i)
{
    if (static_cast<hsize_t>(i) >= length) {
        throw std::runtime_error("Index to HDF5Vector is too big");
    }
    return vector[static_cast<hsize_t>(i)];
}

/**
 * @brief HDF5Vector::getSize
 * @return
 */
hsize_t HDF5Vector::getSize() const
{
    hsize_t size = 1;
    for (hsize_t i = 0; i < length; i++) {
        size *= vector[i];
    }
    return size;
}

/**
 * @brief HDF5Vector::getVectorPtr
 * @return
 */
hsize_t *HDF5Vector::getVectorPtr()
{
    return vector;
}

/**
 * @brief HDF5Vector::assign
 * @param hDF5Vector
 * @param deleteFlag
 */
void HDF5Vector::assign(const HDF5Vector &hDF5Vector, bool deleteFlag)
{
    if (deleteFlag) {
        delete[] vector;
    }
    length = hDF5Vector.length;

    vector = new hsize_t[length]();
    std::memcpy(vector, hDF5Vector.vector, static_cast<size_t>(length) * sizeof(hsize_t));
}

/**
 * @brief HDF5Vector::getLength
 * @return
 */
hsize_t HDF5Vector::getLength() const
{
    return length;
}

/**
 * @brief HDF5Vector::hasZeros
 * @return
 */
bool HDF5Vector::hasZeros() const
{
    for (hsize_t i = 0; i < length; i++) {
        if (vector[i] == 0)
            return true;
    }
    return false;
}

/**
 * @brief HDF5Helper::HDF5Vector::operator std::string
 */
HDF5Helper::HDF5Vector::operator std::string() const
{
    std::string str = "";
    for (hsize_t i = 0; i < length; i++) {
        str += std::to_string(vector[i]);
        if (i < length - 1)
            str += " x ";
    }
    return str;
}
}
