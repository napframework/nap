#version 150 core

uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;
uniform mat4 modelMatrix;
uniform float blendValue;
uniform float normalBlendValue;
uniform vec3 cameraPosition;		//< Camera World Space Position
uniform float time;

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

//	Simplex 3D Noise 
//	by Ian McEwan, Ashima Arts
//
vec4 permute(vec4 x){return mod(((x*34.0)+1.0)*x, 289.0);}
vec4 taylorInvSqrt(vec4 r){return 1.79284291400159 - 0.85373472095314 * r;}

float snoise(vec3 v){ 
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

  //  x0 = x0 - 0. + 0.0 * C 
  vec3 x1 = x0 - i1 + 1.0 * C.xxx;
  vec3 x2 = x0 - i2 + 2.0 * C.xxx;
  vec3 x3 = x0 - 1. + 3.0 * C.xxx;

// Permutations
  i = mod(i, 289.0 ); 
  vec4 p = permute( permute( permute( 
             i.z + vec4(0.0, i1.z, i2.z, 1.0 ))
           + i.y + vec4(0.0, i1.y, i2.y, 1.0 )) 
           + i.x + vec4(0.0, i1.x, i2.x, 1.0 ));

// Gradients
// ( N*N points uniformly over a square, mapped onto an octahedron.)
  float n_ = 1.0/7.0; // N=7
  vec3  ns = n_ * D.wyz - D.xzx;

  vec4 j = p - 49.0 * floor(p * ns.z *ns.z);  //  mod(p,N*N)

  vec4 x_ = floor(j * ns.z);
  vec4 y_ = floor(j - 7.0 * x_ );    // mod(j,N)

  vec4 x = x_ *ns.x + ns.yyyy;
  vec4 y = y_ *ns.x + ns.yyyy;
  vec4 h = 1.0 - abs(x) - abs(y);

  vec4 b0 = vec4( x.xy, y.xy );
  vec4 b1 = vec4( x.zw, y.zw );

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
  return 42.0 * dot( m*m, vec4( dot(p0,x0), dot(p1,x1), 
                                dot(p2,x2), dot(p3,x3) ) );
}

void main(void)
{
	float time_scale = 0.25;
	float current_time = time * time_scale;

	vec3 noise_lookup = vec3(in_UV1.x, in_UV1.y, 0.0);
	float scale = 0.5;

	// calculate noise to offset normal
	float ox = snoise((noise_lookup * 10.0) + 00.0 + current_time) * scale;
	float oy = snoise((noise_lookup * 10.0) + 10.0 + current_time) * scale;
	float oz = snoise((noise_lookup * 10.0) + 20.0 + current_time) * scale;

	// Add noise value to normal
	vec3 displaced_normal = normalize(in_Normal + vec3(ox, oy, oz));

	// Calculate point origin
	vec3 point_origin = in_Position + (-1.0 * (normalize(in_Normal) * 0.01));
	vec3 displ_position = point_origin + (displaced_normal * 0.01);

	vec3 final_pos = mix(in_Position, displ_position, 1.0-in_Tip);

	// Calculate position
    gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(final_pos, 1.0);

	// Pass color
	passTip = in_Tip;

	// Pass uvs
	passUVs0 = in_UV0;
    passUVs1 = in_UV1;

	// Pass normal in object space
	passNormal = displaced_normal;

	// Pass position in object space
	passPosition = final_pos;

	// Pass model matrix for calculations
	passModelMatrix = modelMatrix;
}