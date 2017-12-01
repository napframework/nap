#version 330
 
uniform sampler2D LayerOne;
uniform sampler2D LayerTwo;
  
in vec3 pass_Uvs;
in vec3 pass_Normals;

out vec4 out_Color;

void main() 
{
  vec3 color_layer_one = texture(LayerOne, vec2(pass_Uvs.x, pass_Uvs.y)).rgb;
  vec3 color_layer_two = texture(LayerTwo, vec2(pass_Uvs.x, pass_Uvs.y)).rgb;
  
  // Get greyscale value
  float greyscale = (color_layer_two.r + color_layer_two.g + color_layer_two.b) / 3.0;
  vec3 color = color_layer_one;
  if(greyscale > 0.001)
  {
		color = color_layer_two;
  }

  out_Color = vec4(color, 1.0);
}
