#pragma once
#include "AABB.h";
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "SceneObject.h"

using namespace std;
using namespace glm;

struct AABBNode
{
	AABB aabb;
	unsigned int parentNode;
	unsigned int leftChildNode;
	unsigned int rightChildNode;
	unsigned int nextIndex;     // index map for matrix model data
};

class BVH
{
public:
	BVH();
	void initBVH();
	void addNode();
	void updateBVH();
	void searchBVH();
	void deleteBVH();


private:
	vector<SceneObject> objects;
};

