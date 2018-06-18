#version 430 core

uniform vec3 objectColor;
uniform vec3 lightDir;
uniform sampler2D sampler2dtype;

in vec3 interpNormal;
in vec2 vertexTexCoord1;
in vec3 Position;

void main()
{

	/* swiatlo na teksturze */

	/* w punkcie 0,0,0 jest swiatlo. Od liczonego punktu */
	vec3 lightDir2 = normalize(Position - vec3(0,0,0));


	vec3 normal = normalize(interpNormal);
	/* funkcja dot z dwoch wektorow zwraca produkt */
	/* wartosc swiatla w punkcie to wieksze z 0.0 (0 swiatla) albo ile zostalo wyliczone */
	float diffuse = max(dot(normal, -lightDir2), 0.0);
	vec4 textureColor = texture2D(sampler2dtype, vertexTexCoord1);
	/* wartosc to kolor tekstury w punkcie przemnozony przez ilosc swiatla */
	gl_FragColor = vec4(textureColor.xyz * diffuse, 1.0);
}
