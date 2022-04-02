#include "BVH.h"

AABB BVH::initBVH(vector<SceneObject> objects)
{
	float minX = FLT_MAX, minY = FLT_MAX, minZ = FLT_MAX;
	float maxX = FLT_MIN, maxY = FLT_MIN, maxZ = FLT_MIN;
	vector<int> sortObjects;

	for (SceneObject obj : objects)
	{
		objects.push_back(obj);

		minX = (minX > obj.aabb.minX ? obj.aabb.minX : minX);
		minY = (minY > obj.aabb.minY ? obj.aabb.minY : minY);
		minZ = (minZ > obj.aabb.minZ ? obj.aabb.minZ : minZ);

		maxX = (maxX < obj.aabb.maxX ? obj.aabb.maxX : maxX);
		maxY = (maxY < obj.aabb.maxY ? obj.aabb.maxY : maxY);
		maxZ = (maxZ < obj.aabb.maxZ ? obj.aabb.maxZ : maxZ);
	}

	AABB sceneAABB(minX, minY, minZ, maxX, maxY, maxZ);
	return sceneAABB;
}

void BVH::addNode(SceneObject object)
{
	if (bvhNodes.size() <= 0)
	{
		// add root node
		rootIndex = object.index;
		AABBNode node(object, object.aabb, 0, 0, 0, 0);  // if the index = self index, pointer is point to self
		// pay attention to map the node index into object vector index
		bvhNodes.push_back(node);
		return;
	}
	
	// add non-root node
	unsigned int neighbour_id = findNeighbour(object);
}

unsigned int BVH::findNeighbour(SceneObject object)
{
	return 1;
}