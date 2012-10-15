attribute vec3 Position;
attribute vec3 Normal;
attribute vec4 SourceColor;
attribute vec2 TexCoord;

varying vec4 DestinationColor;
varying vec3 fragmentNormal;

uniform mat4 Projection;
uniform mat4 Modelview;

void main(void)
{
	DestinationColor = SourceColor;
	gl_Position = Projection * Modelview * vec4(Position, 1);
	fragmentNormal = normalize(Normal);
}

