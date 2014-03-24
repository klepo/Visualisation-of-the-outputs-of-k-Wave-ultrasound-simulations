#define highp
#define mediump
#define lowp

attribute vec4 aPosition;
uniform mat4 uMatrix;
uniform mat4 uScaleMatrix;
uniform mat4 uScalelMatrix;
varying vec4 vPosition;
varying vec4 vPositionL;
varying vec4 vPositionF;

void main() {
   vPosition = uScaleMatrix * aPosition;
   gl_Position = uMatrix * uScaleMatrix * aPosition;
   vPositionL = uScalelMatrix * aPosition;
   vPositionF = gl_Position;
};
