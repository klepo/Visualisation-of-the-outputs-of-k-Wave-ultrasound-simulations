#version 150

#define highp
#define mediump
#define lowp

in vec4 aPosition;
in vec2 aTextureCoord;

uniform mat4 uMatrix;
uniform mat4 uScaleMatrix;
uniform mat4 uScalelMatrix;

out vec4 vPosition;

out vec2 vTextureCoord;

void main() {
   vPosition = uScalelMatrix * aPosition;
   gl_Position = uMatrix * uScaleMatrix * aPosition;
   vTextureCoord = aTextureCoord;
}
