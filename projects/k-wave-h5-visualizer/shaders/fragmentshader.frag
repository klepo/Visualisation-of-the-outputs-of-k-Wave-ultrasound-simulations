/**
 * @file        fragmentshader.frag
 * @author      Petr Kleparnik, VUT FIT Brno, ikleparnik@fit.vutbr.cz
 * @version     1.1
 * @date        3  November  2016 (created) \n
 *
 * @brief       Fragment shader for slice and volume rendering.
 *
 * @license     This file is part of the k-Wave-h5-visualizer tool for processing the HDF5 data
 *              created by the k-Wave toolbox - http://www.k-wave.org. This file may be used,
 *              distributed and modified under the terms of the LGPL version 3 open source
 *              license. A copy of the LGPL license should have been received with this file.
 *              Otherwise, it can be found at: http://www.gnu.org/copyleft/lesser.html.
 *
 * @copyright   Copyright © 2017, Petr Kleparnik, VUT FIT Brno. All Rights Reserved.
 *
 */

#version 330

uniform sampler3D uVolume;
uniform sampler3D uVolumeCC;
uniform sampler3D uVolumeLC;
uniform samplerBuffer uTextureBE;
uniform samplerBuffer uTextureBE_1;
uniform sampler1D uColormap;
uniform sampler1D uOpacity;
uniform sampler2D uSlice;
uniform sampler2D uBoxBackSampler;
uniform sampler2D uBoxFrontSampler;

uniform bool uFrame;
uniform bool uSlices;
uniform bool uXYBorder;
uniform bool uXZBorder;
uniform bool uYZBorder;
uniform bool uVolumeRenderingBox;
uniform bool uVolumeRendering;
uniform bool uVolumeCompressRendering;

uniform int uMode;

uniform bool uTrim;

uniform int uSteps;

uniform vec4 uFrameColor;

uniform float uWidth;
uniform float uHeight;

uniform float uMin;
uniform float uMax;

in vec2 vTextureCoord;
in vec3 vTextureCoordBox;

out vec4 outColor;

uniform int uStepLocal;
uniform int uHarmonics;
uniform int uBSize;

ivec3 volumeSize;

bool isTrimmed(float value)
{
    if (uTrim) {
        if (uMin <= uMax) {
            if (value > uMax) {
                return true;
            }
            if (value < uMin) {
                return true;
            }
        } else {
            if (value < uMax) {
                return true;
            }
            if (value > uMin) {
                return true;
            }
        }
    }
    return false;
}

vec4 computeColor(float texel)
{
    if (isTrimmed(texel)) {
        return vec4(0, 0, 0, 0);
    }
    texel = ((texel - uMin) * 1.0f) / (uMax - uMin);
    vec4 color = texture(uColormap, texel);
    vec4 opacity = texture(uOpacity, texel);
    return vec4(color.rgb, opacity);
}

vec2 conjC(vec2 c)
{
    return vec2(c.x, -c.y);
}

float realC(vec2 c)
{
    return c.x;
}

vec2 mulC(vec2 a, vec2 b)
{
    return vec2(a.x * b.x - a.y * b.y, a.y * b.x + a.x * b.y);
}

vec2 getBE(int i)
{
    return vec2(texelFetch(uTextureBE, i).r, texelFetch(uTextureBE, i + 1).r);
}

vec2 getBE_1(int i)
{
    return vec2(texelFetch(uTextureBE_1, i).r, texelFetch(uTextureBE_1, i + 1).r);
}

float computeTimeStep(ivec3 pointI)
{
    ivec3 pointIReal = pointI;
    ivec3 pointIImag = pointI;
    pointIImag.x += 1;
    float stepValue = 0;
    for (int h = 0; h < uHarmonics; h++) {
        int sH = 2 * (h * uBSize + uStepLocal);
        vec2 lCC = conjC(vec2(texelFetch(uVolumeCC, pointIReal, 0).r, texelFetch(uVolumeCC, pointIImag, 0).r));
        vec2 cCC = conjC(vec2(texelFetch(uVolumeLC, pointIReal, 0).r, texelFetch(uVolumeLC, pointIImag, 0).r));
        stepValue += realC(mulC(cCC, getBE(sH))) + realC(mulC(lCC, getBE_1(sH)));
        //stepValue += realC(mulC(vec2(300000, 300000), getBE(sH))) + realC(mulC(vec2(300000, 300000), getBE_1(sH)));
        pointIReal.x += 2;
        pointIImag.x += 2;
    }
    return stepValue;
}

