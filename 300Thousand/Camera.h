#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

class Camera
{
public :
	Camera(float fov, float aspect);
	void lookAt(glm::vec3 from, glm::vec3 to, glm::vec3 up);
	void perspective(float fov, float aspect, float near, float far);
	void update();

private:
	GLuint scene_ubo = -1;
    float fov = 1.f;
	float aspect = 1.f;
	float nearClip = 0.1f;
	float farClip = 100.f;
	glm::vec3 camPos = glm::vec3(1.0);
	glm::vec3 lookAtPoint = glm::vec3(0.0);
	glm::vec3 lookUpDir = glm::vec3(0.0, 1.0, 0.0);
	glm::mat4 viewMat = glm::mat4(1.0);
	glm::mat4 projMat = glm::mat4(1.0);  // only implement perspective
};

