#version 330 

in vec3 a_Pos;
in vec4 a_Color;

out vec4 v_Color;

uniform mat4 worldT;
uniform mat4 viewT;
uniform mat4 projectionT;


void main()
{
	vec4 newPosition = projectionT * viewT * worldT * vec4(a_Pos,1);

	gl_position = newPosition;
	v_Color = a_Color;
}