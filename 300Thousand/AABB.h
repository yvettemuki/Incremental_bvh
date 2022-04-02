#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

struct AABB
{
public:
	AABB(float minX, float minY, float minZ, float maxX, float maxY, float maxZ)
		: minX(minX), minY(minY), minZ(minZ), maxX(maxX), maxY(maxY), maxZ(maxZ) {};

	bool intersect(const AABB& other) const
	{
		return (
			minX < other.maxX&& maxX > other.minX &&
			minY < other.maxY&& maxY > other.minY &&
			minZ < other.maxZ&& maxZ > other.minZ
			);
	};

	AABB update(const glm::vec3 pos) const
	{
		AABB aabb(pos.x + minX, pos.y + minY, pos.z + minZ, 
			pos.x + maxX, pos.y + maxY, pos.z + maxZ);

		return aabb;
	}
	
	float minX;
	float minY;
	float minZ;
	float maxX;
	float maxY;
	float maxZ;
	
};

