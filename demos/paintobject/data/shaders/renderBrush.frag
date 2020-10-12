#version 450 core

// vertex shader input  
in vec3 passUVs;						//< frag Uv's
in vec3 passNormal;						//< frag normal in world space
in vec3 passPosition;					//< frag world space position 
in vec4 passColor;						//< frag color


// uniform buffer inputs
uniform UBO
{
	uniform float inSoftness;
	uniform float inFalloff;
} ubo;

// output
out vec4 out_Color;

void main() 
{
	vec2 uv_center 			= vec2(0.5, 0.5);
	float dist				= distance(uv_center, passUVs.xy) * 2;
	float intensity			= dist - ubo.inFalloff;
	intensity 				= clamp(intensity, 0, 1);
	intensity				= pow(1.0 - intensity, ubo.inSoftness);

	if (passUVs.x <= 0.05 || passUVs.y <= 0.05 || passUVs.x >= 0.95 || passUVs.y >= 0.95)
    	intensity = 0;

	vec4 out_col 			= vec4(intensity, intensity, intensity, 1);

	out_Color = out_col;
}
