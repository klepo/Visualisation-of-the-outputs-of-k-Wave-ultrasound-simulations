/**
 * @file        HDF5Vector.h
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        16 June      2016 (created) \n
 *              27 February  2017 (updated)
 *
 * @brief       The header file with HDF5Vector class declaration.
 *
 * @license     This file is part of the hdf5helper library for k-Wave h5 processing
 *              for preprocessing the HDF5 data created by the k-Wave toolbox - http://www.k-wave.org.
 *              The hdf5helper library is free software.
 *
 * @copyright   Copyright © 2017, Petr Kleparnik, VUT FIT Brno. All Rights Reserved.
 *
 */

#ifndef HDF5VECTOR_H
#define HDF5VECTOR_H

#ifdef __unix
#include <stdexcept>
#endif

#include <iostream>
#include <string>
#include <cstdint>
#include <cstring>

#include <hdf5.h>  // HDF5

namespace HDF5Helper
{
/**
 * @brief The HDF5Vector class represents structure for integer vectors
 */
class HDF5Vector
{
public:
    HDF5Vector();
    HDF5Vector(hsize_t length, hsize_t value = 0);
    HDF5Vector(int length, hsize_t value = 0);
    HDF5Vector(const HDF5Vector &hDF5Vector);
    virtual ~HDF5Vector();
    virtual bool operator ==(const HDF5Vector &hDF5Vector) const;
    virtual bool operator !=(const HDF5Vector &hDF5Vector) const;
    virtual HDF5Vector &operator =(const HDF5Vector &hDF5Vector);
    virtual hsize_t &operator [](hsize_t i) final;
    virtual hsize_t &operator [](hssize_t i) final;
    virtual hsize_t &operator [](int i) final;
    virtual hsize_t &operator [](unsigned int i) final;
    virtual hsize_t getSize() const final;
    virtual hsize_t *getVectorPtr() final;
    hsize_t getLength() const;
    bool hasZeros() const;

    friend std::ostream &operator<<(std::ostream &os, const HDF5Vector &hDF5Vector) {
        os << std::string(hDF5Vector);
        return os;
    }

    operator std::string() const;

private:
    void assign(const HDF5Vector &hDF5Vector, bool deleteFlag);

protected:
    hsize_t *vector;
    hsize_t length;
};
}

#endif // HDF5VECTOR_H
