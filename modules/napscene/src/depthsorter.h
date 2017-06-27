#pragma once

#include "nap/entityinstance.h"
#include "nap/componentinstance.h"
#include "transformcomponent.h"

namespace nap
{
	/**
	 * Helper class that can sort a list of components back to front or front to back.
	 */
	class DepthSorter
	{
	public:
		enum class EMode
		{
			FrontToBack,
			BackToFront
		};

		DepthSorter(EMode mode, const glm::mat4x4& viewMatrix) :
			mViewMatrix(viewMatrix),
			mMode(mode)
		{
		}

		bool operator()(const nap::ComponentInstance* objectA, const nap::ComponentInstance* objectB)
		{
			// Get the transform of objectA in view space
			const nap::EntityInstance& entityA			= *objectA->getEntity();
			const nap::TransformComponent& transformA	= entityA.getComponent<nap::TransformComponent>();
			const glm::mat4 view_space_a				= mViewMatrix * transformA.getGlobalTransform();

			// Get the transform of objectB in view space
			const nap::EntityInstance& entityB			= *objectB->getEntity();
			const nap::TransformComponent& transformB	= entityB.getComponent<nap::TransformComponent>();
			const glm::mat4 view_space_b				= mViewMatrix * transformB.getGlobalTransform();

			// Get the z-component (i.e. depth) of both entities
			float a_z = view_space_a[3].z;
			float b_z = view_space_b[3].z;
			
			// Compare
			if (mMode == EMode::BackToFront)
				return a_z < b_z;
			else
				return a_z > b_z;
		}

	private:
		const glm::mat4x4& mViewMatrix;
		EMode mMode;
	};
}