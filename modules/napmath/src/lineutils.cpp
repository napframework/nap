/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local includes
#include "lineutils.h"

// External includes
#include <assert.h>
#include <glm/detail/func_common.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace nap
{
	float math::getDistancesAlongLine(const std::vector<glm::vec3>& vertexPositions, std::map<float, int>& outDistances, bool loop)
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
		if (loop)
		{
			curre_distance = glm::length(pos_data.back() - pos_data.front());
			total_distance += curre_distance;
			outDistances.emplace(std::make_pair(total_distance, 0));
		}
		return total_distance;
	}


	float math::getVertexLerpValue(const std::map<float, int>& distanceMap, float location, int& outLowerBound, int& outUpperBound)
	{
		// Make sure the sample location if less then or equal to 0
		assert(location <= 1.0f && location >= 0.0f);

		// Get distance to sample along line
		float sample_distance = distanceMap.rbegin()->first * location;

		// Get lower and upper bounds (ie, vertices that hold the lower and upper bound
		auto upper_it = distanceMap.lower_bound(sample_distance);
		assert(upper_it != distanceMap.end());

		// If the upper bound is the first vertex, wrap it
		auto lower_it = upper_it;
		if (lower_it == distanceMap.begin())
			lower_it = distanceMap.end();
		--lower_it;

		// Set upper / lower bound
		outLowerBound = lower_it->second;
		outUpperBound = upper_it->second;

		// Return interpolated value
		return math::fit<float>(sample_distance, lower_it->first, upper_it->first, 0.0f, 1.0f);
	}


	void math::getNormalAlongLine(const std::map<float, int>& distanceMap, const std::vector<glm::vec3>& vertexNormals, float location, glm::vec3& outNormal)
	{
		int min_vertex, max_vertex;
		float lerp_v = getVertexLerpValue(distanceMap, location, min_vertex, max_vertex);

		// Get the values to interpolate
		const glm::vec3& lower_value = vertexNormals[min_vertex];
		const glm::vec3& upper_value = vertexNormals[max_vertex];

		// Make sure the normal has some length to it, otherwise the result is NAN
		// Also, you can't rotate a normal of no length so this is an error
		assert(glm::length(lower_value) >= nap::math::epsilon<float>());
		assert(glm::length(upper_value) >= nap::math::epsilon<float>());

		glm::vec3 n_lower_value = glm::normalize(lower_value);
		glm::vec3 n_higer_value = glm::normalize(upper_value);

		// If the normals are both the same the cross product will be a vector of 0
		// This will result in an invalid matrix, also, there's no reason to blend so return
		if (n_lower_value == n_higer_value)
		{
			outNormal = n_lower_value;
			return;
		}

		// Get the angle of max rotation
		float dot_product = glm::dot(n_higer_value, n_lower_value);
		float angle = glm::acos(glm::clamp<float>(dot_product, -1.0f, 1.0f));
		assert(!(glm::isnan(angle)));

		// Get the rotation vector
		glm::vec3 cross_product = glm::cross(n_higer_value, n_lower_value);

		// Calculate the matrix
		glm::mat3x3 rotation_matrix = glm::rotate(glm::mat4x4(), angle*lerp_v, cross_product);
		outNormal = n_lower_value * rotation_matrix;

		// Ensure the normal is valid
		assert(!(glm::isnan(outNormal).x));
		assert(!(glm::isnan(outNormal).y));
		assert(!(glm::isnan(outNormal).z));
	}

}
