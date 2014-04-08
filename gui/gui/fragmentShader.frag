#version 150

#define highp
#define mediump
#define lowp

uniform float uFrame;

uniform sampler3D uSampler;
uniform float uAlpha;
uniform float uRed;
uniform float uGreen;
uniform float uBlue;

uniform sampler1D uColormapSampler;

in vec4 vPosition;
//in vec4 vPositionL;
//in vec4 vPositionF;

uniform float uWidth;
uniform float uHeight;
uniform float uDepth;

uniform float uMin;
uniform float uMax;

uniform float uSlices;

uniform float uTrim;

uniform sampler2D uSliceSampler;

in vec2 vTextureCoord;

out vec4 colorOut;

void main() {
    if (uFrame) {

        colorOut = vec4(0.5, 0.5, 0.5, 1.0);

    } else if (uSlices) {
        vec4 color = texture(uSliceSampler, vTextureCoord);
        if (uTrim) {
            if (color.r > uMax) discard;
            if (color.r < uMin) discard;
        }

        /*if (color.r < 0) color.r = abs(color.r);
        if (color.r == 0) color.r = 0.00000001;

        float maxN = uMax;
        if (maxN < 0) maxN = abs(maxN);
        if (maxN == 0) maxN = 0.00000001;

        float minN = uMin;
        if (minN < 0) minN = abs(minN);
        if (minN == 0) minN = 0.00000001;

        float maxL = 0;
        float valueL = 20 * log(color.r / maxN);
        float minL = 20 * log(minN / maxN);*/
        //float value = ((valueL - minL) * 1.0) / (maxL - minL);
        float value = ((color.r - uMin) * 1.0) / (uMax - uMin);
        vec4 fColor = texture(uColormapSampler, value);

        colorOut = vec4(fColor.rgb, 0.9);

    } else {

        if (vPosition.x > 0.5f + uWidth / 2.0f || vPosition.x < 0.5f - uWidth / 2.0f) discard;
        if (vPosition.y > 0.5f + uHeight / 2.0f || vPosition.y < 0.5f - uHeight / 2.0f) discard;
        if (vPosition.z > 0.5f + uDepth / 2.0f || vPosition.z < 0.5f - uDepth / 2.0f) discard;

        float xT = (uWidth - 0.5f);
        float yT = (uHeight - 0.5f);
        float zT = (uDepth - 0.5f);

        //vec4 vector = mix(vPosition, vPositionL, 0.5);

        //vec4 new = normalize(vPositionF) * distance(vPosition, vPositionL);

        //vec4 vPositionN = vec4(vPositionL.xy, 0.0, vPositionL.w);

        //vec4 colorL = texture(uSampler, vec3((vPositionN.x + xT) / xRatio, (vPositionN.y + yT) / xRatio, vPositionN.z));
        vec4 color = texture(uSampler, vec3((vPosition.x + xT) / uWidth - 0.5f, (vPosition.y + yT) / uHeight - 0.5f, (vPosition.z + zT) / uDepth - 0.5f));

        //vec4 color = colorL;

        /*for (float s = 0.0; s <= 1.0; s += 0.1) {
           vec4 vPositionM = mix(vPositionN, vPositionL, s);
           vec4 colorM = texture(uSampler, vec3((vPositionM.x + xT) / xRatio, (vPositionM.y + yT) / yRatio, vPositionM.z));
           //color.r = color.r * (1 - uAlpha) + colorM.r * uAlpha;
           color.r = max(color.r, colorM.r);
        }*/
        //vec4 color2 = texture(uSampler, vec3((vPositionL.x + xT) / xRatio, (vPositionL.y + yT) / yRatio, vPositionL.z - 0.5));

        //color = mix(colorL, color2, 0.5);

        /* vec4 colorN;// = mix(color, colorM, 0.9);
        for (float s = 0.0; s <= 1.0; s += 0.5) {
           vec4 vPositionM = mix(vPositionL, vPosition0, s);
           vec4 colorM = texture3D(uSampler, vec3((vPositionM.x + xT) / xRatio, (vPositionM.y + yT) / yRatio, vPositionM.z));
           //colorL.r = colorL.r * (1 - uAlpha) + colorM.r * uAlpha;
           colorL.r = mix(colorL.r, colorM.r, 0.5);
           //colorL.g = colorL.g * (1 - uAlpha) + colorM.g * uAlpha;
           //colorL.b = colorL.b * (1 - uAlpha) + colorM.b * uAlpha;
        }*/
        //if (vPosition.y > 1)
           //gl_FragColor = vec4(1.0, 0, 0, 0.5 );
        //else

        if (uTrim) {
            if (color.r > uMax) discard;
            if (color.r < uMin) discard;
        }

        float value = ((color.r - uMin) * 1.0) / (uMax - uMin);
        vec4 fColor = texture(uColormapSampler, value);
        //float red = fColor.r / 255.0;
        //float green = fColor.g / 255;
        //float blue = fColor.b / 255.0;
        //gl_FragColor = vec4(red, green, blue, uAlpha + red * uRed + green * uGreen +  blue * uBlue);*/
        //gl_FragColor = vec4(red, green, blue, 0.5);
        //if (color.r > 500)
        colorOut = vec4(fColor.rgb, uAlpha + fColor.r * uRed + fColor.g * uGreen +  fColor.b * uBlue);
        //else
           //gl_FragColor = vec4(fColor.rgb, 0.15);
    }
}
