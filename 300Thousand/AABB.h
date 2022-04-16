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
		: minX(minX), minY(minY), minZ(minZ), maxX(maxX), maxY(maxY), maxZ(maxZ) 
	{
		setDefaultAABB(minX, minY, minZ, maxX, maxY, maxZ);
	};

	void setDefaultAABB(float minX, float minY, float minZ, float maxX, float maxY, float maxZ)
	{
		minX_0 = minX;
		minY_0 = minY;
		minZ_0 = minZ;

		maxX_0 = maxX;
		maxY_0 = maxY;
		maxZ_0 = maxZ;
	}

	bool overlap(const AABB& other) const
	{
		return (
			minX < other.maxX&& maxX > other.minX &&
			//minY < other.maxY&& maxY > other.minY &&  // because we just need xz moving
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

	void update(const glm::vec3 pos,  const glm::vec3 scale)
	{
		/*AABB aabb(pos.x + minX, pos.y + minY, pos.z + minZ,
			pos.x + maxX, pos.y + maxY, pos.z + maxZ);

		return aabb;*/

		minX = minX_0 * scale.x + pos.x;
		maxX = maxX_0 * scale.x + pos.x;
		minY = minY_0 * scale.y + pos.y;
		maxY = maxY_0 * scale.y + pos.y;
		minZ = minZ_0 * scale.z + pos.z;
		maxZ = maxZ_0 * scale.z + pos.z;
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

	float minX_0;
	float minY_0;
	float minZ_0;
	float maxX_0;
	float maxY_0;
	float maxZ_0;
	
};

