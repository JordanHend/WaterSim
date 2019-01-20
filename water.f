#version 400

in vec4 FragPos;
in vec3 aNormal;
in vec2 texCoords;
in float oTime;
in mat4 mvp;
out vec4 fragColor;


uniform sampler2D reflectTex;
uniform sampler2D refractTex;

uniform sampler2D diffusetex;

uniform vec3 viewPos;

void main()
{
	vec3 mirrorNormal = vec3(0, 1, 0);
	mirrorNormal = normalize(mirrorNormal);

	vec3 nNorm = normalize(aNormal);

	vec3 norm = aNormal - dot (nNorm, mirrorNormal) * mirrorNormal;
	vec3 eyeNorm = (mvp * vec4(norm, 1)).xyz;
	vec2 refloffset = normalize(eyeNorm.xy) * length(norm) * 0.2;

	vec2 ndc = FragPos.xy / FragPos.w;
	vec2 vTextureReflection = (vec2(0.5, 0.5) * ndc) + 0.5;

	vec4 reflectionTextureColor = texture2D(reflectTex, vTextureReflection + refloffset);


	vec2 nTex = vec2(texCoords.x + (oTime * 0.01), texCoords.y);
  	vec3 viewDir = normalize(viewPos - FragPos.xyz);

	float amb = 0.4;
	vec3 color = texture(diffusetex, nTex).xyz;
	vec3 lightDir = vec3(0.2, 0.5 , 0);
	float diff = max(dot(aNormal, lightDir), 0.0);
	float spec = pow(max(dot(viewDir, lightDir), 0), 126);


	vec3 ambient = color * amb;
	vec3 diffuse = color * diff;
	vec3 specular = color * spec;
	color = ambient + diffuse + specular;

	vec3 color0 = mix(color, reflectionTextureColor.xyz, 0.5);
	fragColor = vec4(color0, 0.5);

}