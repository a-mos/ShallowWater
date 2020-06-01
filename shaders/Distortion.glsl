#version 430

layout(local_size_x = 32, local_size_y = 32, local_size_z = 1) in;
layout(binding = 0) buffer PREV {float H_prev[];};
layout(binding = 1) buffer CURR {float H_curr[];};
layout(binding = 2) buffer NEXT {float H_next[];};
layout(binding = 3) buffer POSB {vec4 POS[];};
layout(binding = 4) buffer NRMB {vec4 NRM[];};

uniform int SIZE;
uniform float DISTORTION_VAL;
uniform ivec2 DISTORTION_POS;

void main() {
    for (int x = -2; x < 3; x++) {
        for (int y = -2; y < 3; y++) {
            if ((DISTORTION_POS.x + x > SIZE - 1) || (DISTORTION_POS.y + y > SIZE - 1) || (DISTORTION_POS.y + y < 0) || (DISTORTION_POS.x + x < 0)) {
                break;
            }
            H_curr[(DISTORTION_POS.y + y) * SIZE + (DISTORTION_POS.x + x)] -=  (9.0 - x * x - y * y) * DISTORTION_VAL;
        }
    }
}
