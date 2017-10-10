#include "frustrumsynccomponent.h"
#include <nap/core.h>
#include <nap/resourcemanager.h>

namespace nap
{
	//////////////////////////////////////////////////////////////////////////

	RTTI_BEGIN_CLASS(nap::FrustrumSyncComponent)
		RTTI_PROPERTY("CanvasEntity", &nap::FrustrumSyncComponent::mCanvasEntity, nap::rtti::EPropertyMetaData::Required)
		RTTI_PROPERTY("LaserOutputComponent", &nap::FrustrumSyncComponent::mLaserOutputComponent, nap::rtti::EPropertyMetaData::Required)
	RTTI_END_CLASS

	RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::FrustrumSyncComponentInstance)
		RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
	RTTI_END_CLASS

	//////////////////////////////////////////////////////////////////////////

	bool FrustrumSyncComponentInstance::init(EntityCreationParameters& entityCreationParams, utility::ErrorState& errorState)
	{
		FrustrumSyncComponent* resource = getComponent<FrustrumSyncComponent>();

		ResourceManagerService& resource_manager = *getEntityInstance()->getCore()->getService<nap::ResourceManagerService>();
		
		// Create frustrum visualizer
		auto laser_draw_entity = resource_manager.createEntity(*(resource->mCanvasEntity), entityCreationParams, errorState);
		if (laser_draw_entity == nullptr)
			return false;

		// Make sure that visualizer has a transform
		mCanvasTransform = laser_draw_entity->findComponent<TransformComponentInstance>(ETypeCheck::IS_DERIVED_FROM);
		if (!errorState.check(mCanvasTransform != nullptr, "missing transform component"))
			return false;

		// Get the output
		mOutput = resource->mLaserOutputComponent.get();

		// Move the frustrum back a bit so objects around 0 are sorted correctly
		mCanvasTransform->setTranslate(glm::vec3(0.0f, 0.0f, -0.1f));

		return true;
	}


	void FrustrumSyncComponentInstance::update(double deltaTime)
	{
		glm::vec2 laser_frustrum = mOutput->mProperties.mFrustrum;
		mCanvasTransform->setScale(glm::vec3(laser_frustrum.x, laser_frustrum.y, 1.0));
	}


	void FrustrumSyncComponent::getDependentComponents(std::vector<rtti::TypeInfo>& components) const
	{
		components.emplace_back(RTTI_OF(nap::TransformComponent));
	}
}