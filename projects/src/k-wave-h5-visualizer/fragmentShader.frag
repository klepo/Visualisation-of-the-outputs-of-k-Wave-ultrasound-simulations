#version 330

uniform sampler3D uVolume;
uniform sampler1D uColormap;
uniform sampler2D uSlice;
uniform sampler2D uBoxSampler;

uniform bool uFrame;
uniform bool uSlices;
uniform bool uXYBorder;
uniform bool uXZBorder;
uniform bool uYZBorder;
uniform bool uVolumeRenderingBack;
uniform bool uVolumeRendering;

uniform bool uTrim;

uniform int uSteps;

uniform vec4 uFrameColor;

uniform float uWidth;
uniform float uHeight;

uniform vec4 uColor;

uniform float uMin;
uniform float uMax;

in vec2 vTextureCoord;
in vec3 vTextureCoordBox;

out vec4 outColor;

void trim(float value) {
    if (uTrim) {
        if (uMin <= uMax) {
            if (value > uMax) discard;
            if (value < uMin) discard;
        } else {
            if (value < uMax) discard;
            if (value > uMin) discard;
        }
    }
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
        vec3 endPoint = vec3(texture(uBoxSampler, coords));
        vec3 startPoint = vTextureCoordBox.stp;

        vec3 path = normalize(endPoint - startPoint) / uSteps;
        float pathDepth = length(path);
        float depth = distance(startPoint, endPoint);
        vec3 point = startPoint;
        float texel = 0.0f;
        float depthC = 0.0f;
        int count = 0;
        //outColor = vec4(vec3(depth), 1.0f);
        //outColor = vec4(endPoint, 1.0f);
        //return;

        while (abs(depthC) < depth) {
            texel = max(texture(uVolume, point).r, texel);
            //texel += texture(uVolume, point).r;
            //if (texel > uMin)
            //    break;
            point += path;
            depthC += pathDepth;
            count++;
        }
        //texel = texel / count;
        trim(texel);
        texel = ((texel - uMin) * 1.0f) / (uMax - uMin);
        vec4 pixel = texture(uColormap, texel);
        float aplha = ((dot(pixel.rgb, uColor.rgb))) / (dot(pixel.rgb, vec3(1.0f)));
        outColor = vec4(pixel.rgb, aplha);

    } else {
        outColor = vec4(0.5f, 0.5f, 0.5f, 1.0f);
    }
}
