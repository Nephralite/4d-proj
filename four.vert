#version 330
layout (location = 0) in vec4 pos;
layout (location = 1) in vec3 aColour;
//4d inputs
uniform vec4 from;
uniform vec4 to;
uniform vec4 up;
uniform vec4 over;
uniform float vAngle;
//3d inputs
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
//output
out vec3 vertexColour;

//4d cross product as defined by "Four-Space Visualization of 4D Objects"
vec4 cross4(vec4 a, vec4 b, vec4 c) {
    vec4 outp;
    float h, i, j, k, l, m;
    // Calculate intermediate values.
    h = (a[0] * b[1]) - (a[1] * b[0]);
    i = (a[0] * b[2]) - (a[2] * b[0]);
    j = (a[0] * b[3]) - (a[3] * b[0]);
    k = (a[1] * b[2]) - (a[2] * b[1]);
    l = (a[1] * b[3]) - (a[3] * b[1]);
    m = (a[2] * b[3]) - (a[3] * b[2]);
    // Calculate the result-vector components.
    outp =   vec4((c[1] * m) - (c[2] * l) + (c[3] * k), - (c[0] * m) + (c[2] * j) - (c[3] * i),
                (c[0] * l) - (c[1] * j) + (c[3] * h), - (c[0] * k) + (c[1] * i) - (c[2] * h));
    return outp;
}

//calculates the viewing transformation matrix
mat4 viewMatrix4(vec4 from, vec4 to, vec4 up, vec4 over) {
    vec4  d = normalize(from - to);
    vec4  a = normalize(cross4(up, over, d));
    vec4  b = normalize(cross4(over, d, a));
    vec4  c = cross4 (d, a, b);
    return mat4(a, b, c, d);
}

//this projection 3 only needs to work for a single vertex
vec3 projection3 (bool parallel, float radius, vec4 pos, float vAngle, mat4 viewMatrix, vec4 from) {
    float  d;    // Divisor Value
    vec3 outp;
    vec4 v = pos - from;
    if (parallel) {
        d = 1 / radius;
    } else {
        d = 1 / tan(vAngle * 0.5f) / dot(v, viewMatrix[3]);
    }
    outp[0] = d * dot(v, viewMatrix[0]);
    outp[1] = d * dot(v, viewMatrix[1]);
    outp[2] = d * dot(v, viewMatrix[2]);
    return outp;
}

void main() {
    vec3 pos3;
    pos3 = projection3(false, 1.0f, pos, vAngle, viewMatrix4(from, to, up, over), from);
    gl_Position = projection * view * model * vec4(pos3, 1.0);
    vertexColour = aColour;
}