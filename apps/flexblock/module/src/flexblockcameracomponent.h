#pragma once

#include <perspcameracomponent.h>

namespace nap
{
	class FlexblockCameraComponentInstance;

	/**
	 * Extends on perpective camera, offsets can be given to adjust the viewport of the camera
	 */
	class NAPAPI FlexblockCameraComponent : public PerspCameraComponent
	{
		RTTI_ENABLE(PerspCameraComponent)
		DECLARE_COMPONENT(FlexblockCameraComponent, FlexblockCameraComponentInstance)
	public:
		float mLeftOffset = 0.0f; ///< Property: 'Left Offset' offset from the middle to right. F.E. a 0.25 value will offset the viewport 25% to the right
		float mTopOffset = 0.0f; ///< Property: 'Top Offset' offset from the middle to the bottom. F.E. a 0.25 value will lower the viewport 25% 
	};

	/**
	* Implementation of the FlexblockCamera
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
