#version 430 core

uniform vec3 objectColor;
uniform vec3 lightDir;
uniform sampler2D sampler2dtype;

in vec3 interpNormal;
in vec2 vertexTexCoord1;

void main()
{




	vec3 normal = normalize(interpNormal);
	

	vec4 textureColor = texture2D(sampler2dtype, vertexTexCoord1);
	gl_FragColor = vec4(textureColor.xyz, 1.0);

	
}
