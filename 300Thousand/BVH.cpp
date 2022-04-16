#include "BVH.h"

BVH::BVH(vector<SceneObject> objects)
{
	// set objects into bvh node list
	for (SceneObject object : objects)
	{
		addNode(object);
	}

	glGenVertexArrays(INSTANCE_NUM - 1, vaos);
	int count = 0;
	std::cout << "````````````````" << std::endl;
	for (int i = 0; i < bvhNodes.size(); i++)
	{
		if (bvhNodes[i].indexMapToScene == -1)
		{
			std::cout << i << std::endl;
			glBindVertexArray(vaos[count]);
			vbos[count] = BVH::createAABBVbo(bvhNodes[i].aabb);
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
			glBindVertexArray(0);
			count++;
		}
	}
	std::cout << "````````````````" << std::endl;

	std::cout << count << " --------- " << std::endl;
}

//BVHNode BVH::traverseBVH()
//{
//	if (bvhNodes.size() <= 0)
//	{ 
//		std::cout << "traverse failed: bvh can not be null!" << std::endl;
//	}
//
//}

void BVH::addNode(SceneObject object)
{
	if (bvhNodes.size() <= 0)
	{
		// create root node
		BVHNode node(object.aabb, -1, -1, -1, 0, object.index);
		bvhNodes.push_back(node);
		rootIndex = 0;
	}
	else
	{
		AABB aabb_1 = object.aabb;
		
		// find closet node 2
		int node_2_index = findClosestNode(aabb_1, rootIndex);

		BVHNode node_2 = bvhNodes[node_2_index];
		AABB aabb_2 = node_2.aabb;

		// create new bounding box
		AABB aabb_new = aabb_1.unions(aabb_2);
		if (node_2_index == 6)
		{
			std::cout << "aabb_new 8 for 6: " << aabb_new.minZ << ", " << aabb_new.maxZ << std::endl;
			std::cout << "aabb_1 8 for 6: " << aabb_1.minZ << ", " << aabb_1.maxZ << std::endl;
			std::cout << "aabb_2 8 for 6: " << aabb_2.minZ << ", " << aabb_2.maxZ << std::endl;
			std::cout << "the current index is: " << object.index << std::endl;
		}

		int node_2_parent_index = node_2.parentNode;
		int node_1_index = bvhNodes.size();
		int branch_node_index = bvhNodes.size() + 1;

		// create new leave node
		BVHNode leave_node(aabb_1, branch_node_index, -1, -1, node_1_index, object.index);
		bvhNodes.push_back(leave_node);

		// crate branch node with new aabb (use -1 as the branch node)
		BVHNode branch_node(aabb_new, node_2_parent_index, node_2_index, node_1_index, branch_node_index, -1);
		bvhNodes.push_back(branch_node);

		// update closest node
		bvhNodes[node_2_index].parentNode = branch_node_index;

		// update parent node
		if (node_2_index == rootIndex) {
			rootIndex = branch_node_index;
		}
		else
		{
			std::cout << "parent root is not null" << std::endl;
			// parent root is not null
			if (bvhNodes[node_2_parent_index].leftChildNode == node_2_index)
			{
				bvhNodes[node_2_parent_index].leftChildNode = branch_node_index;
			}
			else if (bvhNodes[node_2_parent_index].rightChildNode == node_2_index)
			{
				bvhNodes[node_2_parent_index].rightChildNode = branch_node_index;
			}
			updateAABBInBVH(node_2_parent_index);
		}
		
		
	}
}

int BVH::findClosestNode(AABB aabb, int nodeIndex)
{
	// first compare with root, then with other branch node
	if (aabb.overlap(bvhNodes[nodeIndex].aabb))
	{
		std::cout << "overlap" << std::endl;

		if (bvhNodes[nodeIndex].indexMapToScene == -1)
		{
			// branch node and it must have two children at the same time
			int left_child_node = findClosestNode(aabb, bvhNodes[nodeIndex].leftChildNode);
			int right_child_node = findClosestNode(aabb, bvhNodes[nodeIndex].rightChildNode);

			float left_area_sum = aabb.surfaceAreaSum(bvhNodes[left_child_node].aabb);
			float right_area_sum = aabb.surfaceAreaSum(bvhNodes[right_child_node].aabb);
			if (left_area_sum <= right_area_sum)
				return left_child_node;
			else
				return right_child_node;
		}
		else
		{
			// leave node
			return nodeIndex;
		}
	}
	else
	{
		std::cout << "not collision" << std::endl;
		return nodeIndex;
	}
}

void BVH::updateAABBInBVH(int nodeIndex)
{
	int left_child = bvhNodes[nodeIndex].leftChildNode;
	int right_child = bvhNodes[nodeIndex].rightChildNode;

	bvhNodes[nodeIndex].aabb = bvhNodes[left_child].aabb.unions(bvhNodes[right_child].aabb);

	if (nodeIndex != rootIndex)
		updateAABBInBVH(bvhNodes[nodeIndex].parentNode);
}

void BVH::traverseBVH(int index)
{
	// print node
	cout << "object index: " << bvhNodes[index].indexMapToScene << " | "
		<< "tree index: " << bvhNodes[index].index << " | "
		<< "left child: " << bvhNodes[index].leftChildNode << " | "
		<< "right child: " << bvhNodes[index].rightChildNode << " | "
		<< "parent node: " << bvhNodes[index].parentNode
		<< std::endl;

	if (bvhNodes[index].indexMapToScene == -1)
	{
		// branch node
		traverseBVH(bvhNodes[index].leftChildNode);
		traverseBVH(bvhNodes[index].rightChildNode);
	}
	
}

