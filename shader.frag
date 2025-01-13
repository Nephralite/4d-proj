#version 430
in vec3 vertexColour;
out vec4 FragColour;
void main() {
    FragColour = vec4(vertexColour, 1);
}
