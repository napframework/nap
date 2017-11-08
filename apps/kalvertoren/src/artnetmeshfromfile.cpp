#include "artnetmeshfromfile.h"
#include "fbxconverter.h"

// nap::artnetmeshfromfile run time class definition 
RTTI_BEGIN_CLASS(nap::ArtnetMeshFromFile)
	RTTI_PROPERTY("Path", &nap::ArtnetMeshFromFile::mPath, nap::rtti::EPropertyMetaData::FileLink | nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	bool ArtnetMeshFromFile::init(utility::ErrorState& errorState)
	{
		// Load our mesh
		std::unique_ptr<MeshInstance> mesh_instance = loadMesh(mPath, errorState);
		if (!errorState.check(mesh_instance != nullptr, "Unable to load mesh %s for resource %d", mPath.c_str(), mID.c_str()))
			return false;

		// Move
		mMeshInstance = std::move(mesh_instance);

		// Now check for the color attribute
		mColorAttribute = mMeshInstance->FindAttribute<glm::vec4>(MeshInstance::VertexAttributeIDs::GetColorName(0));
		if (!errorState.check(mColorAttribute != nullptr, "unable to find color attribute: %s on mesh: %s", MeshInstance::VertexAttributeIDs::GetColorName(0), mPath.c_str()))
			return false;

		// Extract the channels from the color where R = channel, G = universe and B = subnet
		mChannelAttribute = &mMeshInstance->GetOrCreateAttribute<int>("channel");
		mSubnetAttribute = &mMeshInstance->GetOrCreateAttribute<int>("subnet");
		mUniverseAttribute = &mMeshInstance->GetOrCreateAttribute<int>("universe");

		std::vector<int> channel_data(mColorAttribute->getCount(), 0);
		std::vector<int> subnet_data(mColorAttribute->getCount(), 0);
		std::vector<int> universe_data(mColorAttribute->getCount(), 0);

		int count = 0;
		for (const auto& color : mColorAttribute->getData())
		{
			channel_data[count] = static_cast<int>(color.r * 511.0f);
			universe_data[count] = static_cast<int>(color.g * 15.0f);
			subnet_data[count] = static_cast<int>(color.b * 15.0f);
			count++;
		}

		mChannelAttribute->setData(channel_data);
		mUniverseAttribute->setData(universe_data);
		mSubnetAttribute->setData(subnet_data);

		// Initialize the mesh
		if (!errorState.check(mMeshInstance->init(errorState), "Unable to initialize mesh %s for resource %d", mPath.c_str(), mID.c_str()))
			return false;

		return true;
	}
}