GLuint BVH::createAABBVbo(AABB aabb)
{
	vector<glm::vec3> boundingBoxVertices = generateAABBvertices(aabb);

	GLuint vbo;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * 36, boundingBoxVertices.data(), GL_STATIC_DRAW);

	return vbo;
}

vector<vec3> BVH::generateAABBvertices(const AABB aabb)
{
	std::cout << aabb.minX << ", " << aabb.minY << ", " << aabb.minZ << ", " << aabb.maxX << ", " << aabb.maxY << ", " << aabb.maxZ << std::endl;
	vector<glm::vec3> boundingBoxVerties;

	boundingBoxVerties.push_back(glm::vec3(aabb.minX, aabb.maxY, aabb.maxZ));
	boundingBoxVerties.push_back(glm::vec3(aabb.minX, aabb.minY, aabb.maxZ));
	boundingBoxVerties.push_back(glm::vec3(aabb.maxX, aabb.maxY, aabb.maxZ));

	boundingBoxVerties.push_back(glm::vec3(aabb.maxX, aabb.maxY, aabb.maxZ));
	boundingBoxVerties.push_back(glm::vec3(aabb.minX, aabb.minY, aabb.maxZ));
	boundingBoxVerties.push_back(glm::vec3(aabb.maxX, aabb.minY, aabb.maxZ));

	boundingBoxVerties.push_back(glm::vec3(aabb.maxX, aabb.maxY, aabb.maxZ));
	boundingBoxVerties.push_back(glm::vec3(aabb.maxX, aabb.minY, aabb.maxZ));
	boundingBoxVerties.push_back(glm::vec3(aabb.maxX, aabb.maxY, aabb.minZ));

	boundingBoxVerties.push_back(glm::vec3(aabb.maxX, aabb.maxY, aabb.minZ));
	boundingBoxVerties.push_back(glm::vec3(aabb.maxX, aabb.minY, aabb.maxZ));
	boundingBoxVerties.push_back(glm::vec3(aabb.maxX, aabb.minY, aabb.minZ));

	boundingBoxVerties.push_back(glm::vec3(aabb.maxX, aabb.maxY, aabb.minZ));
	boundingBoxVerties.push_back(glm::vec3(aabb.maxX, aabb.minY, aabb.minZ));
	boundingBoxVerties.push_back(glm::vec3(aabb.minX, aabb.minY, aabb.minZ));

	boundingBoxVerties.push_back(glm::vec3(aabb.maxX, aabb.maxY, aabb.minZ));
	boundingBoxVerties.push_back(glm::vec3(aabb.minX, aabb.minY, aabb.minZ));
	boundingBoxVerties.push_back(glm::vec3(aabb.minX, aabb.maxY, aabb.minZ));

	boundingBoxVerties.push_back(glm::vec3(aabb.minX, aabb.maxY, aabb.minZ));
	boundingBoxVerties.push_back(glm::vec3(aabb.minX, aabb.minY, aabb.minZ));
	boundingBoxVerties.push_back(glm::vec3(aabb.minX, aabb.maxY, aabb.maxZ));

	boundingBoxVerties.push_back(glm::vec3(aabb.minX, aabb.maxY, aabb.maxZ));
	boundingBoxVerties.push_back(glm::vec3(aabb.minX, aabb.minY, aabb.minZ));
	boundingBoxVerties.push_back(glm::vec3(aabb.minX, aabb.minY, aabb.maxZ));

	boundingBoxVerties.push_back(glm::vec3(aabb.minX, aabb.maxY, aabb.minZ));
	boundingBoxVerties.push_back(glm::vec3(aabb.minX, aabb.maxY, aabb.maxZ));
	boundingBoxVerties.push_back(glm::vec3(aabb.maxX, aabb.maxY, aabb.minZ));

	boundingBoxVerties.push_back(glm::vec3(aabb.maxX, aabb.maxY, aabb.minZ));
	boundingBoxVerties.push_back(glm::vec3(aabb.minX, aabb.maxY, aabb.maxZ));
	boundingBoxVerties.push_back(glm::vec3(aabb.maxX, aabb.maxY, aabb.maxZ));

	boundingBoxVerties.push_back(glm::vec3(aabb.minX, aabb.minY, aabb.minZ));
	boundingBoxVerties.push_back(glm::vec3(aabb.maxX, aabb.minY, aabb.minZ));
	boundingBoxVerties.push_back(glm::vec3(aabb.minX, aabb.minY, aabb.maxZ));

	boundingBoxVerties.push_back(glm::vec3(aabb.minX, aabb.minY, aabb.maxZ));
	boundingBoxVerties.push_back(glm::vec3(aabb.maxX, aabb.minY, aabb.minZ));
	boundingBoxVerties.push_back(glm::vec3(aabb.maxX, aabb.minY, aabb.maxZ));

	return boundingBoxVerties;
}

void BVH::drawBVH()
{
	for (int i = 0; i < INSTANCE_NUM - 1; i++)
	{
		glBindVertexArray(vaos[i]);
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glUniform1i(2, 2);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glBindVertexArray(0);
	}
}

int BVH::getRootIndex()
{
	return rootIndex;
}


