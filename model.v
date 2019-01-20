#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aBitTangent;
layout (location = 5) in ivec4 aBoneIDs;
layout (location = 6) in vec4 aBoneWeights;


const int MAX_BONES = 20;
uniform mat4 modelview;
uniform mat4 projection;
uniform mat4 aBones[MAX_BONES];
uniform mat3 inverseModel;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoords;
out mat3 TBN;
void main()
{


	
 	gl_Position = projection * modelview * vec4(aPos.x, -aPos.y, aPos.z, 1.0);


  
    TexCoords = aTexCoords;
    Normal = aNormal;  
    TexCoords = aTexCoords;

}