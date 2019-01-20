#ifndef SHADOW_REF_H
#define SHADOW_REF_H

#include <glad/glad.h>
#include <glfw3.h>
#include <glm\glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include "Shaders\Shader.h"
#include "Camera.h"
#include "Model\Model.h"

unsigned int ReflectFBO, RefractFBO;
extern Camera camera;
Shader reflectShader, refractShader;
Model model;
unsigned int reflT = 0;
const int ReflectW = 1280, ReflectH = 720;
const int RefractW = 500, RefractH = 250;
const float scale = 0.02;
void initShadows()
{

	model.Init("Models/zelda.obj");
	model.eulerAngles.x = 3.14;
	glGenFramebuffers(1, &ReflectFBO);
	// create depth cubemap texture
	if (!glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE)
	{
		std::cout << "Failed to create reflect buffer" << std::endl;
	}







	reflectShader.init("model.v", "model.f");
	reflectShader.use();
	reflectShader.setMat4("view", glm::lookAt(vec3(0), vec3(0) + vec3(0,1,0), glm::vec3(1,0,0)));

	glGenTextures(1, &reflT);
	glBindTexture(GL_TEXTURE_2D, reflT);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, SCREEN_WIDTH,SCREEN_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindFramebuffer(GL_FRAMEBUFFER, ReflectFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, reflT, 0);

	glBindTexture(GL_TEXTURE_2D, 0);
}

extern Shader shader;
void drawReflection()
{
	shader.use();
	glm::mat4 projection = glm::perspective(glm::radians(50.f), (float)SCREEN_WIDTH / SCREEN_HEIGHT, 0.1f, 100.f);

	glm::mat4 view = camera.getViewMatrix();

	glm::vec3 position = vec3(5, 1, 5);
	glm::mat4 modelm;
	modelm = glm::mat4(1);
	modelm = glm::translate(modelm, position);
	modelm *= glm::scale(glm::vec3(scale, scale, scale));

	modelm *= glm::eulerAngleXYZ(model.eulerAngles.x, model.eulerAngles.y, model.eulerAngles.z);


	glm::mat4 viewmat = view * modelm;
	viewmat *= glm::scale(vec3(1, -1, 1));

	glBindFramebuffer(GL_FRAMEBUFFER, ReflectFBO);
	glClear(GL_COLOR_BUFFER_BIT);

	reflectShader.use();



	
	reflectShader.setMat4("projection", projection);
	reflectShader.setMat4("modelview", viewmat);
	model.Draw(glm::vec3(5, 1, 5), glm::vec3(scale, -scale, scale), reflectShader);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glEnable(GL_DEPTH_TEST);


}


#endif