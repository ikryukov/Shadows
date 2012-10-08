const char* QuadVertexShader = STRINGIFY(
attribute vec3 Position;
attribute vec3 Normal;
attribute vec4 SourceColor;
attribute vec2 TexCoord;

varying vec4 fColor;
varying vec3 fNormal;
varying vec2 fTexCoord;
										 
void main(void)
{
	fColor = SourceColor;
	fTexCoord = TexCoord;
	fNormal = Normal;
	gl_Position = vec4(Position, 1);
}
);
