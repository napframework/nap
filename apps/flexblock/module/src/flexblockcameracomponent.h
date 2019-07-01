#pragma once

#include <perspcameracomponent.h>

namespace nap
{
	class FlexblockCameraComponentInstance;

	/**
	* Resource class for the perspective camera. Holds static data as read from file.
	*/
	class NAPAPI FlexblockCameraComponent : public PerspCameraComponent
	{
		RTTI_ENABLE(PerspCameraComponent)
		DECLARE_COMPONENT(FlexblockCameraComponent, FlexblockCameraComponentInstance)
	public:
		float mLeftOffset = 0.0f;
		float mTopOffset = 0.0f;
	};

	/**
	* Implementation of the perspective camera. The view matrix is calculated using the transform attached to the entity.
	*/
	class NAPAPI FlexblockCameraComponentInstance : public PerspCameraComponentInstance
	{
		RTTI_ENABLE(PerspCameraComponentInstance)
	public:
		FlexblockCameraComponentInstance(EntityInstance& entity, Component& resource);
		~FlexblockCameraComponentInstance();

		virtual bool init(utility::ErrorState& errorState) override;

		/**
		* @return camera projection matrix
		* Use this matrix to transform a 3d scene in to a 2d projection
		*/
		virtual const glm::mat4& getProjectionMatrix() const override;
	private:
		float mLeftOffset = 0.0f;
		float mTopOffset = 0.0f;
	};
}
