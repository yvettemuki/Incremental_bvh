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
	AABBNode(SceneObject object, AABB aabb, unsigned int parenetNode, unsigned int leftChilNode, unsigned int rightChildNode, unsigned int nextIndex)
		: object(object), aabb(aabb), parentNode(parenetNode), leftChildNode(leftChildNode), rightChildNode(rightChildNode) {};

	SceneObject object;
	AABB aabb;
	unsigned int parentNode;
	unsigned int leftChildNode;
	unsigned int rightChildNode;
	unsigned int nextIndex;     // index map for matrix model data

};

class BVH
{
public:
	BVH() {};
	AABB initBVH(vector<SceneObject> objects);
	void addNode(SceneObject object);
	/*void updateBVH();
	void searchBVH();
	void deleteBVH();*/
	unsigned int findNeighbour(const SceneObject object);

private:
	vector<AABBNode> bvhNodes;
	vector<SceneObject> objects;
	unsigned int rootIndex;
};

