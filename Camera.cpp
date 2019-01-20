#include "Camera.h"



void Camera::updateVectors()
{
	// Calculate the new Front vector
	glm::vec3 f;
	f.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	f.y = sin(glm::radians(pitch));
	f.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	front = glm::normalize(f);
	// Also re-calculate the Right and Up vector
	right = glm::normalize(glm::cross(f, worldUp));  // Normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
	up = glm::normalize(glm::cross(right, f));
}

Camera::~Camera()
{
}
