#pragma once

// External Includes
#include <nap/component.h>

// Local Includes
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace nap
{
	class TransformComponentInstance;

	/** 
	 * Struct to hold properties shared between Resource and Instance
	 * Note that the rotation is in euler angles (Pitch, Yaw, Roll) but after instantiation all 
	 * rotate related functionality is performed using quaternions
	 */
	struct TransformProperties
	{
		glm::vec3		mTranslate		= glm::vec3(0.0f, 0.0f, 0.0f);		// The translation of this component in units
		glm::vec3		mRotate 		= glm::vec3(0.0f, 0.0f, 0.0f);		// The amount of rotation in degrees (yaw, pitch, roll)											
		glm::vec3		mScale			= glm::vec3(1.0f, 1.0f, 1.0f);		// The scale of this component
		float			mUniformScale	= 1.0f;								// The uniform scale of this component
	};


	/**
	 * Resource for the TransformComponent
	 */
	class NAPAPI TransformComponent : public Component
	{
		RTTI_ENABLE(Component)
	
	public:
		/**
		 * Get the type of ComponentInstance to create
		 */
		virtual const rtti::TypeInfo getInstanceType() const override
		{ 
			return RTTI_OF(TransformComponentInstance); 
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
	class NAPAPI TransformComponentInstance : public ComponentInstance
	{
		RTTI_ENABLE(ComponentInstance)
	public:
		TransformComponentInstance(EntityInstance& entity) :
			ComponentInstance(entity)
		{
		}
        
        using ComponentInstance::update;

		/**
		* Initialize this component from its resource
		*
		* @param resource The resource we're being instantiated from
		* @param entityCreationParams Parameters required to create new entity instances during init
		* @param errorState The error object
		*/
		virtual bool init(const ObjectPtr<Component>& resource, EntityCreationParameters& entityCreationParams, utility::ErrorState& errorState);

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
		nap::TransformComponentInstance* getParentTransform();

		/**
		 * Sets the dirty flag
		 */
		void setDirty();

		/**
		 * @return if the local transform is dirty
		 */
		bool isDirty() const							{ return mWorldDirty; }

		/**
		 * Updates the global matrix based on the parent matrix
		 * If this or the parent matrix was marked dirty the
		 * global matrix of this node is recomputed. This is a recursive
		 * call
		 */
		void update(const glm::mat4& parentTransform);

		void setTranslate(const glm::vec3& translate);
		const glm::vec3& getTranslate() const			{ return mTranslate;  }

		void setRotate(const glm::quat& rotate);
		const glm::quat& getRotate() const				{ return mRotate; }

		void setScale(const glm::vec3& scale);
		const glm::vec3& getScale() const				{ return mScale; }

		void setUniformScale(float scale);
		const float getUniformScale() const				{ return mUniformScale; }

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
		
		// Local instance properties
		glm::vec3	mTranslate		= glm::vec3(0.0f, 0.0f, 0.0f);
		glm::quat	mRotate			= glm::quat();
		glm::vec3	mScale			= glm::vec3(1.0f, 1.0f, 1.0f);
		float		mUniformScale	= 1.0f;
	};

} // nap
