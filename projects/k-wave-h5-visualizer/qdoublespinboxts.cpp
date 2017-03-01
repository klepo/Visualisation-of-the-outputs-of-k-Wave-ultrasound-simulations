/**
 * @file        qdoublespinboxts.cpp
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        30 July      2014 (created) \n
 *              27 February  2017 (updated)
 *
 * @brief       The implementation file containing the QDoubleSpinBoxTS class.
 *              Double spin box with group separator.
 *
 *
 * @license     This file is part of k-Wave visualiser application
 * for visualizing HDF5 data created by the k-Wave toolbox - http://www.k-wave.org.
 *
 * @copyright   Copyright © 2017, Petr Kleparnik, VUT FIT Brno. All Rights Reserved.
 *
 * k-Wave visualiser is free software.
 */

#include "qdoublespinboxts.h"

QDoubleSpinBoxTS::QDoubleSpinBoxTS(QWidget *parent) :
    QDoubleSpinBox(parent)
{
}


QString QDoubleSpinBoxTS::textFromValue(double value) const
{
    return QWidget::locale().toString(value, 'f', 3);
}

double QDoubleSpinBoxTS::valueFromText(const QString &text) const
{
    QString str = text;
    str = str.remove(QWidget::locale().groupSeparator());
    //str = str.remove(" ");
    double d = QWidget::locale().toDouble(str);
    return d;
}
