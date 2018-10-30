/**
 * @file        vector3d.cpp
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        30 July      2014 (created) <br>
 *              30 October   2018 (updated)
 *
 * @brief       The implementation file containing H5Helper::Vector3D class definition.
 *
 * This class is used for 3D 64-bit unsigned integer vector representation.
 *
 * @license     This file is part of the k-wave-h5-helper library for processing the HDF5 data
 *              created by the k-Wave toolbox - http://www.k-wave.org. This file may be used,
 *              distributed and modified under the terms of the LGPL version 3 open source
 *              license. A copy of the LGPL license should have been received with this file.
 *              Otherwise, it can be found at: http://www.gnu.org/copyleft/lesser.html.
 *
 * @copyright   Copyright © 2018, Petr Kleparnik, VUT FIT Brno. All Rights Reserved.
 *
 */

#include "vector3d.h"

namespace H5Helper {

/**
 * @brief Creates Vector3D with zero values
 */
Vector3D::Vector3D()
    : Vector(3)
{
    set(0, 0, 0);
}

/**
 * @brief Copy constructor for general vector
 * @param[in] vector Original Vector object to copy
 */
Vector3D::Vector3D(const Vector &vector)
    : Vector(3)
{
    copy(vector);
}

/**
 * @brief Creates Vector3D with given fill value
 * @param[in] value Fill value
 */
Vector3D::Vector3D(hsize_t value)
    : Vector(3 , value)
{
}

/**
 * @brief Creates Vector3D with given values
 * @param[in] z Z value
 * @param[in] y Y value
 * @param[in] x X value
 */
Vector3D::Vector3D(hsize_t z, hsize_t y, hsize_t x)
    : Vector(3)
{
    set(z, y, x);
}

/**
 * @brief Sets vector values
 * @param[in] z Z value
 * @param[in] y Y value
 * @param[in] x X value
 */
void Vector3D::set(hsize_t z, hsize_t y, hsize_t x)
{
    vector[0] = z;
    vector[1] = y;
    vector[2] = x;
}

/**
 * @brief Sets vector values
 * @param[in] z Z value
 * @param[in] y Y value
 * @param[in] x X value
 */
void Vector3D::set(int z, int y, int x)
{
    set(static_cast<hsize_t>(z), static_cast<hsize_t>(y), static_cast<hsize_t>(x));
}

/**
 * @brief Sets vector x value
 * @param[in] x X value
 */
void Vector3D::x(hsize_t x) const
{
    vector[2] = x;
}

/**
 * @brief Sets vector y value
 * @param[in] y Y value
 */
void Vector3D::y(hsize_t y) const
{
    vector[1] = y;
}

/**
 * @brief Sets vector z value
 * @param[in] z Z value
 */
void Vector3D::z(hsize_t z) const
{
    vector[0] = z;
}

/**
 * @brief Sets vector x value
 * @param[in] x X value
 */
void Vector3D::x(const int x) const
{
    this->x(static_cast<hsize_t>(x));
}

/**
 * @brief Sets vector y value
 * @param[in] y Y value
 */
void Vector3D::y(int y) const
{
    this->y(static_cast<hsize_t>(y));
}

/**
 * @brief Sets vector z value
 * @param[in] z Z value
 */
void Vector3D::z(int z) const
{
    this->z(static_cast<hsize_t>(z));
}

/**
 * @brief Returns x value
 * @return X value
 */
hsize_t Vector3D::x() const
{
    return vector[2];
}

/**
 * @brief Returns y value
 * @return Y value
 */
hsize_t Vector3D::y() const
{
    return vector[1];
}

/**
 * @brief Returns z value
 * @return Z value
 */
hsize_t Vector3D::z() const
{
    return vector[0];
}
}
