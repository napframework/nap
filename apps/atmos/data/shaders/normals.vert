#version 150 core

uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;
uniform mat4 modelMatrix;
uniform vec3 cameraPosition;		//< Camera World Space Position
uniform float time;
uniform float noiseSpeed;
uniform float noiseScale;
uniform float noiseFreq;
uniform float noiseRandom;
uniform float randomLength;
uniform float normalScale;

in vec3	in_Position;				//< Vertex position
in vec3 in_UV0;			        //< First uv coordinate set
in vec3 in_UV1;						  //< Second uv coordinate set
in float in_Tip;					  //< If the vertex is a tip or not
in vec3 in_Normal;					//< Normal

out float passTip;
out vec3 passUVs0;					//< The unwrapped normalized texture
out vec3 passUVs1;					//< The polar unwrapped texture
out vec3 passNormal;				//< vertex normal in object space
out vec3 passPosition;		  //< vertex position in object space
out mat4 passModelMatrix;		//< model matrix

// Simplex 2D noise
vec3 permute(vec3 x) { return mod(((x*34.0)+1.0)*x, 289.0); }

float snoise(vec2 v){
  const vec4 C = vec4(0.211324865405187, 0.366025403784439,
           -0.577350269189626, 0.024390243902439);
  vec2 i  = floor(v + dot(v, C.yy) );
  vec2 x0 = v -   i + dot(i, C.xx);
  vec2 i1;
  i1 = (x0.x > x0.y) ? vec2(1.0, 0.0) : vec2(0.0, 1.0);
  vec4 x12 = x0.xyxy + C.xxzz;
  x12.xy -= i1;
  i = mod(i, 289.0);
  vec3 p = permute( permute( i.y + vec3(0.0, i1.y, 1.0 ))
  + i.x + vec3(0.0, i1.x, 1.0 ));
  vec3 m = max(0.5 - vec3(dot(x0,x0), dot(x12.xy,x12.xy),
    dot(x12.zw,x12.zw)), 0.0);
  m = m*m ;
  m = m*m ;
  vec3 x = 2.0 * fract(p * C.www) - 1.0;
  vec3 h = abs(x) - 0.5;
  vec3 ox = floor(x + 0.5);
  vec3 a0 = x - ox;
  m *= 1.79284291400159 - 0.85373472095314 * ( a0*a0 + h*h );
  vec3 g;
  g.x  = a0.x  * x0.x  + h.x  * x0.y;
  g.yz = a0.yz * x12.xz + h.yz * x12.yw;
  return 130.0 * dot(m, g);
}


float random (vec2 st) 
{
  return fract(sin(dot(st.xy, vec2(12.9898,78.233)))*43758.5453123);
}


float fit(float value, float min, float max, float outMin, float outMax)
{
  float v = clamp(value, min, max);
  float m = max - min;
  if(m==0.0)
    m = 0.00000001;
  return (v - min) / (m) * (outMax - outMin) + outMin;
}

void main(void)
{
  // Current noise sample time
	float current_time = time * noiseSpeed;

  // Seed for noise is the 2nd uv texture coordinate
	vec2 noise_lookup = vec2(in_UV1.x, in_UV1.y);

  // Add some random noise lookup
  vec2 rand_lookup = vec2(
    fit(random(noise_lookup + vec2(0.05, 0.075)),0.0,1.0,-1.0,1.0), 
    fit(random(noise_lookup + vec2(0.10, 0.200)),0.0,1.0,-1.0,1.0));
  rand_lookup = rand_lookup * noiseRandom;

	// calculate noise to offset normal
	float ox = snoise(((noise_lookup * noiseFreq)) + rand_lookup + 00.0 + current_time) * noiseScale;
	float oy = snoise(((noise_lookup * noiseFreq)) + rand_lookup + 10.0 + current_time) * noiseScale;
	float oz = snoise(((noise_lookup * noiseFreq)) + rand_lookup + 20.0 + current_time) * noiseScale;

  // Prepare some variables used for calculating alternative normal
  vec3 nnormal = normalize(in_Normal);
  float normal_length = length(in_Normal);

	// Add noise value to normal
	vec3 displaced_normal = normalize(nnormal + vec3(ox, oy, oz));

	// Calculate point origin
	vec3 point_origin = in_Position + (-1.0 * in_Normal);
	
  // Get random length number
  float length = mix(normal_length, length(in_Normal) * random(noise_lookup), randomLength);
  length = length * normalScale;

  // Calculate displaced point position
  vec3 displ_position = point_origin + (displaced_normal * length);

  // Only displace tip of the line
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