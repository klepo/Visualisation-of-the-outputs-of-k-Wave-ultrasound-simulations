/**
 * @file        vector3d.h
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        30 July      2014 (created) \n
 *              28 August    2017 (updated)
 *
 * @brief       The header file with HDF5Helper::Vector3D class declaration.
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

#ifndef VECTOR3D_H
#define VECTOR3D_H

#include <vector.h>

namespace HDF5Helper
{
class Vector4D;
/**
 * @brief The Vector3D class represents wrapper for 3D 64-bit unsigned integer vectors
 */
class Vector3D : public Vector
{
public:
    Vector3D();
    Vector3D(const Vector &vector);
    Vector3D(const Vector4D &vector);
    Vector3D(const hsize_t value);
    Vector3D(const hsize_t z, const hsize_t y, const hsize_t x);
    void set(const hsize_t z, const hsize_t y, const hsize_t x);
    void set(const int z, const int y, const int x);
    void x(const hsize_t x);
    void y(const hsize_t y);
    void z(const hsize_t z);
    void x(const int x);
    void y(const int y);
    void z(const int z);
    hsize_t x() const;
    hsize_t y() const;
    hsize_t z() const;
};
}

#endif // VECTOR3D_H
