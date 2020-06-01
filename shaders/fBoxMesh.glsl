#version 330

uniform sampler2D Texture;
in vec2 texCoord;
out vec4 color;

void main () {
  vec3 ambient = vec3(1.0);
  vec3 res = ambient * vec3(texture(Texture, texCoord));
  color = vec4(res, 1.0);
}