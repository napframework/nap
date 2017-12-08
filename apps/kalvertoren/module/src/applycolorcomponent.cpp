#include "applycolorcomponent.h"

// External Includes
#include <entity.h>

// nap::colormeshcomponent run time class definition 
RTTI_BEGIN_CLASS(nap::ApplyColorComponent)
	RTTI_PROPERTY("Mesh", &nap::ApplyColorComponent::mMesh, nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Selector", &nap::ApplyColorComponent::mSelector, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

// nap::colormeshcomponentInstance run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::ApplyColorComponentInstance)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	bool ApplyColorComponentInstance::init(utility::ErrorState& errorState)
	{
		// Get the mesh we want to apply colors to
		mMesh = getComponent<ApplyColorComponent>()->mMesh.get();
		
		// Register this one as a valid selection option
		mSelector->registerMeshColorComponent(*this);

		return true;
	}


	void ApplyColorComponentInstance::apply(double deltaTime)
	{
		if (!mActive)
			return;
		applyColor(deltaTime);
	}
}