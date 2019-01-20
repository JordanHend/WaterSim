
#include <glm\glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glfw3.h>
#include <string>
#include <vector>
#include <iostream>
#include <vector>
#include <glm\glm.hpp>
#include "Shaders\Shader.h"
#include "Camera.h"
#include "Includes\stb_image.h"
#include <glfw3native.h>
#include "Timer.h"
#include "ETexture.h"
#include "Shadow.h"
using std::vector;
using glm::vec3;
using glm::vec2;
Shader shader;

//Window info
GLFWwindow * window;
int SCREEN_WIDTH = 1280, SCREEN_HEIGHT = 720;

struct WaterVertex
{
	vec3 pos;
	vec3 normal;
	vec2 uv;
};

//Water surface waterplane
vector<WaterVertex> waterplane;
vector<unsigned int> indices;

//Generates water surface mesh
void setUpSurface(const int w,const int h,const int offset);

void error_callback(int error, const char* description)
{
	fprintf(stderr, "Error: %s\n", description);
}
//Callbacks for input
void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
void mouseCallback(GLFWwindow * window, double xposition, double yposition);
void framebuffer_size_callback(GLFWwindow * window, int width, int height);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);

Camera camera;
//Members used for mouse input.
double lastX = 0, lastY = 0;
bool firstMouse = true;


void APIENTRY MessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
{
	fprintf(stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n\n",
		(type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""),
		type, severity, message);
}


int initOpenGL()
{

	//Init glfw
	if (!glfwInit())
	{
		// Initialization failed
		std::cout << "glfw didnt init" << std::endl;
	}
	//Set version
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	//glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);

	//Creating window
	window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Water", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create window";
		return -1;
	}

	//Set context to current window
	glfwMakeContextCurrent(window);
	//Set event callback functions
	glfwSetKeyCallback(window, keyboardCallback);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouseCallback);
	glfwSetScrollCallback(window, scroll_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	glfwSetErrorCallback(error_callback);

	glfwSwapInterval(0);

	//initialize GLAD function pointers.
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	return 1;

}



Timer wavetimer;
int main()
{
	if (!initOpenGL())
	{
		std::cout << "Failed to init OpenGL" << std::endl;
		return -1;
	}
	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback(MessageCallback, 0);
	shader.init("water.v", "water.f");
	setUpSurface(500, 500, 5);

	//glEnable(GL_DEPTH_TEST);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);


	unsigned int VAO, VBO, EBO;
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);
	glGenVertexArrays(1, &VAO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(WaterVertex) * waterplane.size(), &waterplane[0], GL_STATIC_DRAW);
	
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int)  * indices.size(), &indices[0], GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(WaterVertex), (void*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(WaterVertex), (void*)offsetof(WaterVertex, normal));
	glEnableVertexAttribArray(1);

	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(WaterVertex),(void*)offsetof(WaterVertex, uv));
	glEnableVertexAttribArray(2);


	glBindVertexArray(0);

	camera.position = vec3(12, 34, 22);
	camera.front = vec3(0.68, 0.72, 0.01);
	camera.up = vec3(0.72, 0.68, 0.01);
	camera.yaw = 1.02;
	camera.pitch = -46;
	ETexture tex;
	tex.init("water.png");
	tex.genTexture();
	shader.use();
	shader.setInt("diffusetex", 0);
	shader.setVec3("center", vec3(20, 0, 20));
	glBindTexture(GL_TEXTURE_2D, tex.getTextureID());
	const float deltatime = 0.0001;


	ETexture dudv;
	dudv.init("dudv.jpg");
	dudv.genTexture();

	wavetimer.start();

	initShadows();


	glEnable(GL_BLEND);
	glEnable(GL_STENCIL_TEST);
	glBlendEquation(GL_FUNC_ADD),
	glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_DEPTH_TEST);
	while (!glfwWindowShouldClose(window))
	{
	
		glm::mat4 projection = glm::perspective(glm::radians(50.f), (float)SCREEN_WIDTH / SCREEN_HEIGHT, 0.1f, 100.f);

		glClearColor(0.4f, 0.4f, 0.4f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


		drawReflection();

		glm::vec3 position = vec3(5, 1, 5);
		glm::mat4 modelm;
		modelm = glm::mat4(1);
		modelm = glm::translate(modelm, position);
		modelm *= glm::scale(glm::vec3(scale, scale, scale));

		modelm *= glm::eulerAngleXYZ(model.eulerAngles.x, model.eulerAngles.y, model.eulerAngles.z);


		glm::mat4 viewmat = camera.getViewMatrix() * modelm;
		reflectShader.setMat4("projection", projection);
		reflectShader.setMat4("modelview", viewmat);
		model.Draw(glm::vec3(5, 1, 5 ), glm::vec3(scale), reflectShader);
		shader.use();


		glActiveTexture(GL_TEXTURE0);
		shader.setInt("dudv", 0);
		glBindTexture(GL_TEXTURE_2D, dudv.getTextureID());

		glActiveTexture(GL_TEXTURE1);
		shader.setInt("diffusetex", 1);
		glBindTexture(GL_TEXTURE_2D, tex.getTextureID());

		glActiveTexture(GL_TEXTURE2);
		shader.setInt("reflectTex", 2);
		glBindTexture(GL_TEXTURE_2D, reflT);


		//Camera movement 
		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
			camera.ProcessKeyboard(FORWARD, deltatime);
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
			camera.ProcessKeyboard(BACKWARD, deltatime);
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
			camera.ProcessKeyboard(LEFT, deltatime);
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
			camera.ProcessKeyboard(RIGHT, deltatime);
		if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
		{
			float velocity = camera.speed * deltatime;
			camera.position += glm::vec3(0.0f, 1.0f, 0.0f) * velocity;
		}
		if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
		{
			float velocity = camera.speed * deltatime;
			camera.position += glm::vec3(0.0f, -1.0f, 0.0f) * velocity;
		}



		shader.setMat4("VPMat", projection * camera.getViewMatrix());
		shader.setFloat("time", wavetimer.getTicks() / 1000);
		shader.setVec3("viewPos", camera.position);


		glBindVertexArray(VAO);
		glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);


		glfwSwapBuffers(window);
		glfwPollEvents();
	}
}

