#version 150 core

uniform mat4 	projectionMatrix;
uniform mat4 	viewMatrix;
uniform mat4 	modelMatrix;
uniform float 	rotationValue;
uniform vec3	rotationAxis;

in vec3	in_Position;
in vec3 in_Normal;
in vec3 in_UV0;
in vec3 in_UV1;
in vec3 in_Tangent;         		//< Tangent (object space)
in vec3 in_Bitangent;       		//< Bitangent (object space)

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

	// Pass uvs
    passUVs0 = in_UV0;
    passUVs1 = in_UV1;

    // Get rotation normal
    vec3 nnormal = normalize(in_Normal);
    vec3 rot_normal = normalize(cross(nnormal, rotationAxis));

    // Get angle to rotate towards 
    float angle = acos(dot(nnormal, nnormal *-1.0)) * rotationValue;

    // Rotate normal towards tangent
	mat3 rotationMatrix = mat3(rotationMatrix(rot_normal, angle));
	nnormal = rotationMatrix * nnormal;

	// Pass normal in object space
	passNormal = nnormal;

	// Pass position in object space
	passPosition = in_Position;

	// Pass model matrix for calculations
	passModelMatrix = modelMatrix;
}