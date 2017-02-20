#pragma once

// External Includes
#include <nap/serviceablecomponent.h>
#include <nap/rttinap.h>
#include <nap/coreattributes.h>

// Local Includes
#include "renderattributes.h"

namespace nap
{
	/**
	 * Describes a local transform that is used to compute 
	 * the global transform of an object. When the transform is created
	 * the global and local transform is invalid. You can always query the
	 * current local matrix, the global matrix is updated on compute.
	 */
	class TransformComponent : public ServiceableComponent
	{
		friend class RenderService;
		RTTI_ENABLE_DERIVED_FROM(ServiceableComponent)
	public:
		// Default constructor
		TransformComponent();
		
		// Attributes
		Attribute<glm::vec3>		translate		{ this,	"Translation",	{ 0.0, 0.0, 0.0 } };		// vector 3 - x, y z
		Attribute<glm::quat>		rotate			{ this, "Rotation" };								// quaternion - x, y, z, w
		Attribute<glm::vec3>		scale			{ this, "Scale",		{ 1.0, 1.0, 1.0 } };		// vector 3 - x, y, z
		
		// Uniform Scale
		Attribute<float>			uniformScale	{ this, "UniformScale", 1.0f };						// vector 3 - x, y, z

		/**
		 * Constructs and returns a local transform
		 * @return this transform local matrix
		 */
		const glm::mat4x4& getLocalTransform() const;

		/**
		 * Returns the global transform of this node
		 * Note that the global transform can be out of sync as it's
		 * recomputed on update. TODO: Resolve by walking up the tree
		 * and down till we have the highest dirty node, from that 
		 * point on resolve downwards
		 */
		const glm::mat4x4& getGlobalTransform() const;

		/**
		 * @return the parent transform, nullptr if this node
		 * has no parent transform
		 */
		nap::TransformComponent* getParentTransform();

		/**
		 * Sets the dirty flag
		 */
		void setDirty();

		/**
		 * @return if the local transform is dirty
		 */
		bool isDirty() const					{ return mLocalDirty; }

	protected:
		/**
		 * Sets dirty flags
		 */
		void onSetDirty(AttributeBase& object)		{ setDirty(); }

		/**
		 * Updates the global matrix based on the parent matrix
		 * If this or the parent matrix was marked dirty the
		 * global matrix of this node is recomputed. This is a recursive
		 * call
		 */
		void update(TransformComponent* parent = nullptr);

	private:
		/**
		 * Holds if the current node is dirty
		 */
		mutable bool mNodeDirty  = true;

		// Holds if the local matrix has been dirtied
		mutable bool mLocalDirty = true;

		/**
		* Link to parent transform
		* this link is automatically resolved when added
		* if the link can't be resolved it will be resolved immediately
		* this component will try to the next time it is called
		*/
		ObjectLinkAttribute parentTransform = { this, "ParentXForm", RTTI_OF(TransformComponent) };

		// Local / Global Matrices
		mutable glm::mat4x4 mLocalMatrix;							//< Local  Matrix
		mutable glm::mat4x4 mGlobalMatrix;							//< Global Matrix
		
		// SLots
		NSLOT(xformChanged, AttributeBase&, onSetDirty)


	};

} // nap

RTTI_DECLARE(nap::TransformComponent)