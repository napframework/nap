#version 450 core

// vertex shader input  
in vec3 passUVs;						//< frag Uv's
in vec3 passNormal;						//< frag normal in world space
in vec3 passPosition;					//< frag world space position 
in vec4 passColor;						//< frag color

// uniform buffer inputs
uniform UBO
{
	uniform vec4 	inBrushColor; 				//< color of brush
	uniform vec2 	inMousePosition;			//< mouse position in uv space
	uniform float 	inBrushSize;				//< size of brush relative to uv space
	uniform float 	inAlphaMultiplier;			//< alpha multiplier
	uniform float 	inEraserAmount;
} ubo;

// unfiorm sampler inputs 			
uniform sampler2D inMainTexture;		//< Main Texture
uniform sampler2D inBrushTexture;		//< Brush Texture

// output
out vec4 out_Color;

void main() 
{
	vec4 main_col 			= texture(inMainTexture, passUVs.xy);
	vec2 distance_to_mouse 	= vec2(ubo.inMousePosition.x - passUVs.x, ubo.inMousePosition.y - passUVs.y ) ; 
	vec2 uv_center 			= vec2(0.5, 0.5);
	vec4 brush_col			= texture(inBrushTexture, uv_center + distance_to_mouse / ubo.inBrushSize);
	vec4 out_col 			= mix(main_col, ubo.inBrushColor, brush_col.r);

	brush_col.r 			-= brush_col.r * ubo.inEraserAmount * 2;
	out_col.a 				= clamp(brush_col.r + main_col.a, 0, 1 ) ;
	out_col 				*= ubo.inAlphaMultiplier;

	out_Color = out_col;
}
