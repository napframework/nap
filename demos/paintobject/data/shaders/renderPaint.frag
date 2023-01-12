/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#version 450 core

// vertex shader input  
in vec3 passUVs;						//< frag Uv's
in vec3 passNormal;						//< frag normal in world space
in vec3 passPosition;					//< frag world space position 
in vec4 passColor;						//< frag color

// uniform buffer inputs
uniform UBO
{
	vec4 	inBrushColor; 				//< color of brush
	vec2 	inMousePosition;			//< mouse position in uv space
	float 	inBrushSize;				//< size of brush relative to uv space
	float 	inFinalMultiplier;			//< final multiplier
	float 	inEraserAmount;				//< eraser amount, when 1, brush will clear paint
} ubo;

// unfiorm sampler inputs 			
uniform sampler2D inMainTexture;		//< Main Texture
uniform sampler2D inBrushTexture;		//< Brush Texture

// output
out vec4 out_Color;

void main() 
{
	// get fragment color from texture
	vec4 tex_col 			= texture(inMainTexture, passUVs.xy);

	// calculate distance to mouse mouse
	vec2 distance_to_mouse 	= vec2(ubo.inMousePosition.x - passUVs.x, ubo.inMousePosition.y - passUVs.y ); 

	// lookup brush color from center of brush using mouse distance and brush size
	vec4 brush_col			= texture(inBrushTexture, vec2(0.5, 0.5) + distance_to_mouse / ubo.inBrushSize);

	// mix in brush color with texture color based on whiteness of the brush
	vec4 out_col 			= mix(tex_col, ubo.inBrushColor, brush_col.r);

 	// eraser amount clears any color in the paint texture
	brush_col.r 			-= brush_col.r * ubo.inEraserAmount * 2;

	// determine alpha by adding whiteness plus stored alpha in texture
	out_col.a 				= clamp(brush_col.r + tex_col.a, 0, 1) ;

	// multiply the final color with the final multiplier
	// when the final multiplier is 0, the paint texture will be cleared
	// this is useful for cleaning all the paint in one pass
	out_col 				*= ubo.inFinalMultiplier;

	// finally, set the color based on mix value
	out_Color = out_col;
}
