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

varying vec4 vPosition;
varying vec4 vPositionL;
varying vec4 vPositionF;

uniform float uMin;
uniform float uMax;

void main() {
   if (uFrame == 0.0) {
       vec4 vPositionM;
       float xT = (uWidthDepthRatio - 1) / 2;
       float xRatio = uWidthDepthRatio;
       float yT = (uHeightDepthRatio - 1) / 2;
       float yRatio = uHeightDepthRatio;
       if (vPosition.x > 1.0 + xT || vPosition.x < 0.0 - xT) discard;
       if (vPosition.y > 1.0 + yT || vPosition.y < 0.0 - yT) discard;
       if (vPosition.z > 1.0 || vPosition.z < 0.0) discard;
       vec4 color = texture3D(uSampler, vec3((vPositionL.x + xT) / xRatio, (vPositionL.y + yT) / yRatio, vPositionL.z));
       vec4 color2;

       //for (float s = 0.0; s <= 1.0; s += 0.25) {
       vPositionM = mix(vPositionL, vPosition, 0.5);
       color2 = texture3D(uSampler, vec3((vPositionM.x + xT) / xRatio, (vPositionM.y + yT) / yRatio, vPositionM.z));
       color = mix(color, color2, 0.5);
       color2 = texture3D(uSampler, vec3((vPosition.x + xT) / xRatio, (vPosition.y + yT) / yRatio, vPosition.z));
       color = mix(color, color2, 0.5);
       //}
       //if (vPosition.y > 1)
           //gl_FragColor = vec4(1.0, 0, 0, 0.5 );
       //else
       float value = ((color.r - uMin) * 1.0) / (uMax - uMin);
       vec4 fColor = texture1D(uColormapSampler, value);
       //float red = fColor.r / 255.0;
       //float green = fColor.g / 255;
       //float blue = fColor.b / 255.0;
       //gl_FragColor = vec4(red, green, blue, uAlpha + red * uRed + green * uGreen +  blue * uBlue);*/
       //gl_FragColor = vec4(red, green, blue, 0.5);
       //if (color.r > 500)
       gl_FragColor = vec4(fColor.rgb, uAlpha + fColor.r * uRed + fColor.g * uGreen +  fColor.b * uBlue);
       //else
           //gl_FragColor = vec4(fColor.rgb, 0.15);

   } else {
       gl_FragColor = vec4(0.5, 0.5, 0.5, 0.5);
   }
};
