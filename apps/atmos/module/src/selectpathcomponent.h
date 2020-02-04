#pragma once

// External Includes
#include <component.h>
#include <componentptr.h>
#include <renderablemeshcomponent.h>
#include <parameternumeric.h>
#include <parametervec.h>
#include <polyline.h>
#include <nap/signalslot.h>
#include <transformcomponent.h>

// Local Includes
#include "followpathcontroller.h"

namespace nap
{
	class SelectPathComponentInstance;

	/**
	 * Resource Part: Allows for selecting a polyline for the camera to follow
	 */
	class NAPAPI SelectPathComponent : public Component
	{
		RTTI_ENABLE(Component)
		DECLARE_COMPONENT(SelectPathComponent, SelectPathComponentInstance)
	public:
		std::vector<nap::ResourcePtr<nap::PolyLine>>	mPaths;
		nap::ComponentPtr<nap::RenderableMeshComponent> mPathRenderer;
		nap::ComponentPtr<nap::FollowPathController>	mPathController;
		ResourcePtr<ParameterInt>						mIndex = 0;
		ResourcePtr<ParameterVec3>						mPathPosition;
		ResourcePtr<ParameterVec3>						mPathAxis;
		ResourcePtr<ParameterFloat>						mPathAngle;
		ResourcePtr<ParameterVec3>						mPathScale;
		ResourcePtr<ParameterFloat>						mPathUniformScale;

		bool init(utility::ErrorState& error) override;
	};


	/**
	 * Instance part: Allows for selecting a polyline for the camera to follow
	 */
	class NAPAPI SelectPathComponentInstance : public ComponentInstance
	{
		RTTI_ENABLE(ComponentInstance)
	public:
		SelectPathComponentInstance(EntityInstance& entity, Component& resource) :
			ComponentInstance(entity, resource)									{ }

		/**
		 * Initialize selectpathcomponentInstance based on the selectpathcomponent resource
		 * @param entityCreationParams when dynamically creating entities on initialization, add them to this this list.
		 * @param errorState should hold the error message when initialization fails
		 * @return if the selectpathcomponentInstance is initialized successfully
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * update selectpathcomponentInstance. This is called by NAP core automatically
		 * @param deltaTime time in between frames in seconds
		 */
		virtual void update(double deltaTime) override;

		/**
		 * Selects a new path
		 * @param index new path index, clamped to path range
		 */
		void selectPath(int index);

	private:
		ComponentInstancePtr<nap::RenderableMeshComponent> mRenderer = initComponentInstancePtr(this, &SelectPathComponent::mPathRenderer);
		ComponentInstancePtr<nap::FollowPathController> mController  = initComponentInstancePtr(this, &SelectPathComponent::mPathController);

		std::vector<RenderableMesh> mRenderPaths;
		std::vector<PolyLine*> mPaths;

		RenderableMesh* mCurrentPath = nullptr;
		TransformComponentInstance* mPathTransForm = nullptr;

		void updatePathAxis(glm::vec3 eulerAngles);
		void updatePathAngle(float angle);
		void updatePosition(glm::vec3 position);
		void updateScale(glm::vec3 scale);
		void updateUniformScale(float value);

		nap::Slot<int> mPathIndexChangedSlot			= { this, &SelectPathComponentInstance::selectPath };
		nap::Slot<glm::vec3> mPathAxisChangedSlot		= { this, &SelectPathComponentInstance::updatePathAxis };
		nap::Slot<float> mPathAngleChangedSlot			= { this, &SelectPathComponentInstance::updatePathAngle };
		nap::Slot<glm::vec3> mPathPositionChangedSlot	= { this, &SelectPathComponentInstance::updatePosition };
		nap::Slot<glm::vec3> mPathScaleChangedSlot		= { this, &SelectPathComponentInstance::updateScale };
		nap::Slot<float> mPathUniformScaleChangedSlot	= { this, &SelectPathComponentInstance::updateUniformScale };

		nap::ParameterVec3* mPathAxisParam = nullptr;
		nap::ParameterFloat* mPathAngleParam = nullptr;
	};
}
