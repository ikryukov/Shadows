attribute vec3 Position;

uniform mat4 Projection;
uniform mat4 Modelview;

void main(void)
{
	gl_Position = Projection * Modelview * vec4(Position, 1);
}

