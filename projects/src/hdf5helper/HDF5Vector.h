/*
 * @file        HDF5Vector.h
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     0.0
 * @date        16 June 2016
 *
 * @brief       The header file with HDF5Vector class declaration.
 *
 * @section     Licence
 * This file is part of hdf5file library for k-Wave h5 processing
 * for preprocessing HDF5 data created by the k-Wave toolbox - http://www.k-wave.org.
 * Copyright © 2016, Petr Kleparnik, VUT FIT Brno.
 * hdf5file library is free software.
 */

#ifndef HDF5VECTOR_H
#define HDF5VECTOR_H

#include "HDF5File.h"

namespace HDF5Helper
{
class HDF5Vector
{
public:
    HDF5Vector();
    HDF5Vector(hsize_t length, hsize_t value = 0);
    HDF5Vector(int length, hsize_t value = 0);
    HDF5Vector(const HDF5Vector &hDF5Vector);
    virtual ~HDF5Vector();
    virtual HDF5Vector &operator =(const HDF5Vector &hDF5Vector);
    virtual hsize_t &operator [](hsize_t i) final;
    virtual hsize_t &operator [](int i) final;
    virtual hsize_t &operator [](unsigned int i) final;
    virtual hsize_t getSize() const final;
    virtual hsize_t *getVectorPtr() final;
    hsize_t getLength() const;
    bool hasZeros() const;

    friend std::ostream &operator<<(std::ostream &os, HDF5Vector const &hDF5Vector) {
        for (hsize_t i = 0; i < hDF5Vector.length; i++) {
            os << hDF5Vector.vector[i];
            if (i < hDF5Vector.length - 1)
                os << " x ";
        }
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
