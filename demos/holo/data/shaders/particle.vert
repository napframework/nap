// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#version 450 core

uniform nap
{
	mat4 projectionMatrix;
	mat4 viewMatrix;
	mat4 modelMatrix;
} mvp;

uniform VERTUBO 
{
	float elapsedTime;
	float particleSize;
	float fresnelScale;
	float fresnelPower;
	vec3 cameraLocation;
	vec3 initialPositions[1024];
} vubo; 

in vec3	in_Position;
in vec3 in_Normals;

out vec3 pass_Position;
out vec3 pass_Normal;
out float pass_Fresnel;
out float pass_Id;

const float speedLimitation = 0.25;

// Forward declaration
float simplex(vec3 v);

// Translation matrix
mat4 translate(vec3 d)
{
	return mat4(
		1.0, 0.0, 0.0, 0.0,
		0.0, 1.0, 0.0, 0.0,
		0.0, 0.0, 1.0, 0.0,
		d.x, d.y, d.z, 1.0
	);
}

// Rotation matrix
mat4 rotate(vec3 axis, float angle)
{
	axis = normalize(axis);
	float s = sin(angle);
	float c = cos(angle);
	float oc = 1.0 - c;
	
	return transpose(mat4(
		oc * axis.x * axis.x + c,           oc * axis.x * axis.y - axis.z * s,  oc * axis.z * axis.x + axis.y * s,  0.0,
		oc * axis.x * axis.y + axis.z * s,  oc * axis.y * axis.y + c,           oc * axis.y * axis.z - axis.x * s,  0.0,
		oc * axis.z * axis.x - axis.y * s,  oc * axis.y * axis.z + axis.x * s,  oc * axis.z * axis.z + c,           0.0,
		0.0,                                0.0,                                0.0,                                1.0
	));
}


void main(void)
{
	uint id = gl_InstanceIndex;
	vec4 p = vec4(vubo.initialPositions[id], 1.0);

	// Sample from noise signal based on persistent random vertex positions
	vec3 simplex_pos = { 
		simplex(p.xyz * vubo.elapsedTime * speedLimitation), 
		simplex(p.zxy * vubo.elapsedTime * speedLimitation),
		simplex(p.yzx * vubo.elapsedTime * speedLimitation)
	};

	// Compute particle translation
	mat4 translation = translate(simplex_pos);

	// Compute particle rotation
	float rotation_offset = p.z;
	mat4 rotation = rotate(p.yzx, vubo.elapsedTime * 25.0 * speedLimitation + rotation_offset * 1000.0);

	// Compute transform
	mat4 particle_transform = mvp.modelMatrix * translation * rotation;

	// Vertex position after scale and particle transform
	vec4 world_position = particle_transform * vec4(vubo.particleSize * in_Position, 1.0);

	// Calculate vertex world space position and set
	pass_Position = world_position.xyz;

	// Rotate normal based on model matrix and set
	vec4 normal = vec4(in_Normals, 0.0);
	mat3 normal_matrix = transpose(inverse(mat3(particle_transform)));
	pass_Normal = normalize(normal_matrix * normal.xyz);

	// Calculate position
    gl_Position = mvp.projectionMatrix * mvp.viewMatrix * world_position;

    // Pass fresnel
	vec3 eye_to_surface = normalize(world_position.xyz - vubo.cameraLocation);
	float fresnel = 0.04 + 0.96 * pow(clamp(1.0 + dot(eye_to_surface, pass_Normal), 0.0, 1.0), vubo.fresnelPower);
	pass_Fresnel = vubo.fresnelScale * fresnel;

	// Pass texture id
	pass_Id = float(gl_InstanceIndex);
}







//
// Description : Array and textureless GLSL 2D/3D/4D simplex 
//               noise functions.
//      Author : Ian McEwan, Ashima Arts.
//  Maintainer : stegu
//     Lastmod : 20110822 (ijm)
//     License : Copyright (C) 2011 Ashima Arts. All rights reserved.
//               Distributed under the MIT License. See LICENSE file.
//               https://github.com/ashima/webgl-noise
//               https://github.com/stegu/webgl-noise
// 

float mod289(float x) { return x - floor(x * (1.0 / 289.0)) * 289.0; }
vec2 mod289(vec2 x)   { return x - floor(x * (1.0 / 289.0)) * 289.0; }
vec3 mod289(vec3 x)   { return x - floor(x * (1.0 / 289.0)) * 289.0; }
vec4 mod289(vec4 x)   { return x - floor(x * (1.0 / 289.0)) * 289.0; }

