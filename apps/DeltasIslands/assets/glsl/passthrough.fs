#version 330

smooth in float vHeight;
smooth in float vDepth;

out vec4 fColor;

float easeOutQuad( float t )
{
  return -t * ( t - 2 );
}

float easeOutQuart( float t )
{
  t -= 1;
  return -(t*t*t*t - 1);
}

void main()
{
  float color_t = easeOutQuart(vHeight);
  vec3 dark = vec3(0.0);
  vec3 light = vec3(1.0);
	fColor = vec4(mix(dark, light, color_t) * vec3(vDepth), 1.0);
}
