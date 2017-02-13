#version 330

in vec3 aPosition;

uniform mat4 uMatrix;
uniform mat4 uSliceMatrix;

out vec2 vTextureCoord;
out vec3 vTextureCoordBox;

void main() {
   vTextureCoordBox = (uSliceMatrix * vec4(aPosition, 1.0f)).xyz;
   //vTextureCoord = vec2(aPosition.x, 1 - aPosition.y);
   vTextureCoord = aPosition.xy;
   gl_Position = uMatrix * vec4(aPosition, 1.0f);
}
