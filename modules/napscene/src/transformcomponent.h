#pragma once

// External Includes
#include <nap/serviceablecomponent.h>
#include <nap/rttinap.h>
#include <nap/coreattributes.h>
#include <nap/componentinstance.h>

// Local Includes
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace nap
{
	class TransformComponent;

	/** 
	 * Struct to hold properties shared between Resource and Instance
	 */
	struct TransformProperties
	{
		glm::vec3		mTranslate;											// The translation of this component
		glm::quat		mRotate;											// The rotation of this component
		glm::vec3		mScale			= glm::vec3(1.0f, 1.0f, 1.0f);		// The scale of this component
		float			mUniformScale	= 1.0f;								// The uniform scale of this component
	};


	/**
	 * Resource for the TransformComponent
	 */
	class TransformComponentResource : public ComponentResource
	{
		RTTI_ENABLE(ComponentResource)
	
	public:
		/**
		 * Get the type of ComponentInstance to create
		 */
		virtual const rtti::TypeInfo getInstanceType() const
		{ 
			return RTTI_OF(TransformComponent); 
		}

	public:
		TransformProperties mProperties;
	};


	/**
	 * Describes a local transform that is used to compute 
	 * the global transform of an object. When the transform is created
	 * the global and local transform is invalid. You can always query the
	 * current local matrix, the global matrix is updated on compute.
	 */
	class TransformComponent : public ComponentInstance
	{
		RTTI_ENABLE(ComponentInstance)
	public:
		TransformComponent(EntityInstance& entity) :
			ComponentInstance(entity)
		{
		}

		bool init(const ObjectPtr<ComponentResource>& resource, utility::ErrorState& errorState);

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
		bool isDirty() const					{ return mWorldDirty; }

		/**
		 * Updates the global matrix based on the parent matrix
		 * If this or the parent matrix was marked dirty the
		 * global matrix of this node is recomputed. This is a recursive
		 * call
		 */
		void update(const glm::mat4& parentTransform);

		void setTranslate(const glm::vec3& translate);
		const glm::vec3& getTranslate() const { return mProperties.mTranslate;  }

		void setRotate(const glm::quat& rotate);
		const glm::quat& getRotate() const { return mProperties.mRotate; }

		void setScale(const glm::vec3& scale);
		const glm::vec3& getScale() const { return mProperties.mScale; }

		void setUniformScale(float scale);
		const float getUniformScale() const { return mProperties.mUniformScale; }

	protected:
		/**
		 * Sets dirty flags
		 */
		void onSetDirty(AttributeBase& object)		{ setDirty(); }

	private:
		/**
		 * Holds if the current node is dirty
		 */
		mutable bool mWorldDirty  = true;

		// Holds if the local matrix has been dirtied
		mutable bool mLocalDirty = true;

		// Local / Global Matrices
		mutable glm::mat4x4 mLocalMatrix;							//< Local  Matrix
		mutable glm::mat4x4 mGlobalMatrix;							//< Global Matrix
		
		TransformProperties mProperties;
	};

} // nap
