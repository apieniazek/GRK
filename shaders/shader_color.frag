#version 430 core

uniform vec3 objectColor;
uniform vec3 lightDir;

in vec3 interpNormal;
in vec3 Position;


void main()
{
	vec3 lightDir2 = normalize(Position - vec3(0,0,0));
	
	vec3 normal = normalize(interpNormal);
	float diffuse = max(dot(normal, -lightDir2), 0.0);
	gl_FragColor = vec4(objectColor * diffuse, 1.0);
}
