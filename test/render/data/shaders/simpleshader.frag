#version 450
#extension GL_ARB_separate_shader_objects : enable

in vec3 fragColor;

out vec4 outColor;

void main() {
    outColor = vec4(fragColor, 1.0);
}