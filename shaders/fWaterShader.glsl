#version 330

uniform vec3 camera;
in vec3 fragPos;
in vec3 Normal;

out vec4 color;

vec3 lightColor = vec3(1.0);
vec3 lightPos = vec3(0.0, 2.0, -3.0);

void main() {
    vec3 objectColor = vec3(0.0, 0.5, 1.0);

    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * lightColor;

    vec3 N = normalize(Normal);
    vec3 lightDir = normalize(lightPos - fragPos);

    float diff = max(dot(N, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    vec3 viewDir = normalize(camera - fragPos);
    vec3 reflectDir = reflect(-lightDir, N);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = 1.8f * spec * lightColor;

    vec3 result = (ambient + diffuse + specular) * objectColor;
    color = vec4(result, 0.7);
}