#pragma once

// Local Includes
#include "mathutils.h"

// External Includes
#include <vector>

namespace nap
{
	namespace math
	{
		/**
		* Returns an interpolated value along the line based on @location
		* Note that this function works best with equally distributed vertices
		* @param attr the polyline attribute to sample
		* @param parametric normalized location along the spline
		* @param if the line is closed or not
		* @param outValue the interpolated value
		*/
		template<typename T>
		void getValueAlongLine(const std::vector<T>& vertexData, float location, bool closed, T& outValue);

		/**
		* Utility function to up-sample a poly line to @segments
		* This function distributes the vertices equally among every segment, something that is not desirable with more uneven, complex lines
		* A weighted distribution method is preferred but for now this will do.
		* @param vertices the original mesh vertices
		* @param buffer the buffer that will hold the re-sampled vertices
		* @param segments the amount of segments the poly line should have, a segment is the line between two vertices
		* @param closed if the line is closed or not, if a line is not closed it will contain one extra point to maintain the last vertex
		* @return the total number of generated vertices
		*/
		template<typename T>
		int resampleLine(std::vector<T>& vertices, std::vector<T>& buffer, int segments, bool closed);


		//////////////////////////////////////////////////////////////////////////
		// Template definitions
		//////////////////////////////////////////////////////////////////////////

		template<typename T>
		void getValueAlongLine(const std::vector<T>& vertexData, float location, bool closed, T& outValue)
		{
			// Get vertex range, when the line is closed it means that we want to
			// include the first vertex as last
			int vert_count = static_cast<int>(vertexData.size());
			assert(vert_count > 0);
			int range = closed ? vert_count : vert_count - 1;

			// Get interpolation location
			float loc = location * static_cast<float>(range);

			// Get min and max point bounds, wrap the last vertex if the line is closed
			// ie: the last vertex is the first vertex
			int min_vertex = static_cast<int>(math::floor<float>(loc));
			int max_vertex = static_cast<int>(math::ceil<float>(loc));

			min_vertex = min_vertex % vert_count;
			max_vertex = max_vertex % vert_count;

			// Get lerp value that sits in between min / max
			float lerp_v = loc - static_cast<float>(static_cast<int>(loc));

			// Ensure points are in bounds
			assert(min_vertex <= vert_count - 1);
			assert(max_vertex <= vert_count - 1);

			// Lerp between min and max value
			const T& min_p_value = vertexData[min_vertex];
			const T& max_p_value = vertexData[max_vertex];

			outValue = math::lerp<T>(min_p_value, max_p_value, lerp_v);
		}


		template<typename T>
		int resampleLine(std::vector<T>& vertices, std::vector<T>& buffer, int segments, bool closed)
		{
			assert(segments > 0);
			int vertex_count = static_cast<int>(vertices.size());

			// If there not enough or an equal amount or less vertices, don't do anything
			if (segments <= vertex_count || vertex_count < 2)
			{
				buffer = vertices;
				return vertices.size();
			}

			// Figure out the amount of edges, closed lines have one extra edge (connecting first to last)
			int edge_count = closed ? vertex_count : vertex_count - 1;

			// Calculate the total amount of pointer for every side
			int pps = segments / edge_count;

			// Clear existing buffer data
			buffer.clear();

			// Reserve space for points to add
			buffer.reserve(segments);

			for (int i = 0; i < edge_count; i++)
			{
				// Get edge points
				T& point_one = vertices[i];
				T& point_two = i + 1 >= vertex_count ? vertices[0] : vertices[i + 1];

				// Add edge vertices
				for (int p = 0; p < pps; p++)
				{
					float inc = static_cast<float>(p) / static_cast<float>(pps);
					buffer.emplace_back(nap::math::lerp<T>(point_one, point_two, inc));
				}
			}

			// If the line is open add an additional point
			if (!closed)
			{
				buffer.emplace_back(vertices.back());
			}

			// Return total number of new points
			return buffer.size();
		}
	}
}