float rand(vec2 co) { return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453); }
float rand(vec2 co, float l) {return rand(vec2(rand(co), l));}

float permute(float x) { return mod289(((x*34.0)+1.0)*x); }
vec3 permute(vec3 x)   { return mod289(((x*34.0)+1.0)*x); }
vec4 permute(vec4 x)   { return mod289(((x*34.0)+1.0)*x); }

float taylorInvSqrt(float r) { return 1.79284291400159 - 0.85373472095314 * r; }
vec4 taylorInvSqrt(vec4 r)   { return 1.79284291400159 - 0.85373472095314 * r; }

float simplex(vec3 v)
{ 
	const vec2  C = vec2(1.0/6.0, 1.0/3.0) ;
	const vec4  D = vec4(0.0, 0.5, 1.0, 2.0);

	// First corner
	vec3 i  = floor(v + dot(v, C.yyy) );
	vec3 x0 =   v - i + dot(i, C.xxx) ;

	// Other corners
	vec3 g = step(x0.yzx, x0.xyz);
	vec3 l = 1.0 - g;
	vec3 i1 = min( g.xyz, l.zxy );
	vec3 i2 = max( g.xyz, l.zxy );

	// x0 = x0 - 0.0 + 0.0 * C.xxx;
	// x1 = x0 - i1  + 1.0 * C.xxx;
	// x2 = x0 - i2  + 2.0 * C.xxx;
	// x3 = x0 - 1.0 + 3.0 * C.xxx;
	vec3 x1 = x0 - i1 + C.xxx;
	vec3 x2 = x0 - i2 + C.yyy;	// 2.0*C.x = 1/3 = C.y
	vec3 x3 = x0 - D.yyy;		// -1.0+3.0*C.x = -0.5 = -D.y

	// Permutations
	i = mod289(i); 
	vec4 p = permute( permute( permute( 
		i.z + vec4(0.0, i1.z, i2.z, 1.0 ))
		+ i.y + vec4(0.0, i1.y, i2.y, 1.0 )) 
		+ i.x + vec4(0.0, i1.x, i2.x, 1.0 ));

	// Gradients: 7x7 points over a square, mapped onto an octahedron.
	// The ring size 17*17 = 289 is close to a multiple of 49 (49*6 = 294)
	float n_ = 0.142857142857; // 1.0/7.0
	vec3  ns = n_ * D.wyz - D.xzx;

	vec4 j = p - 49.0 * floor(p * ns.z * ns.z);  //  mod(p,7*7)

	vec4 x_ = floor(j * ns.z);
	vec4 y_ = floor(j - 7.0 * x_ );    // mod(j,N)

	vec4 x = x_ *ns.x + ns.yyyy;
	vec4 y = y_ *ns.x + ns.yyyy;
	vec4 h = 1.0 - abs(x) - abs(y);

	vec4 b0 = vec4( x.xy, y.xy );
	vec4 b1 = vec4( x.zw, y.zw );

	//vec4 s0 = vec4(lessThan(b0,0.0))*2.0 - 1.0;
	//vec4 s1 = vec4(lessThan(b1,0.0))*2.0 - 1.0;
	vec4 s0 = floor(b0)*2.0 + 1.0;
	vec4 s1 = floor(b1)*2.0 + 1.0;
	vec4 sh = -step(h, vec4(0.0));

	vec4 a0 = b0.xzyw + s0.xzyw*sh.xxyy ;
	vec4 a1 = b1.xzyw + s1.xzyw*sh.zzww ;

	vec3 p0 = vec3(a0.xy,h.x);
	vec3 p1 = vec3(a0.zw,h.y);
	vec3 p2 = vec3(a1.xy,h.z);
	vec3 p3 = vec3(a1.zw,h.w);

	//Normalise gradients
	vec4 norm = taylorInvSqrt(vec4(dot(p0,p0), dot(p1,p1), dot(p2, p2), dot(p3,p3)));
	p0 *= norm.x;
	p1 *= norm.y;
	p2 *= norm.z;
	p3 *= norm.w;

	// Mix final noise value
	vec4 m = max(0.6 - vec4(dot(x0,x0), dot(x1,x1), dot(x2,x2), dot(x3,x3)), 0.0);
	m = m * m;
	return 42.0 * dot( m*m, vec4( dot(p0,x0), dot(p1,x1), dot(p2,x2), dot(p3,x3) ) );
}
