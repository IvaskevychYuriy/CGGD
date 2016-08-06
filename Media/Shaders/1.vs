#version 330 core

in vec2 vPos;
in vec3 vColor;
in vec2 vTex;
uniform mat4 transformMatrix;

out vec2 fTex;
out vec3 fColor;

void main()
{
	gl_Position = vec4(vPos.x, vPos.y, 0.0f, 1.0f) * transformMatrix;
	fColor = vColor / 255.0f;
	fTex = vTex;
}