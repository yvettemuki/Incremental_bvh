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
	//std::cout << "````````````````" << std::endl;
	for (int i = 0; i < bvhNodes.size(); i++)
	{
		if (bvhNodes[i].indexMapToScene == -1)
		{
			//std::cout << i << std::endl;
			glBindVertexArray(vaos[count]);
			vbos[count] = BVH::createAABBVbo(bvhNodes[i].aabb);
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
			glBindVertexArray(0);
			count++;
		}
	}
	//std::cout << "````````````````" << std::endl;

	//std::cout << count << " --------- " << std::endl;
}

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
		/*if (node_2_index == 6)
		{
			std::cout << "aabb_new 8 for 6: " << aabb_new.minZ << ", " << aabb_new.maxZ << std::endl;
			std::cout << "aabb_1 8 for 6: " << aabb_1.minZ << ", " << aabb_1.maxZ << std::endl;
			std::cout << "aabb_2 8 for 6: " << aabb_2.minZ << ", " << aabb_2.maxZ << std::endl;
			std::cout << "the current index is: " << object.index << std::endl;
		}*/

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
			// parent root is not null
			if (bvhNodes[node_2_parent_index].leftChildNode == node_2_index)
			{
				bvhNodes[node_2_parent_index].leftChildNode = branch_node_index;
			}
			else if (bvhNodes[node_2_parent_index].rightChildNode == node_2_index)
			{
				bvhNodes[node_2_parent_index].rightChildNode = branch_node_index;
			}
			refitParentAABBInBVH(node_2_parent_index);
		}
		
		
	}
}

void BVH::updateNode(int objectNodeIndex, int parentNodeIndex)
{
	AABB aabb_1 = bvhNodes[objectNodeIndex].aabb;

	// find closet node 2
	int node_2_index = findClosestNode(aabb_1, rootIndex);
	BVHNode node_2 = bvhNodes[node_2_index];
	AABB aabb_2 = node_2.aabb;
	int node_2_parent_index = node_2.parentNode;

	// create new bounding box
	AABB aabb_new = aabb_1.unions(aabb_2);

	// update branch node (no need to update leave node because its parent is the same)
	bvhNodes[parentNodeIndex].leftChildNode = node_2_index;
	bvhNodes[parentNodeIndex].rightChildNode = objectNodeIndex;
	bvhNodes[parentNodeIndex].parentNode = node_2_parent_index;

	// update closest node
	bvhNodes[node_2_index].parentNode = parentNodeIndex;

	// update parent node
	if (node_2_index == rootIndex) {
		rootIndex = parentNodeIndex;
	}
	else
	{
		// parent root is not null
		if (bvhNodes[node_2_parent_index].leftChildNode == node_2_index)
		{
			bvhNodes[node_2_parent_index].leftChildNode = parentNodeIndex;
		}
		else if (bvhNodes[node_2_parent_index].rightChildNode == node_2_index)
		{
			bvhNodes[node_2_parent_index].rightChildNode = parentNodeIndex;
		}
		refitParentAABBInBVH(node_2_parent_index);
	}
}

int BVH::findClosestNode(AABB aabb, int nodeIndex)
{
	// first compare with root, then with other branch node
	if (aabb.overlap(bvhNodes[nodeIndex].aabb))
	{
		//std::cout << "overlap" << std::endl;

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
		//std::cout << "not collision" << std::endl;
		return nodeIndex;
	}
}

void BVH::refitParentAABBInBVH(int nodeIndex)
{
	int left_child = bvhNodes[nodeIndex].leftChildNode;
	int right_child = bvhNodes[nodeIndex].rightChildNode;

	bvhNodes[nodeIndex].aabb = bvhNodes[left_child].aabb.unions(bvhNodes[right_child].aabb);

	if (nodeIndex != rootIndex)
		refitParentAABBInBVH(bvhNodes[nodeIndex].parentNode);
}

void BVH::traverseBVH(int index)
{
	// print node
	/*cout << "object index: " << bvhNodes[index].indexMapToScene << " | "
		<< "tree index: " << bvhNodes[index].index << " | "
		<< "left child: " << bvhNodes[index].leftChildNode << " | "
		<< "right child: " << bvhNodes[index].rightChildNode << " | "
		<< "parent node: " << bvhNodes[index].parentNode
		<< std::endl;*/

	if (bvhNodes[index].indexMapToScene == -1)
	{
		// branch node
		traverseBVH(bvhNodes[index].leftChildNode);
		traverseBVH(bvhNodes[index].rightChildNode);
	}
	
}

