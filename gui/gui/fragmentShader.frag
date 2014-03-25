#version 150

#define highp
#define mediump
#define lowp

uniform float uFrame;
uniform sampler3D uSampler;
uniform float uHeightDepthRatio;
uniform float uWidthDepthRatio;
uniform float uAlpha;
uniform float uRed;
uniform float uGreen;
uniform float uBlue;

uniform sampler1D uColormapSampler;

in vec4 vPosition;
in vec4 vPositionL;
in vec4 vPositionF;

uniform float uMin;
uniform float uMax;

uniform float uSlices;

uniform sampler2D uSliceSampler;

in vec2 vTextureCoord;

out vec4 colorOut;

void main() {
   if (uFrame) {

       colorOut = vec4(1.0, 1.0, 0.0, 1.0);

   } else if (uSlices) {

       vec4 color = texture(uSliceSampler, vTextureCoord);
       float value = ((color.r - uMin) * 1.0) / (uMax - uMin);
       vec4 fColor = texture(uColormapSampler, value);

       colorOut = vec4(fColor.rgb, 0.5);

   } else {

       float xT = (uWidthDepthRatio - 1) / 2;
       float xRatio = uWidthDepthRatio;
       float yT = (uHeightDepthRatio - 1) / 2;
       float yRatio = uHeightDepthRatio;

       if (vPosition.x > 1.0 + xT || vPosition.x < 0.0 - xT) discard;
       if (vPosition.y > 1.0 + yT || vPosition.y < 0.0 - yT) discard;
       if (vPosition.z > 1.0 || vPosition.z < 0.0) discard;

       //vec4 colorF = texture(uSampler, vPosition.xyz + (vPositionF.xyz));
       vec4 color = texture(uSampler, vec3((vPosition.x + xT) / xRatio, (vPosition.y + yT) / yRatio, vPosition.z));

       //color = mix(color, colorF, 0.5);

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
       float value = ((color.r - uMin) * 1.0) / (uMax - uMin);
       vec4 fColor = texture(uColormapSampler, value);
       //float red = fColor.r / 255.0;
       //float green = fColor.g / 255;
       //float blue = fColor.b / 255.0;
       //gl_FragColor = vec4(red, green, blue, uAlpha + red * uRed + green * uGreen +  blue * uBlue);*/
       //gl_FragColor = vec4(red, green, blue, 0.5);
       //if (color.r > 500)
       colorOut = vec4(fColor.rgb, uAlpha/* + fColor.r * uRed + fColor.g * uGreen +  fColor.b * uBlue*/);
       //else
           //gl_FragColor = vec4(fColor.rgb, 0.15);
   }
}
