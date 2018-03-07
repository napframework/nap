#include "depthsorter.h"

namespace nap
{

	DepthSorter::DepthSorter(EMode mode, const glm::mat4x4& viewMatrix) :
		mViewMatrix(viewMatrix),
		mMode(mode) { }


	bool DepthSorter::operator()(const nap::ComponentInstance* objectA, const nap::ComponentInstance* objectB)
	{
		// Get the transform of objectA in view space
		const nap::EntityInstance& entityA = *objectA->getEntityInstance();
		const nap::TransformComponentInstance& transformA = entityA.getComponent<nap::TransformComponentInstance>();
		const glm::mat4 view_space_a = mViewMatrix * transformA.getGlobalTransform();

		// Get the transform of objectB in view space
		const nap::EntityInstance& entityB = *objectB->getEntityInstance();
		const nap::TransformComponentInstance& transformB = entityB.getComponent<nap::TransformComponentInstance>();
		const glm::mat4 view_space_b = mViewMatrix * transformB.getGlobalTransform();

		// Get the z-component (i.e. depth) of both entities
		float a_z = view_space_a[3].z;
		float b_z = view_space_b[3].z;

		// Compare
		return mMode == EMode::BackToFront ? a_z < b_z : a_z > b_z;
	}

}