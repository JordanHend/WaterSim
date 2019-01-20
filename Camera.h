#ifndef CAMERA_H
#define CAMERA_H


#include <glm\glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <vector>
using std::vector;
using glm::vec3;
using glm::vec2;

enum Camera_Movement {
	FORWARD,
	BACKWARD,
	LEFT,
	RIGHT
};
class Camera
{
public:

	float speed = 50;
	vec3 position;
	vec3 up;
	vec3 front;
	vec3 right;

	vec3 worldUp;

	float pitch, yaw;

	Camera(vec3 position = vec3(0,0,0), vec3 front = vec3(0,0,1), vec3 up = vec3(0,1,0))
	{
		this->position = position;
		this->front = front;
		this->up = up;
		right = { -1.0f, 0.0f, 0.0f };
		this->worldUp = vec3(0, 1, 0);
	};

	void updateVectors();
	void ProcessMouseMovement(float xoffset, float yoffset, bool constrainPitch = true)
	{
		xoffset *= 0.1f;
		yoffset *= 0.1f;

		yaw += xoffset;
		pitch += yoffset;

		// Make sure that when pitch is out of bounds, screen doesn't get flipped
		if (constrainPitch)
		{
			if (pitch > 89.0f)
				pitch = 89.0f;
			if (pitch < -89.0f)
				pitch = -89.0f;
		}

		// Update Front, Right and Up Vectors using the updated Eular angles
		updateVectors();
	};


	glm::mat4 getViewMatrix()
	{
		return glm::lookAt(position, position + front, up);
	};

	void ProcessKeyboard(int direction, float deltaTime)
	{

			float velocity = speed * deltaTime;
			if (direction == FORWARD)
				position += front * velocity;
			if (direction == BACKWARD)
				position -= front * velocity;
			if (direction == LEFT)
				position -= right * velocity;
			if (direction == RIGHT)
				position += right * velocity;
		
	}
	

	~Camera();
};

#endif