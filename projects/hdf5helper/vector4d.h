/**
 * @file        vector4d.h
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        30 July      2014 (created) \n
 *              30 August    2017 (updated)
 *
 * @brief       The header file with HDF5Helper::Vector4D class declaration.
 *
 * @license     This file is part of the hdf5helper library for processing the HDF5 data
 *              created by the k-Wave toolbox - http://www.k-wave.org. This file may be used,
 *              distributed and modified under the terms of the LGPL version 3 open source
 *              license. A copy of the LGPL license should have been received with this file.
 *              Otherwise, it can be found at: http://www.gnu.org/copyleft/lesser.html.
 *
 * @copyright   Copyright © 2017, Petr Kleparnik, VUT FIT Brno. All Rights Reserved.
 *
 */

#ifndef VECTOR4D_H
#define VECTOR4D_H

#include <vector.h>

namespace HDF5Helper
{
class Vector3D;
/**
 * @brief The Vector4D class represents wrapper for 4D 64-bit unsigned integer vectors
 */
class Vector4D : public Vector
{
public:
    Vector4D();
    Vector4D(const Vector &vector);
    Vector4D(hsize_t value);
    Vector4D(hsize_t value, const Vector3D &vector);
    Vector4D(hsize_t w, hsize_t z, hsize_t y, hsize_t x);
    void set(hsize_t w, hsize_t z, hsize_t y, hsize_t x);
    void set(int w, int z, int y, int x);
    void x(hsize_t x) const;
    void y(hsize_t y) const;
    void z(hsize_t z) const;
    void w(hsize_t w) const;
    void t(hsize_t t) const;
    void x(int x) const;
    void y(int y) const;
    void z(int z) const;
    void w(int w) const;
    void t(int t) const;
    hsize_t x() const;
    hsize_t y() const;
    hsize_t z() const;
    hsize_t w() const;
    hsize_t t() const;
};
}

#endif // VECTOR4D_H
