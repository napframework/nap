#pragma once

#include "entity.h"
#include "component.h"
#include "transformcomponent.h"

namespace nap
{
	/**
	 * Helper class that can sort a list of components back to front or front to back.
	 */
	class NAPAPI DepthSorter
	{
	public:
		enum class EMode
		{
			FrontToBack = 0,			///< Sort front to back
			BackToFront					///< Sort back to front
		};

		/**
		 * Default constructor
		 * @param mode the sort mode to use
		 * @param viewMatrix the camera location in world space
		 */
		DepthSorter(EMode mode, const glm::mat4x4& viewMatrix);

		/**
		 * Compares location of objectA to objectB based on the current sort mode
		 * @param objectA the first object to compare against
		 * @param objectB the second object to compare against
		 * @return if objectA is closer or further away than objectB based on the current sort mode
		 */
		bool operator()(const nap::ComponentInstance* objectA, const nap::ComponentInstance* objectB);

	private:
		const glm::mat4x4& mViewMatrix;
		EMode mMode = EMode::FrontToBack;
	};
}