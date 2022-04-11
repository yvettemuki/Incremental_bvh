#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <math.h>
#include <iostream>

struct AABB
{
public:
	AABB(float minX, float minY, float minZ, float maxX, float maxY, float maxZ)
		: minX(minX), minY(minY), minZ(minZ), maxX(maxX), maxY(maxY), maxZ(maxZ) {};

	bool overlap(const AABB& other) const
	{
		return (
			minX < other.maxX&& maxX > other.minX &&
			minY < other.maxY&& maxY > other.minY &&
			minZ < other.maxZ&& maxZ > other.minZ
			);
	};

	glm::vec3 intersection(const AABB& other) const
	{
		glm::vec3 intersectArea = glm::vec3(
			std::min(maxX - other.minX, other.maxX - minX),
			std::min(maxY - other.minY, other.maxY - minY),
			std::min(maxZ - other.minZ, other.maxZ - minZ)
		);

		return intersectArea;
	}

	AABB unions(const AABB& other) const
	{
		return AABB(
			std::min(minX, other.minX), std::min(minY, other.minY), std::min(minZ, minZ),
			std::max(maxX, other.maxY), std::max(maxY, other.maxY), std::max(maxZ, maxZ)
		);
	}

	void update(const glm::vec3 deltaPos)
	{
		/*AABB aabb(pos.x + minX, pos.y + minY, pos.z + minZ, 
			pos.x + maxX, pos.y + maxY, pos.z + maxZ);

		return aabb;*/

		minX += deltaPos.x;
		maxX += deltaPos.x;
		minY += deltaPos.y;
		maxY += deltaPos.y;
		minZ += deltaPos.z;
		maxZ += deltaPos.z;
	}

	float surfaceAreaSum(const AABB& other) const
	{
		// please attection to the error on the min max 
		float width = abs(std::max(maxX, other.maxX) - std::min(minX, other.minX));
		float height = abs(std::max(maxY, other.maxY) - std::min(minY, other.minY));
		float depth = abs(std::max(maxZ, other.maxZ) - std::min(minZ, other.minZ));

		return (width * height + width * depth + height * depth) * 2.0f;
	}
	
	float minX;
	float minY;
	float minZ;
	float maxX;
	float maxY;
	float maxZ;
	
};

