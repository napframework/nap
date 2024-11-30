#include "pancontroller.h"

// External Includes
#include <entity.h>
#include <inputcomponent.h>

// nap::pancontroller run time class definition 
RTTI_BEGIN_CLASS(nap::PanController)
	RTTI_PROPERTY("RenderWindow", &nap::PanController::mRenderWindow, nap::rtti::EPropertyMetaData::Required, "Window that displays the texture")
RTTI_END_CLASS

// nap::pancontrollerInstance run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::PanControllerInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


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
		mRenderTarget = resource->mRenderWindow.get();

		// Handle pointer input
		pointer_component->pressed.connect(std::bind(&PanControllerInstance::onMouseDown, this, std::placeholders::_1));
		pointer_component->moved.connect(std::bind(&PanControllerInstance::onMouseMove, this, std::placeholders::_1));
		pointer_component->released.connect(std::bind(&PanControllerInstance::onMouseUp, this, std::placeholders::_1));

		// Orthographic camera mode = pixels
		mOrthoCameraComponent->setMode(nap::EOrthoCameraMode::PixelSpace);

		return true;
	}


	void PanControllerInstance::update(double deltaTime)
	{

	}


	void PanControllerInstance::frameTexture(const glm::vec2& textureSize, nap::TransformComponentInstance& ioTextureTransform, float scale)
	{
		// Compute current frame ratios (buffer & texture)
		glm::vec2 buf_size = mRenderTarget->getBufferSize();
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


	void PanControllerInstance::frameTexture(const Texture2D& texture, nap::TransformComponentInstance& ioTextureTransform, float scale)
	{
		frameTexture(texture.getSize(), ioTextureTransform, scale);
	}


	void PanControllerInstance::onMouseDown(const PointerPressEvent& pointerPressEvent)
	{
		
	}


	void PanControllerInstance::onMouseUp(const PointerReleaseEvent& pointerReleaseEvent)
	{

	}


	void PanControllerInstance::onMouseMove(const PointerMoveEvent& pointerMoveEvent)
	{

	}

}
