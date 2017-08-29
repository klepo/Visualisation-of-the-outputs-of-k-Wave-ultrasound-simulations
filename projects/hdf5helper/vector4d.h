/**
 * @file        vector4d.h
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        30 July      2014 (created) \n
 *              28 August    2017 (updated)
 *
 * @brief       The header file with Vector4D class declaration.
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

#ifndef VECTOR4D_H
#define VECTOR4D_H

#include <vector.h>

namespace HDF5Helper
{
class Vector3D;
/**
 * @brief The Vector4D class represents wrapper for 4D integer vectors
 */
class Vector4D : public Vector
{
public:
    Vector4D();
    Vector4D(const Vector &vector);
    Vector4D(const hsize_t value);
    Vector4D(const hsize_t value, const Vector3D &vector);
    Vector4D(const hsize_t w, const hsize_t z, const hsize_t y, const hsize_t x);
    void set(const hsize_t w, const hsize_t z, const hsize_t y, const hsize_t x);
    void set(const int w, const int z, const int y, const int x);
    void x(const hsize_t x);
    void y(const hsize_t y);
    void z(const hsize_t z);
    void w(const hsize_t w);
    void x(const int x);
    void y(const int y);
    void z(const int z);
    void w(const int w);
    hsize_t x() const;
    hsize_t y() const;
    hsize_t z() const;
    hsize_t w() const;
};
}

#endif // VECTOR4D_H
