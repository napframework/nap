#pragma once

// External Includes
#include <nap/component.h>
#include <nap/componentptr.h>
#include <inputcomponent.h>
#include <orthocameracomponent.h>
#include <glm/glm.hpp>

namespace nap
{
	class OrthoControllerInstance;
	class PointerPressEvent;
	class PointerMoveEvent;
	class PointerReleaseEvent;
	class TransformComponentInstance;
	class TransformComponent;

	/**
	* Resource for the OrbitController
	*/
	class NAPAPI OrthoController : public Component
	{
		RTTI_ENABLE(Component)
		DECLARE_COMPONENT(OrthoController, OrthoControllerInstance)
	public:
		/**
 		 * Get the types of components on which this component depends
		 */
		virtual void getDependentComponents(std::vector<rtti::TypeInfo>& components) const override;

		float										mZoomSpeed = 0.5f;		// The speed with which to move
		ComponentPtr<nap::OrthoCameraComponent>		mOrthoCameraComponent;	// Camera that we're controlling
	};


	/**
	 * The OrthoController is a component that implements orthographic camera control for the entity it is attached to.
	 * It uses the TransformComponent to move the entity and the InputComponent to receive input
	 *
	 * Hold left mouse button to pan, right mouse button to zoom.
	 */
	class NAPAPI OrthoControllerInstance : public ComponentInstance
	{
		RTTI_ENABLE(ComponentInstance)
	public:
		OrthoControllerInstance(EntityInstance& entity, Component& resource);

		/**
		 * Initialize this ComponentInstance
		 */
		virtual bool init(EntityCreationParameters& entityCreationParams, utility::ErrorState& errorState) override;

		/**
		 * Enables controlling of the camera while setting the position and rotation.
		 * @param cameraPos Camera translation to set.
		 * @param cameraRotate Camera rotation to set.
		 */
		void enable(const glm::vec3& cameraPos, const glm::quat& cameraRotate);

		/**
		 * Disables controlling of the camera.
		 */
		void disable() { mEnabled = false; }

		/**
		 * @return The orthographic camera component that we're controlling.
		 */
		CameraComponentInstance& getCameraComponent();

	private:

		/**
		 * Handler for mouse down events
		 */
		void onMouseDown(const PointerPressEvent& pointerPressEvent);

		/**
		* Handler for mouse up events
		*/
		void onMouseUp(const PointerReleaseEvent& pointerReleaseEvent);

		/**
		* Handler for mouse move events
		*/
		void onMouseMove(const PointerMoveEvent& pointerMoveEvent);

		/**
		 * Updates camera planes based on pan and zoom
		 */
		void updateCameraProperties();

	private:
		enum EMode
		{
			None,
			Pan,		// Currently panning
			Zoom		// Currently zooming
		};

		TransformComponentInstance*		mTransformComponent = nullptr;		// The transform component used to move the entity
		bool							mEnabled = true;					// Set if enabled for input
		float							mCameraScale = 50.0f;				// Current scale, selects the width of the space you can see
		float							mCameraScaleAtClick = 0.0f;			// Scale that was set when clicking with the mouse button
		EMode							mMode = EMode::None;				// Pan/Zoom mode
		glm::vec2						mMousePosAtClick;					// Mouse position that was set when clicking with the mouse button
		glm::vec3						mTranslateAtClick;					// Camera translation that was set when clicking with the mouse button
	};

}