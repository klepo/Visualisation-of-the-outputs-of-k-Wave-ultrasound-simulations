/**
 * @file        vertexshader.vert
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        3  November  2016 (created) <br>
 *              26 September 2018 (updated)
 *
 * @brief       Vertex shader for slice and volume rendering.
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

#version 330

in vec3 aPosition;

uniform mat4 uMatrix;
uniform mat4 uSliceMatrix;

out vec2 vTextureCoord;
out vec3 vTextureCoordBox;

void main() {
   vTextureCoordBox = (uSliceMatrix * vec4(aPosition, 1.0f)).xyz;
   //vTextureCoord = vec2(aPosition.x, 1 - aPosition.y);
   vTextureCoord = aPosition.xy;
   gl_Position = uMatrix * vec4(aPosition, 1.0f);
}
