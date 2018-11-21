#version 330

// uniform inputs
uniform vec3 color;

// output
out vec4 out_Color;

void main() 
{
    out_Color = vec4(color, 1.0);
}