void setUpSurface(const int w,const int h,const int offset)
{
	double Width = (w - 1) * offset;
	double Height = (h - 1) * offset;
	double halfWidth = Width * 0.5f;
	double halfHeight = Height * 0.5f;
	unsigned mapSize = w * h;
	waterplane.resize(mapSize);


	for (int x = 0; x < w; x++)
	{
		for (int z = 0; z < h; z++)
		{
			unsigned int index = ((x) * (h)) + (z);
			float S = ((float)x / (float)w);
			float T = ((float)z / (float)h);

			float X = (S * Width) - halfWidth;
			float Y = 0;
			float Z = (T * Height) - halfHeight;

			waterplane[index].pos = vec3(x / 10.0f, 1.0f, z / 10.0f);
			waterplane[index].uv = vec2(S, T);
		}
	}
	////////////////////////////////////////////////////////////////////
	const unsigned int tWidth = w;
	const unsigned int tHeight = h;

	// 2 triangles for every quad of the terrain mesh
	const unsigned int numTriangles = (tWidth - 1) * (tHeight - 1) * 2;

	// 3 indices for each triangle in the terrain mesh
	indices.resize(numTriangles * 3);

	unsigned int index = 0; // Index in the index buffer
	for (unsigned int j = 0; j < (tHeight - 1); ++j)
	{
		for (unsigned int i = 0; i < (tWidth - 1); ++i)
		{
			int vertexIndex = (j * tWidth) + i;
			// Top triangle (T0)
			indices[index++] = vertexIndex;                           // V0
			indices[index++] = vertexIndex + tWidth + 1;        // V3
			indices[index++] = vertexIndex + 1;                       // V1
																	  // Bottom triangle (T1)
			indices[index++] = vertexIndex;                           // V0
			indices[index++] = vertexIndex + tWidth;            // V2
			indices[index++] = vertexIndex + tWidth + 1;        // V3
		}
	}


	for (unsigned int i = 0; i < indices.size(); i += 3)
	{
		glm::vec3 v0 = waterplane[indices[i + 0]].pos;
		glm::vec3 v1 = waterplane[indices[i + 1]].pos;
		glm::vec3 v2 = waterplane[indices[i + 2]].pos;


		glm::vec3 normal = glm::normalize(glm::cross(v1 - v0, v2 - v0));

		waterplane[indices[i + 0]].normal += normal;
		waterplane[indices[i + 1]].normal += normal;
		waterplane[indices[i + 2]].normal += normal;
	}

	for (unsigned int i = 0; i < waterplane.size(); ++i)
	{
		waterplane[i].normal = glm::normalize(waterplane[i].normal);
	}

}

void keyboardCallback(GLFWwindow * window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_ESCAPE)
		glfwSetWindowShouldClose(window, true);
}

void mouseCallback(GLFWwindow * window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

	lastX = xpos;
	lastY = ypos;

	camera.ProcessMouseMovement(xoffset, yoffset);
}

void framebuffer_size_callback(GLFWwindow * window, int width, int height)
{
	SCREEN_WIDTH = width;
	SCREEN_HEIGHT = height;
	glViewport(0, 0, width, height);
}

void scroll_callback(GLFWwindow * window, double xoffset, double yoffset)
{
}

void mouse_button_callback(GLFWwindow * window, int button, int action, int mods)
{
}
