/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "entity.h"
#include "component.h"
#include "transformcomponent.h"

namespace nap
{
	// Forward declares
	class RenderableComponentInstance;

	/**
	 * Helper class that can sort a list of components back to front or front to back.
	 */
	class NAPAPI DepthComparer
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
		DepthComparer(EMode mode, const glm::mat4x4& viewMatrix);

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


	namespace sorter
	{
		/**
		 * Sorts a set of renderable components based on distance to the camera, ie: depth
		 * Note that when the object is of a type mesh it will use the material to sort based on opacity
		 * If the renderable object is not a mesh the sorting will occur front-to-back regardless of it's type as we don't
		 * know the way the object is rendered to screen
		 * @param comps the renderable components to sort
		 * @param viewMatrix the camera view matrix used for sorting based on distance
		 */
		void NAPAPI sortObjectsByDepth(std::vector<RenderableComponentInstance*>& comps, const glm::mat4& viewMatrix);
	}
}