// should access whether draw the bvh
void BVH::updateBVH()
{
	if (bvhNodes.size() == 1)
		return;

	if (bvhNodes.size() == 3)
	{
		bvhNodes[2].aabb = bvhNodes[0].aabb.unions(bvhNodes[1].aabb);
		return;
	}

	for (int i = 0; i < bvhNodes.size(); i++)
	{
		BVHNode node = bvhNodes[i];
		if (node.indexMapToScene != -1)
		{
			// leave node
			int parent_index = node.parentNode;
			int left_child_index = bvhNodes[parent_index].leftChildNode;
			int right_child_index = bvhNodes[parent_index].rightChildNode;
			int parent_of_parent_index = bvhNodes[parent_index].parentNode;
			int p_of_p_left_child_index = bvhNodes[parent_of_parent_index].leftChildNode;
			int p_of_p_right_child_index = bvhNodes[parent_of_parent_index].rightChildNode;

			if (i == left_child_index)
			{
				// set right child as the leave node, fake delete the parent node and current node
				// update another leaves node
				bvhNodes[right_child_index].parentNode = parent_of_parent_index;

				// update parent's parnets child node as the another leaves node
				if (parent_index == p_of_p_left_child_index)
					bvhNodes[parent_of_parent_index].leftChildNode = right_child_index;
				else if (parent_index == p_of_p_right_child_index)
					bvhNodes[parent_of_parent_index].rightChildNode = right_child_index;
			}
			else if (i == right_child_index)
			{
				// set left child as the leave node, remove the parent with same method
				bvhNodes[left_child_index].parentNode = parent_of_parent_index;

				if (parent_index == p_of_p_left_child_index)
					bvhNodes[parent_of_parent_index].leftChildNode = left_child_index;
				else if (parent_index == p_of_p_right_child_index)
					bvhNodes[parent_of_parent_index].rightChildNode = left_child_index;
			}

			// update parent aabb 
			refitParentAABBInBVH(parent_of_parent_index);

			// add the fake delte two nodes
			updateNode(i, parent_index);
		}
	}

	// update bvh data to draw
	int count = 0;
	for (int i = 0; i < bvhNodes.size(); i++)
	{
		if (bvhNodes[i].indexMapToScene == -1)
		{
			std::cout << vbos[count] << std::endl;
			// update the aabb box vertex data
			vector<glm::vec3> vertices = BVH::generateAABBvertices(bvhNodes[i].aabb);

			// update aabb vbo
			glBindBuffer(GL_ARRAY_BUFFER, vbos[count]);
			glBufferSubData(GL_ARRAY_BUFFER, 0, 36 * sizeof(glm::vec3), vertices.data());
			glBindBuffer(GL_ARRAY_BUFFER, 0);

			count++;
		}
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
	//std::cout << aabb.minX << ", " << aabb.minY << ", " << aabb.minZ << ", " << aabb.maxX << ", " << aabb.maxY << ", " << aabb.maxZ << std::endl;
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

vector<int> BVH::CollisionDetection(AABB aabb, int sceneIndex)
{
	vector<int> collisionObjects;  // record the sceneIndex of the collisions
	int nodeIndex = 0;

	// map to the bvh index
	if (sceneIndex == 0)
		nodeIndex = 0;
	else
		nodeIndex = (sceneIndex - 1) * 2 + 1;

	searchCollision(aabb, nodeIndex, rootIndex, collisionObjects);

	return collisionObjects;

}

void BVH::searchCollision(AABB aabb, int nodeIndex, int searchNodeIndex, vector<int>& collisions)
{
	if (nodeIndex == searchNodeIndex)
		return;

	BVHNode node = bvhNodes[searchNodeIndex];

	if (aabb.overlap(node.aabb))
	{
		// overlap with the box
		if (node.indexMapToScene == -1)
		{
			// branch collision
			searchCollision(aabb, nodeIndex, node.leftChildNode, collisions);
			searchCollision(aabb, nodeIndex, node.rightChildNode, collisions);
		}
		else
		{
			// leave collisoin
			// care about the replication situation (a,b) vs (b,a)
			if (nodeIndex < searchNodeIndex)
				collisions.push_back(node.indexMapToScene);
		}
	}
}




