#include "applyvideocomponent.h"

// External Includes
#include <entity.h>
#include <meshutils.h>

// nap::applycompositioncomponent run time class definition 
RTTI_BEGIN_CLASS(nap::ApplyVideoComponent)
	RTTI_PROPERTY("VideoController", &nap::ApplyVideoComponent::mVideoController, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

// nap::applycompositioncomponentInstance run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::ApplyVideoComponentInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	void ApplyVideoComponent::getDependentComponents(std::vector<rtti::TypeInfo>& components) const
	{

	}


	bool ApplyVideoComponentInstance::init(utility::ErrorState& errorState)
	{
		if (!ApplyColorComponentInstance::init(errorState))
			return false;

		return true;
	}


	void ApplyVideoComponentInstance::applyColor(double deltaTime)
	{
		// Get the model we want to color
		nap::ArtnetMeshFromFile& mesh = getMesh();

		// Get the instance
		nap::MeshInstance& mesh_instance = mesh.getMeshInstance();

		nap::utility::ErrorState error;
		if (!mesh_instance.update(error))
		{
			assert(false);
		}
	}
}