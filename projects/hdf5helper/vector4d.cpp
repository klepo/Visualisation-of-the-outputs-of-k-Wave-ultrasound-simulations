/**
 * @file        vector4d.cpp
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        30 July      2014 (created) \n
 *              28 August    2017 (updated)
 *
 * @brief       The implementation file containing HDF5Helper::Vector4D class definition.
 *              This class is used for 4D integer vector representation.
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

#include <vector4d.h>
#include <vector3d.h>

namespace HDF5Helper {

/**
 * @brief Creates Vector4D with zero values
 */
Vector4D::Vector4D()
    : Vector(4)
{
    set(0, 0, 0, 0);
}

/**
 * @brief Copy constructor for general vector
 * @param[in] vector Original Vector object to copy
 * @throw std::runtime_error
 */
Vector4D::Vector4D(const Vector &vector)
    : Vector(4)
{
    Vector tmp = vector;
    hsize_t length = vector.getLength();

    if (4 > length) {
        throw std::runtime_error("Convert error - The length of Vector is not 4");
    }

    set(tmp[length - 4], tmp[length - 3], tmp[length - 2], tmp[length - 1]);
}

/**
 * @brief Creates Vector4D with given fill value
 * @param[in] value Fill value
 */
Vector4D::Vector4D(const hsize_t value)
    : Vector(4, value)
{

}

/**
 * @brief Copy constructor for Vector3D
 * @param[in] value W value
 * @param[in] vector Original Vector3D object to copy
 */
Vector4D::Vector4D(const hsize_t value, const Vector3D &vector)
    : Vector(4)
{
    set(value, vector.z(), vector.y(), vector.x());
}

/**
 * @brief Creates Vector4D with given values
 * @param[in] w W value
 * @param[in] z Z value
 * @param[in] y Y value
 * @param[in] x X value
 */
Vector4D::Vector4D(const hsize_t w, const hsize_t z, const hsize_t y, const hsize_t x)
    : Vector(4)
{
    set(w, z, y, x);
}

/**
 * @brief Sets vector values
 * @param[in] w W value
 * @param[in] z Z value
 * @param[in] y Y value
 * @param[in] x X value
 */
void Vector4D::set(const hsize_t w, const hsize_t z, const hsize_t y, const hsize_t x)
{
    vector[0] = w;
    vector[1] = z;
    vector[2] = y;
    vector[3] = x;
}

/**
 * @brief Sets vector values
 * @param[in] w W value
 * @param[in] z Z value
 * @param[in] y Y value
 * @param[in] x X value
 */
void Vector4D::set(const int w, const int z, const int y, const int x)
{
    set(static_cast<hsize_t>(w), static_cast<hsize_t>(z), static_cast<hsize_t>(y), static_cast<hsize_t>(x));
}

/**
 * @brief Sets vector x value
 * @param[in] x X value
 */
void Vector4D::x(const hsize_t x)
{
    vector[3] = x;
}

/**
 * @brief Sets vector y value
 * @param[in] y Y value
 */
void Vector4D::y(const hsize_t y)
{
    vector[2] = y;
}

/**
 * @brief Sets vector z value
 * @param[in] z Z value
 */
void Vector4D::z(const hsize_t z)
{
    vector[1] = z;
}

/**
 * @brief Sets vector w value
 * @param[in] w W value
 */
void Vector4D::w(const hsize_t w)
{
    vector[0] = w;
}

/**
 * @brief Sets vector x value
 * @param[in] x X value
 */
void Vector4D::x(const int x)
{
    this->x(static_cast<hsize_t>(x));
}

/**
 * @brief Sets vector y value
 * @param[in] y Y value
 */
void Vector4D::y(const int y)
{
    this->y(static_cast<hsize_t>(y));
}

/**
 * @brief Sets vector z value
 * @param[in] z Z value
 */
void Vector4D::z(const int z)
{
    this->z(static_cast<hsize_t>(z));
}

/**
 * @brief Sets vector w value
 * @param[in] w W value
 */
void Vector4D::w(const int w)
{
    this->w(static_cast<hsize_t>(w));
}

/**
 * @brief Returns x value
 * @return X value
 */
hsize_t Vector4D::x() const
{
    return vector[3];
}

/**
 * @brief Returns y value
 * @return Y value
 */
hsize_t Vector4D::y() const
{
    return vector[2];
}

/**
 * @brief Returns z value
 * @return Z value
 */
hsize_t Vector4D::z() const
{
    return vector[1];
}

/**
 * @brief Returns w value
 * @return W value
 */
hsize_t Vector4D::w() const
{
    return vector[0];
}
}
