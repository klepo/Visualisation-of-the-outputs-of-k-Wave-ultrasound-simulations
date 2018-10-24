/**
 * @file        vertices.h
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        30 July      2014 (created) <br>
 *              10 October   2018 (updated)
 *
 * @brief       Some helper arrays for slices and 3D frame.
 *
 * @license     This file is part of the k-Wave-h5-visualizer tool for processing the HDF5 data
 *              created by the k-Wave toolbox - http://www.k-wave.org. This file may be used,
 *              distributed and modified under the terms of the LGPL version 3 open source
 *              license. A copy of the LGPL license should have been received with this file.
 *              Otherwise, it can be found at: http://www.gnu.org/copyleft/lesser.html.
 *
 * @copyright   Copyright © 2018, Petr Kleparnik, VUT FIT Brno. All Rights Reserved.
 *
 */

#ifndef GVERTICES_H
#define GVERTICES_H

#include "gwindow.h"

const GLfloat GWindow::sliceVertices[] = {
    0.0, 0.0, 0.0,
    1.0, 0.0, 0.0,
    1.0, 1.0, 0.0,
    0.0, 1.0, 0.0,
};

const GLint GWindow::sliceElements[] = {
    0, 1, 2,
    2, 3, 0,
};

const GLfloat GWindow::cubeVertices[] = {
    // front
    0.0, 0.0, 0.0,
    1.0, 0.0, 0.0,
    1.0, 1.0, 0.0,
    0.0, 1.0, 0.0,
    // back
    0.0, 0.0, 1.0,
    1.0, 0.0, 1.0,
    1.0, 1.0, 1.0,
    0.0, 1.0, 1.0,
};

const GLint GWindow::cubeElements[] = {
    // front
    0, 1, 2,
    2, 3, 0,
    // left
    4, 0, 3,
    3, 7, 4,
    // back
    5, 4, 7,
    7, 6, 5,
    // right
    1, 5, 6,
    6, 2, 1,
    // top
    2, 6, 7,
    7, 3, 2,
    // right
    1, 0, 4,
    4, 5, 1,
};

#endif // GVERTICES_H
