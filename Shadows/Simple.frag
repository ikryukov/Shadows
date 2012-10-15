varying highp vec4 DestinationColor;
varying highp vec3 fragmentNormal;

highp vec3 Light = vec3(0.0, 0.0, 1.0);
highp vec4 Color = vec4(0.2, 0.4, 0.5, 1.0);
void main(void)
{
	Light = normalize(Light);
	gl_FragColor = Color * max(0.0, dot(fragmentNormal, Light));
}
