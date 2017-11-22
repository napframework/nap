#include "updatenormalscomponent.h"

// External Includes
#include "entity.h"
#include <nap/logger.h>

// nap::updatenormalscomponent run time class definition 
RTTI_BEGIN_CLASS(nap::UpdateNormalsComponent)
	RTTI_PROPERTY("NormalMesh", &nap::UpdateNormalsComponent::mNormalMesh, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

// nap::updatenormalscomponentInstance run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::UpdateNormalsComponentInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	void UpdateNormalsComponent::getDependentComponents(std::vector<rtti::TypeInfo>& components) const
	{

	}


	bool UpdateNormalsComponentInstance::init(utility::ErrorState& errorState)
	{
		// Copy mesh
		mNormalMesh = getComponent<UpdateNormalsComponent>()->mNormalMesh.get();

		// Accept
		return true;
	}


	void UpdateNormalsComponentInstance::update(double deltaTime)
	{
		nap::utility::ErrorState error;
		if (!mNormalMesh->updateNormals(error, true))
		{
			nap::Logger::warn(error.toString());
		}
	}
}