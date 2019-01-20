#version 330 core

out vec4 FragColor;
uniform sampler2D texture_diffuse1;
uniform sampler2D texture_specular1;
uniform sampler2D texture_normal1;
uniform sampler2D texture_height1;

in vec3 Normal;  
in vec3 FragPos;  
in vec2 TexCoords;
in mat3 TBN;


void main()
{
    // properties
    vec3 norm;
    norm = texture(texture_normal1, TexCoords).rgb;
	norm = normalize(norm); 

    vec4 temp = texture(texture_diffuse1, TexCoords);
    if(temp.w < 0.5)
    discard;
	else
    
    FragColor = vec4(temp.xyz, temp.a);

}

