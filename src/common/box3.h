#pragma once

#include <vector>
#include <glm/glm.hpp>  

struct box3
{
	/// min coordinate point
	glm::vec3 min;

	/// max coordinate point
	glm::vec3 max;

	/// The bounding box constructor (make it of size s centered in 0^3
	inline  box3(float s) { min = glm::vec3(-s / 2.f); max = glm::vec3(s / 2.f); }

	/// The bounding box constructor (make it empty)
	inline  box3() { min = glm::vec3(1.f); max = glm::vec3(-1.f); }

	/// Min Max constructor
	inline  box3(const glm::vec3& mi, const glm::vec3& ma) { min = mi; max = ma; }

	/// The bounding box distructor
	inline ~box3() { }

	/** Modify the current bbox to contain also the passed point
	*/
	void add(const glm::vec3& p)
	{
		if (min.x > max.x) { min = max = p; }
		else
		{
			if (min.x > p.x) min.x = p.x;
			if (min.y > p.y) min.y = p.y;
			if (min.z > p.z) min.z = p.z;

			if (max.x < p.x) max.x = p.x;
			if (max.y < p.y) max.y = p.y;
			if (max.z < p.z) max.z = p.z;
		}
	}

	/** Modify the current bbox to contain also another box
	*/
	void add(const box3& b)
	{
		add(glm::vec3(b.min));
		add(glm::vec3(b.max));
	}

	bool is_empty() const { return min.x > max.x ; }

	float diagonal() const
	{
		return glm::length(min - max);
	}

	glm::vec3 center() const
	{
		return (min + max) * 0.5f;
	}

	glm::vec3 p(unsigned int i) {
		return glm::vec3((i % 2 == 0) ? min.x : max.x, ((i / 2) % 2 == 0) ? min.y : max.y, ((i / 4) == 0) ? min.z : max.z);
	}

};