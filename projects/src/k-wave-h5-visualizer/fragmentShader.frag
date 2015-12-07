#version 330

#define highp
#define mediump
#define lowp

uniform sampler3D uSampler;
uniform float uAlpha;
uniform float uRed;
uniform float uGreen;
uniform float uBlue;

uniform sampler1D uColormapSampler;

in vec4 vPosition;

uniform float uXMax;
uniform float uYMax;
uniform float uZMax;
uniform float uXMin;
uniform float uYMin;
uniform float uZMin;

uniform float uWidth;
uniform float uHeight;
uniform float uDepth;

uniform float uMin;
uniform float uMax;

uniform bool uFrame;
uniform vec4 uFrameColor;

uniform bool uXYBorder;
uniform bool uXZBorder;
uniform bool uYZBorder;

uniform bool uSlices;

uniform bool uVolumeRendering;

uniform bool uTrim;

uniform sampler2D uSliceSampler;

in vec2 vTextureCoord;

out vec4 outColor;

void main() {
    if (uFrame) {

        outColor = uFrameColor;

    } else if (uXYBorder) {

        outColor = vec4(0.0, 0.0, 0.8, 1.0);

    } else if (uXZBorder) {

        outColor = vec4(0.0, 0.8, 0.0, 1.0);

    } else if (uYZBorder) {

        outColor = vec4(0.8, 0.0, 0.0, 1.0);

    } else if (uSlices) {
        vec4 color = texture(uSliceSampler, vTextureCoord);
        if (uTrim) {
            if (color.r > uMax) discard;
            if (color.r < uMin) discard;
        }

        float value = ((color.r - uMin) * 1.0) / (uMax - uMin);
        vec4 fColor = texture(uColormapSampler, value);

        outColor = vec4(fColor.rgb, 0.9);

    } else if (uVolumeRendering){

        if (vPosition.x > uXMax || vPosition.x < uXMin) discard;
        if (vPosition.y > uYMax || vPosition.y < uYMin) discard;
        if (vPosition.z > uZMax || vPosition.z < uZMin) discard;

        vec4 color = texture(uSampler, vec3((vPosition.x - 0.5f) * uWidth + 0.5f, (vPosition.y - 0.5f) * uHeight + 0.5f, (vPosition.z - 0.5f) * uDepth + 0.5f));

        if (uTrim) {
            if (color.r > uMax) discard;
            if (color.r < uMin) discard;
        }

        float value = ((color.r - uMin) * 1.0) / (uMax - uMin);
        vec4 fColor = texture(uColormapSampler, value);

        outColor = vec4(fColor.rgb, uAlpha + fColor.r * uRed + fColor.g * uGreen +  fColor.b * uBlue);
    } else {
        outColor = vec4(0.5, 0.5, 0.5, 1.0);
    }
}
