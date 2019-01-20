#version 400
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 UV;

uniform mat4 VPMat;
uniform float time;
uniform vec3 center;
uniform sampler2D dudv;

out vec3 aNormal;
out vec2 texCoords;
out vec4 FragPos;
out float oTime;
out mat4 mvp;
const float offset = 0.1;
void main()
{
oTime = time;
aNormal = normal;
texCoords = UV;

mvp = VPMat;

vec4 height = texture(dudv, UV);

aNormal = vec3(normal.x, cos(time + position.x) * offset * height.r, normal.z);
FragPos = VPMat * vec4(position.x, cos(time + position.x) * offset * height.r, position.z , 1.0);
gl_Position = FragPos;
}