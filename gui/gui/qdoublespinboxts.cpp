/*
 * @file        mainwindow.cpp
 * @author      Petr Kleparnik, VUT FIT Brno, xklepa01@stud.fit.vutbr.cz
 * @version     0.0
 * @date        30 July 2014
 *
 * @brief       The implementation file containing the QDoubleSpinBoxTS class.
 *              Double spin box with group separator.
 *
 * @section     Licence
 * This file is part of k-Wave visualiser application
 * for visualizing HDF5 data created by the k-Wave toolbox - http://www.k-wave.org.
 * Copyright © 2014, Petr Kleparnik, VUT FIT Brno.
 * k-Wave visualiser is free software.
 */

#include "qdoublespinboxts.h"
#include <QDebug>

QDoubleSpinBoxTS::QDoubleSpinBoxTS(QWidget *parent) :
    QDoubleSpinBox(parent)
{
}


QString QDoubleSpinBoxTS::textFromValue(double value) const
{
    return QWidget::locale().toString(value, 'f', 4);
}

double QDoubleSpinBoxTS::valueFromText(const QString &text) const
{
    QString str = text;
    str = str.remove(QWidget::locale().groupSeparator());
    //str = str.remove(" ");
    double d = QWidget::locale().toDouble(str);
    return d;
}
