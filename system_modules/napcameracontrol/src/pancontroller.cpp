/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local includes
#include "pancontroller.h"

// External includes
#include <entity.h>
#include <inputcomponent.h>

// nap::pancontroller run time class definition 
RTTI_BEGIN_CLASS(nap::PanController)
RTTI_PROPERTY("RenderWindow", &nap::PanController::mRenderWindow, nap::rtti::EPropertyMetaData::Required, "Window that displays the texture")
RTTI_PROPERTY("ZoomSpeed", &nap::PanController::mZoomSpeed, nap::rtti::EPropertyMetaData::Default, "Zoom speed")
RTTI_END_CLASS

// nap::pancontrollerInstance run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::PanControllerInstance)
RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS


namespace nap
{
	void PanController::getDependentComponents(std::vector<rtti::TypeInfo>& components) const
	{
		components.emplace_back(RTTI_OF(TransformComponent));
		components.emplace_back(RTTI_OF(PointerInputComponent));
		components.emplace_back(RTTI_OF(OrthoCameraComponent));
	}


	bool PanControllerInstance::init(utility::ErrorState& errorState)
	{
		// TransformComponent is required to move the entity
		mTransformComponent = getEntityInstance()->findComponent<TransformComponentInstance>();
		if (!errorState.check(mTransformComponent != nullptr, "%s: missing TransformComponent", mID.c_str()))
			return false;

		// Orthographic camera
		mOrthoCameraComponent = getEntityInstance()->findComponent<OrthoCameraComponentInstance>();
		if (!errorState.check(mOrthoCameraComponent != nullptr, "%s: missing OrthoCameraComponent", mID.c_str()))
			return false;

		// Pointer input
		PointerInputComponentInstance* pointer_component = getEntityInstance()->findComponent<PointerInputComponentInstance>();
		if (!errorState.check(pointer_component != nullptr, "%s: missing PointerInputComponent", mID.c_str()))
			return false;

		// Render target
		auto* resource = getComponent<PanController>();
		mWindow = resource->mRenderWindow.get();

		// Handle pointer input
		pointer_component->pressed.connect(std::bind(&PanControllerInstance::onMouseDown, this, std::placeholders::_1));
		pointer_component->moved.connect(std::bind(&PanControllerInstance::onMouseMove, this, std::placeholders::_1));
		pointer_component->released.connect(std::bind(&PanControllerInstance::onMouseUp, this, std::placeholders::_1));

		// Setup our orthographic camera
		mOrthoCameraComponent->setMode(nap::EOrthoCameraMode::Custom);
		mCameraProperties = mOrthoCameraComponent->getProperties();
		mCameraProperties.mNearClippingPlane = 1.0f;
		mCameraProperties.mFarClippingPlane  = 1.0f + cameraPosition.z;

		// Copy zoom speed
		mZoomSpeed = math::max<float>(math::epsilon<float>(), resource->mZoomSpeed);
		return true;
	}


	void PanControllerInstance::update(double deltaTime)
	{
		// Ensure camera planes are not distorted
		float window_ratio = mWindow->getRatio();
		if (math::abs<float>(window_ratio - mCameraProperties.getRatio()) > math::epsilon<float>())
			mCameraProperties.adjust(window_ratio);

		// Always update camera planes -> prevents us from having to deal with specific events (resize etc.)
		mOrthoCameraComponent->setProperties(mCameraProperties);
	}


	void PanControllerInstance::frameTexture(const Texture2D& texture, nap::TransformComponentInstance& ioTextureTransform, float scale)
	{
		frameTexture(texture.getSize(), ioTextureTransform, scale);
	}


	void PanControllerInstance::frameTexture(const glm::vec2& textureSize, nap::TransformComponentInstance& ioTextureTransform, float scale)
	{
		// Compute current frame ratios (buffer & texture)
		glm::vec2 buf_size = mWindow->getBufferSize();
		glm::vec2 tex_size = textureSize;
		glm::vec2 tar_scale;

		// Texture wider (ratio) -> horizontal leading
		glm::vec2 ratios = { buf_size.y / buf_size.x, tex_size.y / tex_size.x };
		if (ratios.x > ratios.y)
		{
			tar_scale.x = buf_size.x;
			tar_scale.y = buf_size.x * ratios.y;
		}
		// Texture taller (ratio) -> vertical leading
		else
		{
			tar_scale.x = buf_size.y / ratios.y;
			tar_scale.y = buf_size.y;
		}

		// Compute 2D (XY) position and update transform
		glm::vec2 tex_pos = { buf_size.x * 0.5f, buf_size.y * 0.5f };
		ioTextureTransform.setTranslate(glm::vec3(tex_pos, 0.0f));
		ioTextureTransform.setScale(glm::vec3(tar_scale * scale, 1.0f));
	}


