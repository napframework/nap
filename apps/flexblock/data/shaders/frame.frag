#version 330

// vertex shader input
in vec3 passPosition;					//< frag position in object space
in mat4 passModelMatrix;				//< modelMatrix

// uniform inputs
uniform vec3 	inCameraPosition;		//< Camera World Space Position
uniform vec3 	inColor;				//< Color Of Block

//
out vec4 out_Color;

const vec3		lightPos = vec3(0.0, 15.0, 5.0);
const float 	lightIntensity = 1.0;
const float 	specularIntensity = 0.5;
const vec3  	specularColor = vec3(1.0, 1.0, 1.0);
const float 	shininess = 2;
const float 	ambientIntensity = 0.5f;

void main()
{
	out_Color =  vec4(inColor.rgb, 1.0);
}
