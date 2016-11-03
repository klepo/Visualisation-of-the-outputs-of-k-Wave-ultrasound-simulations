#version 330

uniform sampler3D uVolume;
uniform sampler1D uColormap;
uniform sampler1D uOpacity;
uniform sampler2D uSlice;
uniform sampler2D uBoxSampler;

uniform bool uFrame;
uniform bool uSlices;
uniform bool uXYBorder;
uniform bool uXZBorder;
uniform bool uYZBorder;
uniform bool uVolumeRenderingBack;
uniform bool uVolumeRendering;

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

void trim(float value) {
    if (uTrim) {
        if (uMin <= uMax) {
            if (value > uMax) {
                discard;
                return;
            }
            if (value < uMin) {
                discard;
                return;
            }
        } else {
            if (value < uMax) {
                discard;
                return;
            }
            if (value > uMin) {
                discard;
                return;
            }
        }
    }
}

vec4 computeColor(float texel)
{
    trim(texel);
    texel = ((texel - uMin) * 1.0f) / (uMax - uMin);
    vec4 color = texture(uColormap, texel);
    vec4 opacity = texture(uOpacity, texel);
    return vec4(color.rgb, opacity);
}

void main() {
    if (uFrame) {
        outColor = uFrameColor;
    } else if (uVolumeRenderingBack) {
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
            trim(value);
            value = ((value - uMin) * 1.0f) / (uMax - uMin);
            outColor = vec4(texture(uColormap, value).rgb, 1.0f);
            //outColor = vec4(vTextureCoordBox.stp, 1.0f);
        }
    } else if (uVolumeRendering) {
        vec2 coords = vec2((gl_FragCoord.x / uWidth), (gl_FragCoord.y / uHeight));
        vec3 backPoint = vec3(texture(uBoxSampler, coords));
        vec3 frontPoint = vTextureCoordBox.stp;

        vec3 path = normalize(backPoint - frontPoint) / uSteps;
        float depth = distance(frontPoint, backPoint);
        vec3 point = frontPoint;
        float texel = 0.0f;

        //outColor = vec4(vec3(depth), 1.0f);
        //outColor = vec4(backPoint, 1.0f);
        //return;

        int i = 0;
        int it = int(ceil(depth / float(length(path))));

        vec4 cOut = vec4(0, 0, 0, 0);
        for (i = 0; i < it; i++) {
            if (uMode == 1) { // Maximum intensity projection
                texel = max(texture(uVolume, point).r, texel);
            } else if (uMode == 2) { // Minimum intensity projection
                texel = min(texture(uVolume, point).r, texel);
            } else if (uMode == 0) { // Accumulation
                vec4 cIn = computeColor(texture(uVolume, point).r);
                cIn.a = 50 * cIn.a / uSteps;
                cOut.rgb = cOut.rgb + (1 - cOut.a) * cIn.rgb * cIn.a;
                cOut.a   = cOut.a   + (1 - cOut.a) * cIn.a;
            } else if (uMode == 3) { // Average intensity projection
                texel += texture(uVolume, point).r;
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
            //trim(texel);
            outColor = computeColor(texel);
        }

    } else {
        outColor = vec4(0.5f, 0.5f, 0.5f, 1.0f);
    }
}
