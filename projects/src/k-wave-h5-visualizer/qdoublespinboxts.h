/**
 * @file        qdoublespinboxts.h
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        30 July      2014 (created)
 *              6  December  2015 (updated)
 *              8  September 2016 (updated)
 *
 * @brief       The header file with QDoubleSpinBoxTS class declaration.
 *
 *
 * @license     This file is part of k-Wave visualiser application
 * for visualizing HDF5 data created by the k-Wave toolbox - http://www.k-wave.org.
 *
 * @copyright   Copyright © 2016, Petr Kleparnik, VUT FIT Brno. All Rights Reserved.
 *
 * k-Wave visualiser is free software.
 */

#ifndef QDOUBLESPINBOXTS_H
#define QDOUBLESPINBOXTS_H

#include <QDoubleSpinBox>

class QDoubleSpinBoxTS : public QDoubleSpinBox
{
    Q_OBJECT
public:
    explicit QDoubleSpinBoxTS(QWidget *parent = 0);
    virtual QString textFromValue(double value) const;
    virtual double valueFromText(const QString &text) const;

signals:

public slots:

};

#endif // QDOUBLESPINBOXTS_H