	void PanControllerInstance::reset()
	{
		mTransformComponent->setTranslate(cameraPosition);
		mTransformComponent->setUniformScale(1.0f);
		mTransformComponent->setScale({ 1.0f, 1.0f, 1.0f });

		// Reset camera planes
		glm::vec2 buffer_size = mWindow->getBufferSize();
		mCameraProperties.mLeftPlane = 0.0f;
		mCameraProperties.mRightPlane = buffer_size.x;
		mCameraProperties.mBottomPlane = 0.0f;
		mCameraProperties.mTopPlane = buffer_size.y;
	}


	float PanControllerInstance::getZoomLevel() const
	{
		return mCameraProperties.getWidth() /
			static_cast<float>(mWindow->getBufferSize().x);
	}


	void PanControllerInstance::onMouseDown(const PointerPressEvent& pointerPressEvent)
	{
		assert(pointerPressEvent.mWindow == mWindow->getNumber());
		switch (pointerPressEvent.mButton)
		{
			case nap::PointerClickEvent::EButton::LEFT:
			{
				mClickCoordinates = { pointerPressEvent.mX, pointerPressEvent.mY };
				mPan = true;
				break;
			}
			case nap::PointerClickEvent::EButton::RIGHT:
			{
				mClickCoordinates = { pointerPressEvent.mX, pointerPressEvent.mY };
				mZoom = true;
				break;
			}
			default:
			{
				break;
			}
		}
	}


	void PanControllerInstance::onMouseUp(const PointerReleaseEvent& pointerReleaseEvent)
	{
		assert(pointerReleaseEvent.mWindow == mWindow->getNumber());
		switch (pointerReleaseEvent.mButton)
		{
			case nap::PointerClickEvent::EButton::LEFT:
			{
				mPan = false;
				break;
			}
			case nap::PointerClickEvent::EButton::RIGHT:
			{
				mZoom = false;
				break;
			}
			default:
			{
				break;
			}
		}
	}


	void PanControllerInstance::onMouseMove(const PointerMoveEvent& pointerMoveEvent)
	{
		assert(pointerMoveEvent.mWindow == mWindow->getNumber());
		if (mPan)
			panCamera(mClickCoordinates,
				{ pointerMoveEvent.mX, pointerMoveEvent.mY }, {pointerMoveEvent.mRelX, pointerMoveEvent.mRelY}
			);
		if (mZoom)
		{
			zoomCamera(mClickCoordinates,
				{ pointerMoveEvent.mX, pointerMoveEvent.mY }, { pointerMoveEvent.mRelX, pointerMoveEvent.mRelY }
			);
		}
	}


	void PanControllerInstance::panCamera(const glm::vec2& clickPosition, glm::vec2&& position, glm::vec2&& relMovement)
	{
		glm::vec2 translate = relMovement * getZoomLevel();
		glm::vec3 xform = mTransformComponent->getTranslate();
		mTransformComponent->setTranslate(xform - glm::vec3(translate, 0.0f));
	}


	void PanControllerInstance::zoomCamera(const glm::vec2& clickPosition, glm::vec2&& position, glm::vec2&& relMovement)
	{
		// Compute amount to zoom in or out
		glm::vec2 buffer_size = mWindow->getBufferSize();
		float amount = glm::dot({ 1.0f, 0.0f }, relMovement) * mZoomSpeed * getZoomLevel();
		glm::vec2 zoom = { amount, amount * mWindow->getRatio() };

		// Compute new plane offsets based on click weight (normalized distribution)
		// Allows us to zoom into a specific coordinate, instead of the center
		glm::vec2 click_weight = clickPosition / buffer_size;
		glm::vec2 x_offset = { zoom.x * click_weight.x, -(zoom.x * (1.0f - click_weight.x)) };
		glm::vec2 y_offset = { zoom.y * click_weight.y, -(zoom.y * (1.0f - click_weight.y)) };

		// Set new planes
		mCameraProperties.mLeftPlane += x_offset.x;
		mCameraProperties.mRightPlane += x_offset.y;
		mCameraProperties.mBottomPlane += y_offset.x;
		mCameraProperties.mTopPlane += y_offset.y;
	}
}
