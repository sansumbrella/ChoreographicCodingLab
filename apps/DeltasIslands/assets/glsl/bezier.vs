#version 330

uniform mat4 ciModelViewProjection;
uniform vec4 uColor = vec4(1.0);
uniform mat3 ciNormalMatrix;

uniform vec3 uLightDirection = vec3(0, 1, 0);
uniform vec3 uGroundColor = vec3(0.4);
uniform vec3 uSkyColor = vec3(1.0);

uniform float uDepthCutoff = 500.0;
uniform float uDepthFalloff = 2.4;

uniform float uT = 0.5;

// Bezier positions
in vec3 A, B, C, D;
in float CurveWeight;
in vec3 NormalBegin, NormalEnd;

in mat4 Transform;
in float Activation;

smooth out float vHeight;

vec4 hemisphereColor(vec3 normal)
{
	vec3 light_dir = normalize(uLightDirection);
	float cos_theta = dot(normal, light_dir);
	float a = cos_theta * 0.5 + 0.5;

	return vec4(mix(uGroundColor, uSkyColor, a), 1.0);
}

vec4 depth(vec4 position)
{
	float z = clamp(vec4(ciModelViewProjection * position).z, 0.0, uDepthCutoff);
	float a = pow(1.0 - z / uDepthCutoff, uDepthFalloff);
	return vec4( vec3( a ), 1.0 );
}

void main()
{
	float t = Activation * CurveWeight;
	vec3 ab = mix(A, B, t);
	vec3 bc = mix(B, C, t);
	vec3 cd = mix(C, D, t);
	vec4 position = Transform * vec4(mix(mix(ab, bc, t), mix(bc, cd, t), t), 1.0);
	vec3 normal = normalize(ciNormalMatrix * mat3(Transform) * mix(NormalBegin, NormalEnd, t));

  vHeight = clamp(position.y / 1.0, 0.0, 1.0);
//	vColor = hemisphereColor(normal) * uColor * depth(position);
//  vColor = vec4(1.0, 1.0, 0.0, 1.0);
	gl_Position = ciModelViewProjection * position;
}
