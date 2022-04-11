#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "AABB.h"

using namespace glm;

struct SceneObject
{
public:

	SceneObject(unsigned int index, vec3 currPos, vec3 prevPos, vec3 velocity, AABB aabb) :
		index(index), currPos(currPos), prevPos(prevPos), velocity(velocity), aabb(aabb) {};   // keep attection the assignment constuctor will execute automatically

	vec3 getBbMin();
	vec3 getBbMax();

	unsigned int index;  // index map to the opengl input instance id
	vec3 currPos;
	vec3 prevPos;
	vec3 velocity;
	AABB aabb;

};
