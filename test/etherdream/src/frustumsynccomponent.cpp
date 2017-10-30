#include "frustumsynccomponent.h"
#include <nap/core.h>
#include <nap/resourcemanager.h>

namespace nap
{
	//////////////////////////////////////////////////////////////////////////

	RTTI_BEGIN_CLASS(nap::FrustumSyncComponent)
		RTTI_PROPERTY("CanvasEntity", &nap::FrustumSyncComponent::mCanvasEntity, nap::rtti::EPropertyMetaData::Required)
		RTTI_PROPERTY("LaserOutputComponent", &nap::FrustumSyncComponent::mLaserOutputComponent, nap::rtti::EPropertyMetaData::Required)
	RTTI_END_CLASS

	RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::FrustumSyncComponentInstance)
		RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
	RTTI_END_CLASS

	//////////////////////////////////////////////////////////////////////////

	bool FrustumSyncComponentInstance::init(EntityCreationParameters& entityCreationParams, utility::ErrorState& errorState)
	{
		FrustumSyncComponent* resource = getComponent<FrustumSyncComponent>();

		ResourceManagerService& resource_manager = *getEntityInstance()->getCore()->getService<nap::ResourceManagerService>();
		
		// Create frustrum visualizer
		auto laser_draw_entity = resource_manager.createEntity(*(resource->mCanvasEntity), entityCreationParams, errorState);
		if (laser_draw_entity == nullptr)
			return false;
		getEntityInstance()->addChild(*laser_draw_entity);

		// Make sure that visualizer has a transform
		mCanvasTransform = laser_draw_entity->findComponent<TransformComponentInstance>(ETypeCheck::IS_DERIVED_FROM);
		if (!errorState.check(mCanvasTransform != nullptr, "missing transform component"))
			return false;

		// Move the frustrum back a bit so objects around 0 are sorted correctly
		mCanvasTransform->setTranslate(glm::vec3(0.0f, 0.0f, -0.1f));

		return true;
	}


	void FrustumSyncComponentInstance::update(double deltaTime)
	{
		glm::vec2 laser_frustrum = mOutput->mProperties.mFrustum;
		mCanvasTransform->setScale(glm::vec3(laser_frustrum.x, laser_frustrum.y, 1.0));
	}


	void FrustumSyncComponent::getDependentComponents(std::vector<rtti::TypeInfo>& components) const
	{
		components.emplace_back(RTTI_OF(nap::TransformComponent));
	}
}