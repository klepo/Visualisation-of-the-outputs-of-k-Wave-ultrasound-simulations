#version 120

#define highp
#define mediump
#define lowp

attribute vec4 aPosition;
attribute vec2 aTextureCoord;

uniform mat4 uMatrix;
uniform mat4 uScaleMatrix;
uniform mat4 uScalelMatrix;

varying vec4 vPosition;
//out vec4 vPositionL;

varying vec2 vTextureCoord;

void main() {
   vPosition = uScalelMatrix * aPosition;
   gl_Position = uMatrix * uScaleMatrix * aPosition;
   vTextureCoord = aTextureCoord;
   //vPositionL = uScaleMatrix * aPosition;
}