float getTexelValue(vec3 point)
{
    if (uVolumeCompressRendering) {
        ivec3 pointI = ivec3(vec3(volumeSize) * point);
        pointI.x = pointI.x * 2 * uHarmonics;
        return computeTimeStep(pointI);
        //return texelFetch(uVolumeLC, pointI, 0).r;
    } else {
        return texture(uVolume, point).r;
    }
}

void main() {
    if (uFrame) {
        outColor = uFrameColor;
    } else if (uVolumeRenderingBox) {
        outColor = vec4(vTextureCoordBox.stp, 1.0f);
    } else if (uSlices) {
        if (uXYBorder) {
            outColor = vec4(0.0f, 0.0f, 0.8f, 1.0f);
        } else if (uXZBorder) {
            outColor = vec4(0.0f, 0.8f, 0.0f, 1.0f);
        } else if (uYZBorder) {
            outColor = vec4(0.8f, 0.0f, 0.0f, 1.0f);
        } else {
            float value = texture(uSlice, vTextureCoord).r;
            if (isTrimmed(value)) {
                discard;
                return;
            }
            value = ((value - uMin) * 1.0f) / (uMax - uMin);
            outColor = vec4(texture(uColormap, value).rgb, 1.0f);
            //outColor = vec4(vTextureCoordBox.stp, 1.0f);
        }
    } else if (uVolumeRendering) {
        if (uVolumeCompressRendering) {
            volumeSize = textureSize(uVolume, 0);
        }
        vec2 coords = vec2((gl_FragCoord.x / uWidth), (gl_FragCoord.y / uHeight));
        vec3 backPoint = vec3(texture(uBoxBackSampler, coords));
        vec3 frontPoint = vec3(texture(uBoxFrontSampler, coords));
        //vec3 frontPoint = vTextureCoordBox.stp;

        float depth = distance(frontPoint, backPoint);

        vec3 path = normalize(backPoint - frontPoint) / uSteps;
        vec3 point = frontPoint;
        float texel = 0.0f;

        int i = 0;
        int it = int(depth / length(path));

        //outColor = vec4(vec3(length(path)), 1.0f);
        //outColor = vec4(vec3(depth), 1.0f);
        //outColor = vec4(backPoint, 1.0f);
        //outColor = vec4(frontPoint, 1.0f);
        //return;

        vec4 cOut = vec4(0, 0, 0, 0);

        outColor = cOut;

        for (i = 0; i < it; i++) {
            if (uMode == 1) { // Maximum intensity projection
                texel = max(getTexelValue(point), texel);
            } else if (uMode == 2) { // Minimum intensity projection
                texel = min(getTexelValue(point), texel);
            } else if (uMode == 0) { // Accumulation
                vec4 cIn = computeColor(getTexelValue(point));
                //if (cIn.a == 0.0f)
                //    continue;
                cIn.a = 50 * cIn.a / uSteps;
                cOut.rgb = cOut.rgb + (1 - cOut.a) * cIn.rgb * cIn.a;
                cOut.a   = cOut.a   + (1 - cOut.a) * cIn.a;
                if (cOut.a >= 1.0f)
                    break;
            } else if (uMode == 3) { // Average intensity projection
                texel += getTexelValue(point);
            } else { // Unknown mode
                break;
            }
            point += path;
        }

        if (uMode == 3) { // Average intensity projection
            texel = texel / i;
        }

        if (uMode == 0) { // Accumulation
            outColor = cOut;
        } else {
            outColor = computeColor(texel);
        }

    } else {
        outColor = vec4(0.5f, 0.5f, 0.5f, 1.0f);
    }
}
