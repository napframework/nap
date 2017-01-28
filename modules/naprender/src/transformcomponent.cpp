// local includes
#include "transformcomponent.h"

// External includes
#include <glm/gtx/transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include <mathutils.h>

namespace nap
{
	// Initialize identity
	const glm::mat4x4 sIdentyMatrix = glm::mat4x4();

	// Constructor
	TransformComponent::TransformComponent()
	{
		// Set flag for link
		parentTransform.setFlag(nap::ObjectFlag::Visible, false);
	
		// Connect attributes for dirty
		translate.valueChanged.connect(xformChanged);
		scale.valueChanged.connect(xformChanged);
		rotate.valueChanged.connect(xformChanged);
		uniformScale.valueChanged.connect(xformChanged);
	}


	// Constructs and returns this components local transform
	const glm::mat4x4& TransformComponent::getLocalTransform() const
	{
		if (mLocalDirty)
		{
			glm::mat4x4 xform_matrix = glm::translate(glm::mat4x4(), translate.getValue());
			glm::mat4x4 rotat_matrix = glm::toMat4(vectorToQuat(rotate.getValue()));
			glm::mat4x4 scale_matrix = glm::scale(glm::mat4x4(), scale.getValue() * uniformScale.getValue());
			mLocalMatrix = (xform_matrix * rotat_matrix * scale_matrix);
			mLocalDirty  = false;
		}
		return mLocalMatrix;
	}


	// Return the global transform
	const glm::mat4x4& TransformComponent::getGlobalTransform() const
	{
		if (isDirty())
			nap::Logger::warn(*this, "global matrix out of sync");
		return mGlobalMatrix;
	}


	// Returns the parent transform, nullptr if this node has no transform
	nap::TransformComponent* TransformComponent::getParentTransform()
	{
		// Get this parent
		nap::Entity* parent = this->getParent();
		assert(parent != nullptr);

		// Get transform parent
		nap::Entity* parent_entity = parent->getParent();

		// If there's no parent this object is floating around
		// somewhere, probably not intentional
		if (parent_entity == nullptr)
		{
			nap::Logger::warn(*this, "unable to resolve parent entity, this object appears to be floating");
			return nullptr;
		}

		// Otherwise try to find transform
		TransformComponent* parent_xform = parent_entity->getComponent<TransformComponent>();
		if (parent_xform == nullptr)
		{
			nap::Logger::warn(*this, "parent entity has no transform");
			return nullptr;
		}

		return parent_xform;
	}

	// Sets local flag dirty
	void TransformComponent::setDirty()
	{
		mLocalDirty = true;
		mNodeDirty  = true;
	}


	// Updates it's global and local matrix
	void TransformComponent::update(TransformComponent* parent)
	{
		// Check if it's dirty
		bool parent_dirty = parent != nullptr ? parent->mNodeDirty : false;

		// Update global matrix if parent is dirty or we're dirty
		if (mNodeDirty | parent_dirty)
		{
			const glm::mat4x4& parent_mat = parent == nullptr ? sIdentyMatrix : parent->getGlobalTransform();
			mGlobalMatrix = parent_mat * getLocalTransform();
		}

		// Update dirty based on parent and locality
		// Necessary for other components to check dirty state
		mNodeDirty |= parent_dirty;

		// Get child transforms and update
		nap::Entity* parent_entity = getParent();
		assert(parent_entity != nullptr);

		// Update child xform components: TODO: auto compute
		for (auto& e : parent_entity->getEntities())
		{
			for (auto& xform : e->getComponentsOfType<TransformComponent>())
				xform->update();
		}

		// Change dirty flag
		mNodeDirty = false;
	}
}
RTTI_DEFINE(nap::TransformComponent)