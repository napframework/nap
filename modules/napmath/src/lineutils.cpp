// Local includes
#include "lineutils.h"

namespace nap
{
	float math::getDistancesAlongLine(const std::vector<glm::vec3>& vertexPositions, std::map<float, int>& outDistances, bool closed)
	{
		// Get position data
		const std::vector<glm::vec3>& pos_data = vertexPositions;

		// Reserve space to hold all distance data
		int vert_count = static_cast<int>(pos_data.size());
		assert(vert_count > 1);
		outDistances.clear();

		float total_distance(0.0f);
		float curre_distance(0.0f);
		for (int i = 0; i < vert_count; i++)
		{
			// Lookup previous point, 0 references itself and will be a distance of 0
			int prev_point = nap::math::max(i - 1, 0);

			// Get distance to that point
			curre_distance = glm::length(pos_data[i] - pos_data[prev_point]);

			// Add and increment
			total_distance += curre_distance;
			outDistances.emplace(std::make_pair(total_distance, i));
		}

		// Add extra distance point when dealing with closed shapes
		if (closed)
		{
			curre_distance = glm::length(pos_data.back() - pos_data.front());
			total_distance += curre_distance;
			outDistances.emplace(std::make_pair(total_distance, 0));
		}
		return total_distance;
	}

}