#version 150 core

uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;
uniform mat4 modelMatrix;
uniform float blendValue;
uniform float normalBlendValue;
uniform vec3 		cameraPosition;							//< Camera World Space Position

in vec3	in_Position;				//< Vertex position
in vec3 in_UV0;						//< First uv coordinate set
in vec3 in_UV1;						//< Second uv coordinate set
in float in_Tip;					//< If the vertex is a tip or not
in vec3 in_Normal;					//< Normal

out float passTip;
out vec3 passUVs0;					//< The unwrapped normalized texture
out vec3 passUVs1;					//< The polar unwrapped texture
out vec3 passNormal;				//< vertex normal in object space
out vec3 passPosition;				//< vertex position in object space
out mat4 passModelMatrix;			//< model matrix

mat4 rotationMatrix(vec3 axis, float angle)
{
    axis = normalize(axis);
    float s = sin(angle);
    float c = cos(angle);
    float oc = 1.0 - c;
    
    return mat4(oc * axis.x * axis.x + c,           oc * axis.x * axis.y - axis.z * s,  oc * axis.z * axis.x + axis.y * s,  0.0,
                oc * axis.x * axis.y + axis.z * s,  oc * axis.y * axis.y + c,           oc * axis.y * axis.z - axis.x * s,  0.0,
                oc * axis.z * axis.x - axis.y * s,  oc * axis.y * axis.z + axis.x * s,  oc * axis.z * axis.z + c,           0.0,
                0.0,                                0.0,                                0.0,                                1.0);
}

void main(void)
{
	// Calculate position
    gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(in_Position, 1.0);

	// Pass color
	passTip = in_Tip;

	// Pass uvs
	passUVs0 = in_UV0;
    passUVs1 = in_UV1;

    vec3 cam_normal = normalize(cameraPosition - in_Position);
    vec3 cro_normal = normalize(cross(cam_normal, in_Normal));
    float angle = acos(dot(in_Normal, cro_normal));

	mat3 rotationMatrix = mat3(rotationMatrix(in_Normal, angle));
	passNormal = cro_normal;

	// Pass normal in object space
	//vec3 cross_normal = cross(in_Normal, vec3(0.0,1.0,0.0));
	//vec3 tip_normal = mix(in_Normal, vec3(0.0,1.0,0.0), 1.0-in_Tip);

	//passNormal = mix(cross_normal, tip_normal, 0.66);

	// Pass position in object space
	passPosition = in_Position;

	// Pass model matrix for calculations
	passModelMatrix = modelMatrix;
}