#version 330

uniform mat4 ciModelViewProjection;
uniform mat4 ciModelViewMatrix;
uniform mat4 ciModelMatrix;
uniform mat3 ciNormalMatrix;

uniform vec3 uLightDirection = vec3(0, 1, 0);
uniform vec3 uGroundColor = vec3(0.12, 0.05, 0.05);
uniform vec3 uSkyColor = vec3(0.8, 0.9, 1.0);
uniform vec4 uColor = vec4(1.0);

uniform float uDepthCutoff = 500.0;
uniform float uDepthFalloff = 2.4;

in vec4 ciPosition;
in vec3 ciNormal;

smooth out vec4 vColor;

vec4 hemisphereColor(vec3 normal)
{
	vec3 light_dir = normalize(uLightDirection);
	float cos_theta = dot(normal, light_dir);
	float a = cos_theta * 0.5 + 0.5;

	return vec4(mix(uGroundColor, uSkyColor, a), 1.0);
}

vec4 depth()
{
	float z = clamp(vec4(ciModelViewProjection * ciPosition).z, 0.0, uDepthCutoff);
	float a = pow(1.0 - z / uDepthCutoff, uDepthFalloff);
	return vec4( vec3( a ), 1.0 );
}

void main()
{
	vec3 normal = normalize(ciNormalMatrix * ciNormal);
	vColor = hemisphereColor(normal) * uColor * depth();
	gl_Position = ciModelViewProjection * ciPosition;
}
