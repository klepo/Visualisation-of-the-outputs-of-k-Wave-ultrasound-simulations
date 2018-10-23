/**
 * @file        vector3d.h
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        30 July      2014 (created) <br>
 *              9  October   2018 (updated)
 *
 * @brief       The header file with H5Helper::Vector3D class declaration.
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

#ifndef VECTOR3D_H
#define VECTOR3D_H

#include <vector.h>

namespace H5Helper
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
    Vector3D(hsize_t value);
    Vector3D(hsize_t z, hsize_t y, hsize_t x);
    void set(hsize_t z, hsize_t y, hsize_t x);
    void set(int z, int y, int x);
    void x(hsize_t x) const;
    void y(hsize_t y) const;
    void z(hsize_t z) const;
    void x(int x) const;
    void y(int y) const;
    void z(int z) const;
    hsize_t x() const;
    hsize_t y() const;
    hsize_t z() const;
};
}

#endif // VECTOR3D_H
