#pragma once

#include "nap/componentinstance.h"

namespace nap
{
	class FirstPersonController;
	class KeyPressEvent;
	class KeyReleaseEvent;
	class TransformComponent;
	class TransformComponentResource;
	class KeyInputComponent;
	class KeyInputComponentResource;

	/**
	 * Resource for the FirstPersonController
	 */
	class FirstPersonControllerResource : public ComponentResource
	{
		RTTI_ENABLE(ComponentResource)
	public:
		/**
		 * Get the type of ComponentInstance to create
		 */
		virtual const rtti::TypeInfo getInstanceType() const 
		{
			return RTTI_OF(FirstPersonController); 
		}

		/**
		 * Get the types of components on which this component depends
		 */
		virtual void getDependentComponents(std::vector<rtti::TypeInfo>& components) 
		{
			components.push_back(RTTI_OF(TransformComponentResource));
			components.push_back(RTTI_OF(KeyInputComponentResource));
		}

		float mMovementSpeed	= 3.0f;		// The speed with which to move
		float mRotateSpeed		= 1.0f;		// The speed with which to rotate
	};


	/**
	 * The FirstPersonController is a component that implements first-person movement for the entity it is attached to.
	 * It uses the TransformComponent to move the entity and the InputComponent to receive input
	 *
	 * WASD to move, arrow keys to rotate
	 */
	class FirstPersonController : public ComponentInstance
	{
		RTTI_ENABLE(ComponentInstance)
	public:
		FirstPersonController(EntityInstance& entity);

		/**
		 * Initialize this ComponentInstance
		 */
		virtual bool init(const ObjectPtr<ComponentResource>& resource, EntityCreationParameters& entityCreationParams, utility::ErrorState& errorState) override;

		/**
		 * Update this ComponentInstance
		 */
		virtual void update(double deltaTime) override;

	private:
		/**
		 * Handler for key press events
		 */
		void onKeyPress(const KeyPressEvent& keyPressEvent);

		/**
		 * Handler for key release events
		 */
		void onKeyRelease(const KeyReleaseEvent& keyReleaseEvent);

	private:
		FirstPersonControllerResource*	mResource			= nullptr;		// The resource containing the movement properties
		TransformComponent*				mTransformComponent = nullptr;		// The transform component used to move the entity
		bool							mMoveForward		= false;		// Whether we're moving forward
		bool							mMoveBackward		= false;		// Whether we're moving backwards
		bool							mMoveLeft			= false;		// Whether we're moving left
		bool							mMoveRight			= false;		// Whether we're moving right
		bool							mLookUp				= false;		// Whether we're rotating up
		bool							mLookDown			= false;		// Whether we're rotating down
		bool							mLookLeft			= false;		// Whether we're rotating left
		bool							mLookRight			= false;		// Whether we're rotating right
	};

}