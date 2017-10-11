#pragma once

#include "nap/component.h"
#include <glm/glm.hpp>

namespace nap
{
	class OrbitControllerInstance;
	class KeyPressEvent;
	class KeyReleaseEvent;
	class PointerPressEvent;
	class PointerMoveEvent;
	class PointerReleaseEvent;
	class TransformComponentInstance;
	class TransformComponent;
	class KeyInputComponent;
	class KeyInputComponent;

	/**
	* Resource for the OrbitController
	*/
	class OrbitController : public Component
	{
		RTTI_ENABLE(Component)
			DECLARE_COMPONENT(OrbitController, OrbitControllerInstance)
	public:
		/**
		* Get the types of components on which this component depends
		*/
		virtual void getDependentComponents(std::vector<rtti::TypeInfo>& components) const override
		{
			components.push_back(RTTI_OF(TransformComponent));
			components.push_back(RTTI_OF(KeyInputComponent));
		}

		float mMovementSpeed = 0.5f;		// The speed with which to move
		float mRotateSpeed = 0.005f;		// The speed with which to rotate
	};


	/**
	* The FirstPersonController is a component that implements first-person movement for the entity it is attached to.
	* It uses the TransformComponent to move the entity and the InputComponent to receive input
	*
	* WASD to move, arrow keys to rotate
	*/
	class OrbitControllerInstance : public ComponentInstance
	{
		RTTI_ENABLE(ComponentInstance)
	public:
		OrbitControllerInstance(EntityInstance& entity, Component& resource);

		/**
		* Initialize this ComponentInstance
		*/
		virtual bool init(EntityCreationParameters& entityCreationParams, utility::ErrorState& errorState) override;

		void enable(bool enabled) { mEnabled = enabled; }
		void setLookAtPos(const glm::vec3& lookAtPos) { mLookAtPos = lookAtPos; }

	private:
		void onMouseDown(const PointerPressEvent& pointerPressEvent);
		void onMouseMove(const PointerMoveEvent& pointerMoveEvent);
		void onMouseUp(const PointerReleaseEvent& pointerReleaseEvent);

		void startDrag();

	private:
		enum class EMode
		{
			Idle,
			Rotating,
			Zooming
		};

		TransformComponentInstance*		mTransformComponent = nullptr;		// The transform component used to move the entity
		EMode							mMode = EMode::Idle;
		glm::vec3						mLookAtPos;
		bool							mEnabled = false;
	};

}