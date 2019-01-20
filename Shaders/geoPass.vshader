#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aBitTangent;
layout (location = 5) in ivec4 aBoneIDs;
layout (location = 6) in vec4 aBoneWeights;


const int MAX_BONES = 100;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 aBones[MAX_BONES];
uniform mat3 inverseModel;
uniform bool hasAnimations;
out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoords;
out float visibility;
out mat3 TBN;
const float density = 0.002;
const float gradient = 2.0;
void main()
{



if(hasAnimations && aBoneWeights[0] != 0)
{
	mat4 boneTransform = aBones[aBoneIDs[0]] * aBoneWeights[0];
	boneTransform += aBones[aBoneIDs[1]] * aBoneWeights[1];
	boneTransform += aBones[aBoneIDs[2]] * aBoneWeights[2];	
	boneTransform += aBones[aBoneIDs[3]] * aBoneWeights[3];


 	gl_Position = projection * view * model * boneTransform * vec4(aPos, 1.0);
 	FragPos = vec3(model  *  vec4(aPos, 1.0));
 	Normal =  mat3(transpose(inverse(model))) * mat3(boneTransform) *  aNormal;; 
}

else
{
	
	gl_Position = projection * view * model  * vec4(aPos, 1.0);
 	FragPos = vec3(model  *  vec4(aPos, 1.0));
 	Normal =  mat3(transpose(inverse(model)))  *  aNormal;; 



}

   vec3 T = normalize(vec3(model * vec4(aTangent,   0.0)));
   vec3 B = normalize(vec3(model * vec4(aBitTangent, 0.0)));
   vec3 N = normalize(vec3(model * vec4(aNormal,    0.0)));
   TBN = mat3(T, B, N);

TexCoords = aTexCoords;



}