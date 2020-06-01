#version 330

layout(location = 0) in vec4 aVertex;
layout(location = 1) in vec4 aNormal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 fragPos;
out vec3 Normal;

void main(void) {
  fragPos = vec3(model * aVertex);
  Normal = mat3(transpose(inverse(model))) * vec3(aNormal);
  gl_Position = projection * view * model * aVertex;
}