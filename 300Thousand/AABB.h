#pragma once
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
	
	float minX;
	float minY;
	float minZ;
	float maxX;
	float maxY;
	float maxZ;
	
};

