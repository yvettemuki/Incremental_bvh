#pragma once
#include "AABB.h";
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "SceneObject.h"
#include "Constants.hpp"

using namespace std;
using namespace glm;

/*
 when node is root, its parentNode = -1, when node is leave, its childnode = -1, -1 equals null
*/

struct BVHNode
{
	BVHNode(AABB aabb, int parenetNode, int leftChildNode, int rightChildNode, int index, int indexInMapToScene)
		: aabb(aabb), parentNode(parenetNode), leftChildNode(leftChildNode), rightChildNode(rightChildNode), index(index), indexMapToScene(indexInMapToScene) {};

	AABB aabb;
	int parentNode;
	int leftChildNode;
	int rightChildNode;
	int index;
	int indexMapToScene;
};

class BVH
{
public:
	BVH(const vector<SceneObject>);
	void initBVH();
	void addNode(SceneObject object);
	void updateBVH();
	void traverseBVH(int index);
	void deleteBVH();
	int findClosestNode(AABB aabb, int nodeIndex);
	void updateAABBInBVH(int node_2_parent_index);
	void drawBVH();
	int getRootIndex();
	static GLuint createAABBVbo(AABB aabb);
	static vector<vec3> generateAABBvertices(AABB aabb);

	GLuint vaos[INSTANCE_NUM - 1];
	GLuint vbos[INSTANCE_NUM - 1];

private:
	vector<BVHNode> bvhNodes;
	//vector<SceneObject> objects;
	int rootIndex;
};

