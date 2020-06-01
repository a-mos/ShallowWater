#version 430

layout(local_size_x = 32, local_size_y = 32, local_size_z = 1) in;
layout(binding = 0) buffer PREV {float H_prev[];};
layout(binding = 1) buffer CURR {float H_curr[];};
layout(binding = 2) buffer NEXT {float H_next[];};
layout(binding = 3) buffer POSB {vec4 POS[];};
layout(binding = 4) buffer NRMB {vec4 NRM[];};

uniform int SIZE;

int clip(int val, int min, int max) {
    if (val >= max) {
        return max;
    }
    if (val <= min) {
        return min;
    }
    return val;
}

void main () {
    int i = clip(int(gl_GlobalInvocationID.x), 0, SIZE);
    int j = clip(int(gl_GlobalInvocationID.y), 0, SIZE);
    if (i == 0 || j == 0 || i == SIZE - 1 || j == SIZE - 1) {
       H_next[j * SIZE + i] = 0.0f;
       return;
    }
    H_next[j * SIZE + i] = (1 - 1.985f) * H_prev[j * SIZE + i]
                 + 1.985f * 0.25f * (
                 H_curr[j * SIZE + i - 1] +
                 H_curr[j * SIZE + i + 1] +
                 H_curr[(j + 1) * SIZE + i] +
                 H_curr[(j - 1) * SIZE + i]);
    POS[j * SIZE + i].z = H_curr[j * SIZE + i];
    NRM[j * SIZE + i].x = H_curr[j * SIZE + i - 1] - H_curr[j * SIZE + i + 1];
    NRM[j * SIZE + i].y = H_curr[(j - 1) * SIZE + i] - H_curr[(j + 1) * SIZE + i];
}
