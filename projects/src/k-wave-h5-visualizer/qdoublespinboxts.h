/*
 * @file        qdoublespinboxts.h
 * @author      Petr Kleparnik, VUT FIT Brno, xklepa01@stud.fit.vutbr.cz
 * @version     0.0
 * @date        30 July      2014 (created)
 *              6  December  2015 (updated)
 *
 * @brief       The header file with QDoubleSpinBoxTS class declaration.
 *
 * @section     Licence
 * This file is part of k-Wave visualiser application
 * for visualizing HDF5 data created by the k-Wave toolbox - http://www.k-wave.org.
 * Copyright © 2014, Petr Kleparnik, VUT FIT Brno.
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
