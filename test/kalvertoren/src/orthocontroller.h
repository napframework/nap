#pragma once

#include "nap/component.h"
#include "nap/componentptr.h"
#include "orthocameracomponent.h"
#include <glm/glm.hpp>

namespace nap
{
	class OrthoControllerInstance;
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
	class OrthoController : public Component
	{
		RTTI_ENABLE(Component)
		DECLARE_COMPONENT(OrthoController, OrthoControllerInstance)
	public:
		/**
		* Get the types of components on which this component depends
		*/
		virtual void getDependentComponents(std::vector<rtti::TypeInfo>& components) const override
		{
			components.push_back(RTTI_OF(TransformComponent));
			components.push_back(RTTI_OF(KeyInputComponent));
		}

		float										mZoomSpeed = 0.5f;		// The speed with which to move
		ComponentPtr<nap::OrthoCameraComponent>		mOrthoCameraComponent;
	};


	/**
	*
	*/
	class OrthoControllerInstance : public ComponentInstance
	{
		RTTI_ENABLE(ComponentInstance)
	public:
		OrthoControllerInstance(EntityInstance& entity, Component& resource);

		/**
		* Initialize this ComponentInstance
		*/
		virtual bool init(EntityCreationParameters& entityCreationParams, utility::ErrorState& errorState) override;

		void enable(const glm::vec3& cameraPos, const glm::quat& cameraRotate);
		void disable() { mEnabled = false; }

		CameraComponentInstance& getCameraComponent();

	private:
		void onMouseDown(const PointerPressEvent& pointerPressEvent);
		void onMouseUp(const PointerReleaseEvent& pointerReleaseEvent);
		void onMouseMove(const PointerMoveEvent& pointerMoveEvent);
		void updateCameraProperties();

	private:
		enum EMode
		{
			None,
			Pan,
			Zoom
		};

		TransformComponentInstance*		mTransformComponent = nullptr;		// The transform component used to move the entity
		bool							mEnabled = false;
		float							mCameraScale = 50.0f;
		float							mCameraScaleAtClick = 0.0f;
		EMode							mMode = EMode::None;
		glm::vec2						mMousePosAtClick;
		glm::vec3						mTranslateAtClick;
	};

}