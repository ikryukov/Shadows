const char* SimpleFragmentShader = STRINGIFY(
varying lowp vec4 DestinationColor;
varying lowp vec3 fragmentNormal;

lowp vec3 Light = vec3(0.0, 0.0, 1.0);
lowp vec4 Color = vec4(0.2, 0.4, 0.5, 1.0);
void main(void)
{
	//gl_FragColor = DestinationColor;
	Light = normalize(Light);
	gl_FragColor = Color * max(0.0, dot(normalize(fragmentNormal), Light));
}
);