#version 330

// vertex shader input
in vec3 passPosition;					//< frag position in object space
in mat4 passModelMatrix;				//< modelMatrix

// uniform inputs
uniform vec3 	inCameraPosition;		//< Camera World Space Position
uniform vec3 	inColor;				//< Color Of Frame

//
out vec4 out_Color;

void main()
{
	out_Color =  vec4(inColor.rgb, 1.0);
}
