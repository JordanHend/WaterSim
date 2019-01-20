#version 330 core
layout (location = 0) out vec3 _Position;
layout (location = 1) out vec3 _Normal;
layout (location = 2) out vec4 _Color;

uniform bool hasNorm;
uniform sampler2D texture_diffuse1;
uniform sampler2D texture_normal1;
uniform sampler2D texture_specular1;
in vec3 Normal;  
in vec3 FragPos;  
in vec2 TexCoords;
in mat3 TBN;
in float visibility; 
float spec = 0.4;
void main()
{ 


   vec3 norm = texture(texture_normal1, TexCoords).rgb;
    norm = normalize(norm * 2.0 - 1.0);   
    norm = normalize(TBN * norm); 
   	
    _Position = FragPos;
    _Normal = normalize(Normal);
    _Color = texture(texture_diffuse1, TexCoords);
    _Color.a = spec;

}
