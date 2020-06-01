#version 330

layout(location = 0) in vec3 aVertex;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 fragPos;
out vec2 texCoord;

void main(void) {
  texCoord = aTexCoord;
  fragPos = aVertex;
  gl_Position = projection * view * model * vec4(fragPos, 1.0);
}
