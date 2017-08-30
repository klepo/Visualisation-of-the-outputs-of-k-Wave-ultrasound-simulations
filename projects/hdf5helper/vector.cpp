/**
 * @file        vector.cpp
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@stud.fit.vutbr.cz
 * @version     1.1
 * @date        16 June      2016 (created) \n
 *              28 August    2017 (updated)
 *
 * @brief       The implementation file containing HDF5Helper::Vector class definition.
 *              This class is used for multidimensional integer vector representation.
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

#include <vector.h>

namespace HDF5Helper {

/**
 * @brief Creates Vector with zero length
 */
Vector::Vector()
    : vector(0)
    , length(0)
{

}

/**
 * @brief Creates Vector with given length and fill value
 * @param[in] length Vector length
 * @param[in] value Fill value
 */
Vector::Vector(hsize_t length, hsize_t value)
{
    this->length = length;
    vector = new hsize_t[length]();
    for (hsize_t i = 0; i < length; i++)
        vector[i] = value;
}

/**
 * @brief Creates Vector with given integer length and fill value
 * @param[in] length Vector length
 * @param[in] value Fill value
 * @throw std::runtime_error
 */
Vector::Vector(int length, hsize_t value)
{
    if (length >= 0) {
        this->length = static_cast<hsize_t>(length);
        vector = new hsize_t[static_cast<hsize_t>(length)]();
        for (int i = 0; i < length; i++)
            vector[i] = value;
    } else {
        throw std::runtime_error("Length of Vector is less than zero");
    }
}

/**
 * @brief Copy constructor
 * @param[in] vector Original Vector object to copy
 */
Vector::Vector(const Vector &vector)
{
    assign(vector, false);
}

/**
 * @brief Destructor of Vector object
 *
 * Does nothing.
 */
Vector::~Vector()
{
    delete[] vector;
    vector = 0;
}

/**
 * @brief Assignment operator
 * @param[in] vector Reference to the existing Vector
 * @return Reference to Vector instance
 */
Vector &Vector::operator =(const Vector &vector)
{
    if (this != &vector) {
        assign(vector, true);
    }
    return *this;
}

/**
 * @brief Relational operator ==
 * @param[in] vector Reference to the existing Vector
 * @return True/False
 */
bool Vector::operator ==(const Vector &vector) const
{
    if (length != vector.length)
        return false;
    for (hsize_t i = 0; i < length; i++) {
        if (this->vector[i] != vector.vector[i])
            return false;
    }
    return true;
}

/**
 * @brief Relational operator !=
 * @param[in] vector Reference to the existing Vector
 * @return True/False
 */
bool Vector::operator !=(const Vector &vector) const
{
    if (vector == *this)
        return false;
    return true;
}

/**
 * @brief Subscripting operator []
 * @param i Index
 * @return Vector value at index
 * @throw std::runtime_error
 */
hsize_t &Vector::operator [](hsize_t i)
{
    if (i >= length) {
        throw std::runtime_error("Index to Vector is too big");
    }
    return vector[i];
}

/**
 * @brief Subscripting operator []
 * @param i Index
 * @return Vector value at index
 * @throw std::runtime_error
 */
hsize_t &Vector::operator [](hssize_t i)
{
    if (static_cast<hsize_t>(i) >= length) {
        throw std::runtime_error("Index to Vector is too big");
    }
    return vector[static_cast<hsize_t>(i)];
}

/**
 * @brief Subscripting operator []
 * @param i Index
 * @return Vector value at index
 * @throw std::runtime_error
 */
hsize_t &Vector::operator [](int i)
{
    if (static_cast<hsize_t>(i) >= length) {
        throw std::runtime_error("Index to Vector is too big");
    }
    return vector[static_cast<hsize_t>(i)];
}

/**
 * @brief Subscripting operator []
 * @param i Index
 * @return Vector value at index
 * @throw std::runtime_error
 */
hsize_t &Vector::operator [](unsigned int i)
{
    if (static_cast<hsize_t>(i) >= length) {
        throw std::runtime_error("Index to Vector is too big");
    }
    return vector[static_cast<hsize_t>(i)];
}

/**
 * @brief Returns size of vector
 * @return Size of vector
 */
hsize_t Vector::getSize() const
{
    hsize_t size = 1;
    for (hsize_t i = 0; i < length; i++) {
        size *= vector[i];
    }
    return size;
}

/**
 * @brief Returns vector pointer
 * @return Vector pointer
 */
hsize_t *Vector::getVectorPtr()
{
    return vector;
}

/**
 * @brief Returns length of vector
 * @return Length of vector
 */
hsize_t Vector::getLength() const
{
    return length;
}

/**
 * @brief Has vector everywhere zeros?
 * @return True/False
 */
bool Vector::hasZeros() const
{
    for (hsize_t i = 0; i < length; i++) {
        if (vector[i] == 0)
            return true;
    }
    return false;
}

/**
 * @brief Operator std::string
 * @return String
 */
HDF5Helper::Vector::operator std::string() const
{
    std::string str = "";
    for (hsize_t i = 0; i < length; i++) {
        str += std::to_string(vector[i]);
        if (i < length - 1)
            str += " x ";
    }
    return str;
}

/**
 * @brief Helper assignment function
 * @param[in] vector Reference to the existing Vector
 * @param deleteFlag Delete flag (optional)
 */
void Vector::assign(const Vector &vector, bool deleteFlag)
{
    if (deleteFlag) {
        delete[] this->vector;
    }
    length = vector.length;

    this->vector = new hsize_t[length]();
    std::memcpy(this->vector, vector.vector, static_cast<size_t>(length) * sizeof(hsize_t));
}
}
