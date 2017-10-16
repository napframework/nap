#include "orthocontroller.h"
#include "inputevent.h"
#include "inputcomponent.h"
#include "transformcomponent.h"
#include <nap/entity.h>
#include <mathutils.h>

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>


RTTI_BEGIN_CLASS(nap::OrthoController)
	RTTI_PROPERTY("MovementSpeed",			&nap::OrthoController::mMovementSpeed,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("OrthoCameraComponent",	&nap::OrthoController::mOrthoCameraComponent,	nap::rtti::EPropertyMetaData::Default)
	RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::OrthoControllerInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

namespace nap
{
	OrthoControllerInstance::OrthoControllerInstance(EntityInstance& entity, Component& resource) :
		ComponentInstance(entity, resource)
	{
	}


	bool OrthoControllerInstance::init(EntityCreationParameters& entityCreationParams, utility::ErrorState& errorState)
	{
		// TransformComponent is required to move the entity
		mTransformComponent = getEntityInstance()->findComponent<TransformComponentInstance>();
		if (!errorState.check(mTransformComponent != nullptr, "Could not find transform component"))
			return false;

		PointerInputComponentInstance* pointer_component = getEntityInstance()->findComponent<PointerInputComponentInstance>();
		if (!errorState.check(pointer_component != nullptr, "Could not find PointerInputComponent"))
			return false;

		pointer_component->pressed.connect(std::bind(&OrthoControllerInstance::onMouseDown, this, std::placeholders::_1));
		pointer_component->moved.connect(std::bind(&OrthoControllerInstance::onMouseMove, this, std::placeholders::_1));
		pointer_component->released.connect(std::bind(&OrthoControllerInstance::onMouseUp, this, std::placeholders::_1));

		getComponent<OrthoController>()->mOrthoCameraComponent->setMode(OrthoCameraComponentInstance::EMode::CorrectAspectRatio);

		return true;
	}

	CameraComponentInstance& OrthoControllerInstance::getCameraComponent()
	{
		return *getComponent<OrthoController>()->mOrthoCameraComponent;
	}

	void OrthoControllerInstance::updateCameraProperties()
	{
		nap::OrthoCameraProperties camera_properties = getComponent<OrthoController>()->mOrthoCameraComponent->getProperties();
		float half_scale = mCameraScale * 0.5f;
		camera_properties.mLeftPlane = -half_scale;
		camera_properties.mRightPlane = half_scale;
		camera_properties.mTopPlane = -half_scale;
		camera_properties.mBottomPlane = half_scale;
		getComponent<OrthoController>()->mOrthoCameraComponent->setProperties(camera_properties);
	}

	void OrthoControllerInstance::enable(const glm::vec3& cameraPos, const glm::quat& cameraRotate)
	{
		mTransformComponent->setTranslate(cameraPos);
		mTransformComponent->setRotate(cameraRotate);

		updateCameraProperties();

		mEnabled = true;
	}

	void OrthoControllerInstance::onMouseDown(const PointerPressEvent& pointerPressEvent)
	{
		if (!mEnabled)
			return;

		if (pointerPressEvent.mButton == EMouseButton::LEFT)
		{
			mMode = EMode::Pan;
		}
		else if (pointerPressEvent.mButton == EMouseButton::RIGHT)
		{
			mMode = EMode::Zoom;
			mMousePosAtClick = glm::vec2(pointerPressEvent.mX, pointerPressEvent.mY);
			mCameraScaleAtClick = mCameraScale;
		}
	}


	void OrthoControllerInstance::onMouseUp(const PointerReleaseEvent& pointerReleaseEvent)
	{
		mMode = EMode::None;
	}


	void OrthoControllerInstance::onMouseMove(const PointerMoveEvent& pointerMoveEvent)
	{
		if (mMode == EMode::Pan)
		{
			glm::ivec2 render_target_size = getComponent<OrthoController>()->mOrthoCameraComponent->getRenderTargetSize();
			float aspect_ratio = (float)render_target_size.y / (float)render_target_size.x;
			glm::vec2 mouse_scale = glm::vec2(mCameraScale / (float)render_target_size.x, (mCameraScale * aspect_ratio) / (float)render_target_size.y);

			const glm::mat4& camera_transform = mTransformComponent->getGlobalTransform();
			glm::vec3 right_translate = (float)-pointerMoveEvent.mRelX * mouse_scale.x * camera_transform[0];
			glm::vec3 up_translate = (float)-pointerMoveEvent.mRelY * mouse_scale.y * camera_transform[1];

			mTransformComponent->setTranslate(mTransformComponent->getTranslate() + right_translate + up_translate);
		}
		else if (mMode == EMode::Zoom)
		{
			int pointerMove = pointerMoveEvent.mRelX;
			if (abs(pointerMoveEvent.mRelY) > abs(pointerMoveEvent.mRelX))
				pointerMove = pointerMoveEvent.mRelY;

			mCameraScale += (float)pointerMove;
			mCameraScale = glm::max(1.0f, mCameraScale);

//			mCameraScaleAtClick - mCameraScale;

// 			const glm::mat4& camera_transform = mTransformComponent->getGlobalTransform();
// 			glm::vec3 right_translate = (float)-pointerMoveEvent.mRelX * mouse_scale.x * camera_transform[0];
// 			glm::vec3 up_translate = (float)-pointerMoveEvent.mRelY * mouse_scale.y * camera_transform[1];
// 
// 			mTransformComponent->setTranslate(mTransformComponent->getTranslate() + right_translate + up_translate);


			updateCameraProperties();
		}
	}
}