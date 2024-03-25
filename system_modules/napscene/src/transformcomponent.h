/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// External Includes
#include "component.h"

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
		glm::vec3		mTranslate		= { 0.0f, 0.0f, 0.0f };				///< Property: 'Translate' Position (x, y, z)
		glm::vec3		mRotate 		= { 0.0f, 0.0f, 0.0f };				///< Property: 'Rotation' Amount of rotation in degrees (yaw, pitch, roll)
		glm::vec3		mScale			= { 1.0f, 1.0f, 1.0f };				///< Property: 'Scale' Axis scaling factor (x, y, z)
		float			mUniformScale	= 1.0f;								///< Property: 'UniformScale' Uniform scaling factor
	};


	/**
	 * Struct to cache a transform instance
	 * Note that the rotation is a quaternion unlike TransformProperties
	 */
	struct TransformInstanceProperties
	{
		// Default constructor
		TransformInstanceProperties() = default;

		// Constructor
		TransformInstanceProperties(const glm::vec3& translate, const glm::quat& rotate, const glm::vec3& scale, float uniformScale) :
			mTranslate(translate), mRotate(rotate), mScale(scale), mUniformScale(uniformScale) {}

		glm::vec3		mTranslate		= { 0.0f, 0.0f, 0.0f };				// The translation of this component in units
		glm::quat		mRotate			= { 0.0f, 0.0f, 0.0f, 1.0f };		// The amount of rotation in degrees (yaw, pitch, roll)											
		glm::vec3		mScale			= { 1.0f, 1.0f, 1.0f };				// The scale of this component
		float			mUniformScale	= 1.0f;								// The uniform scale of this component
	};


	/**
	 * Resource part of the transform component.
	 * Describes a local transform that is used to compute
	 * the global transform of an entity.
	 */
	class NAPAPI TransformComponent : public Component
	{
		RTTI_ENABLE(Component)
		DECLARE_COMPONENT(TransformComponent, TransformComponentInstance)

	public:
		TransformProperties mProperties;									///< Property: 'Properties', translate, rotate and scale
	};


	/**
	 * Describes the local transform of an entity, used to compute 
	 * the global transform of an entity at runtime. When the transform is created
	 * the global and local transform is invalid. You can always query the
	 * current local matrix, the global matrix is updated on update().
	 */
	class NAPAPI TransformComponentInstance : public ComponentInstance
	{
		RTTI_ENABLE(ComponentInstance)
	public:
		TransformComponentInstance(EntityInstance& entity, Component& resource) :
			ComponentInstance(entity, resource)
		{ }
        
        using ComponentInstance::update;

		/**
		 * Initializes this component.
		 * @param errorState The error object
		 */
		virtual bool init(utility::ErrorState& errorState);

		/**
		 * Constructs and returns the local transform.
		 * @return this transform local transformation matrix
		 */
		const glm::mat4x4& getLocalTransform() const;

		/**
		 * Set the local transform based on the given matrix.
		 * Note that the matrix is decomposed, result is stored in the individual elements:
		 * Uniform scale is discarded, ie: the result will be 1.
		 * @param matrix new local transformation matrix. 
		 */
		void setLocalTransform(const glm::mat4x4& matrix);

		/**
		 * Overrides the local transform without decomposing the matrix into individual elements.
		 * Transform, rotation and scale properties are not updated and will be out of sync.
		 * @param matrix new local transformation matrix. 
		 */
		void overrideLocalTransform(const glm::mat4x4& matrix);

		/**
		 * Returns the global transform of this node.
		 * Note that the global transform can be out of sync as it's recomputed on update.
		 * point on resolve downwards
		 */
		const glm::mat4x4& getGlobalTransform() const;

		/**
		 * When set dirty, the transform component will re-compute 
		 * the global and local transform matrices when requested.
		 */
		void setDirty();

		/**
		 * @return if the local transform is dirty.
		 */
		bool isDirty() const							{ return mWorldDirty; }

		/**
		 * Updates the global matrix based on the parent matrix
		 * If this or the parent matrix was marked dirty the
		 * global matrix of this node is recomputed. This is a recursive
		 * call
		 */
		void update(const glm::mat4& parentTransform);

		/**
		 * Sets the transformation part of this component.
		 * @param translate new component translation.
		 */
		void setTranslate(const glm::vec3& translate);
		
		/**
		 * @return component translation
		 */
		const glm::vec3& getTranslate() const			{ return mTranslate;  }

		/**
		 * Sets the rotation part of this component.
		 * @param rotate new component rotation.
		 */
		void setRotate(const glm::quat& rotate);
		
		/**
		 * @return component rotation
		 */
		const glm::quat& getRotate() const				{ return mRotate; }

		/**
		 * Sets the scale factor of the x, y and z axis of this component.
		 * Note that the uniform scale is applied after axis dependent scale factor.
		 * @param scale the new component scale.
		 */
		void setScale(const glm::vec3& scale);
		
		/**
		 * @return component scale
		 */
		const glm::vec3& getScale() const				{ return mScale; }

		/**
		 * Sets the uniform scale factor, applied to all axis.
		 * Note that the uniform scale is applied after the axis independent scale factor.
		 * @param scale the new component scale.
		 */
		void setUniformScale(float scale);
		
		/**
		 * @return uniform component scale.
		 */
		const float getUniformScale() const				{ return mUniformScale; }

		/**
		 * @return the current transform instance properties for i.e. caching and restoring
		 * the transform later.
		 */
		TransformInstanceProperties getInstanceProperties() const;

		/**
		 * Sets the instance properties of this camera
		 */
		void setInstanceProperties(const TransformInstanceProperties& props);

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
