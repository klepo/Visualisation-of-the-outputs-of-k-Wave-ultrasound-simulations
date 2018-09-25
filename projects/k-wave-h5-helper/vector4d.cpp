/**
 * @file        vector4d.cpp
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        30 July      2014 (created) \n
 *              30 August    2017 (updated)
 *
 * @brief       The implementation file containing H5Helper::Vector4D class definition.
 *
 * This class is used for 4D 64-bit unsigned integer vector representation.
 *
 * @license     This file is part of the k-wave-h5-helper library for processing the HDF5 data
 *              created by the k-Wave toolbox - http://www.k-wave.org. This file may be used,
 *              distributed and modified under the terms of the LGPL version 3 open source
 *              license. A copy of the LGPL license should have been received with this file.
 *              Otherwise, it can be found at: http://www.gnu.org/copyleft/lesser.html.
 *
 * @copyright   Copyright © 2017, Petr Kleparnik, VUT FIT Brno. All Rights Reserved.
 *
 */

#include <vector4d.h>
#include <vector3d.h>

namespace H5Helper {

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
    copy(vector);
}

/**
 * @brief Creates Vector4D with given fill value
 * @param[in] value Fill value
 */
Vector4D::Vector4D(hsize_t value)
    : Vector(4, value)
{

}

/**
 * @brief Copy constructor for Vector3D
 * @param[in] value W value
 * @param[in] vector Original Vector3D object to copy
 */
Vector4D::Vector4D(hsize_t value, const Vector3D &vector)
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
Vector4D::Vector4D(hsize_t w, hsize_t z, hsize_t y, hsize_t x)
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
void Vector4D::set(hsize_t w, hsize_t z, hsize_t y, hsize_t x)
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
void Vector4D::set(int w, int z, int y, int x)
{
    set(static_cast<hsize_t>(w), static_cast<hsize_t>(z), static_cast<hsize_t>(y), static_cast<hsize_t>(x));
}

/**
 * @brief Sets vector x value
 * @param[in] x X value
 */
void Vector4D::x(hsize_t x) const
{
    vector[3] = x;
}

/**
 * @brief Sets vector y value
 * @param[in] y Y value
 */
void Vector4D::y(hsize_t y) const
{
    vector[2] = y;
}

/**
 * @brief Sets vector z value
 * @param[in] z Z value
 */
void Vector4D::z(hsize_t z) const
{
    vector[1] = z;
}

/**
 * @brief Sets vector w value
 * @param[in] w W value
 */
void Vector4D::w(hsize_t w) const
{
    vector[0] = w;
}

/**
 * @brief Sets vector t value (same as w)
 * @param[in] t T value
 */
void Vector4D::t(hsize_t t) const
{
    vector[0] = t;
}

/**
 * @brief Sets vector x value
 * @param[in] x X value
 */
void Vector4D::x(int x) const
{
    this->x(static_cast<hsize_t>(x));
}

/**
 * @brief Sets vector y value
 * @param[in] y Y value
 */
void Vector4D::y(int y) const
{
    this->y(static_cast<hsize_t>(y));
}

/**
 * @brief Sets vector z value
 * @param[in] z Z value
 */
void Vector4D::z(int z) const
{
    this->z(static_cast<hsize_t>(z));
}

/**
 * @brief Sets vector w value
 * @param[in] w W value
 */
void Vector4D::w(int w) const
{
    this->w(static_cast<hsize_t>(w));
}

/**
 * @brief Sets vector t value (same as w)
 * @param[in] t T value
 */
void Vector4D::t(int t) const
{
    this->t(static_cast<hsize_t>(t));
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

/**
 * @brief Returns t value (same as w)
 * @return T value
 */
hsize_t Vector4D::t() const
{
    return vector[0];
}
}
