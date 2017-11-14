#include "artnetmeshfromfile.h"
#include "fbxconverter.h"
#include "meshutils.h"
#include <mathutils.h>

// nap::artnetmeshfromfile run time class definition 
RTTI_BEGIN_CLASS(nap::ArtnetMeshFromFile)
	RTTI_PROPERTY("Path", &nap::ArtnetMeshFromFile::mPath, nap::rtti::EPropertyMetaData::FileLink | nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("OverrideSubnet", &nap::ArtnetMeshFromFile::mOverrideSubnet, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Subnet", &nap::ArtnetMeshFromFile::mSubnetAddress, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("ChannelOffset", &nap::ArtnetMeshFromFile::mChannelOffset, nap::rtti::EPropertyMetaData::Default)
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
		if (!errorState.check(mColorAttribute != nullptr, "unable to find color attribute: %s on mesh: %s", MeshInstance::VertexAttributeIDs::GetColorName(0).c_str(), mPath.c_str()))
			return false;

		// Get position
		mPositionAttribute = mMeshInstance->FindAttribute<glm::vec3>(MeshInstance::VertexAttributeIDs::GetPositionName());
		assert(mPositionAttribute != nullptr);

		// Get uv
		mUVAttribute = mMeshInstance->FindAttribute<glm::vec3>(MeshInstance::VertexAttributeIDs::GetUVName(0));
		if (!errorState.check(mUVAttribute != nullptr, "unable to find uv attribute: %s on mesh: %s", MeshInstance::VertexAttributeIDs::GetUVName(0).c_str(), mPath.c_str()))
			return false;

		// Extract the channels from the color where R = channel, G = universe and B = subnet
		mChannelAttribute = &mMeshInstance->GetOrCreateAttribute<int>("channel");
		mSubnetAttribute = &mMeshInstance->GetOrCreateAttribute<int>("subnet");
		mUniverseAttribute = &mMeshInstance->GetOrCreateAttribute<int>("universe");

		std::vector<int> channel_data(mColorAttribute->getCount(), 0);
		std::vector<int> subnet_data(mColorAttribute->getCount(), 0);
		std::vector<int> universe_data(mColorAttribute->getCount(), 0);

		// Set extracted art net attributes from color
		int count = 0;
		for (const auto& color : mColorAttribute->getData())
		{
			channel_data[count]  = math::min<int>(static_cast<int>(color.r * 511.0f) + mChannelOffset, 511);
			universe_data[count] = static_cast<int>(color.g * 15.0f);
			subnet_data[count]   = mOverrideSubnet ? mSubnetAddress : static_cast<int>(color.b * 15.0f);
			count++;
		}
		mChannelAttribute->setData(channel_data);
		mUniverseAttribute->setData(universe_data);
		mSubnetAttribute->setData(subnet_data);

		// Verify that our triangles all have the same channels as vertex attributes
		// Also store all the available artnet addresses this mesh hosts
		int tri_count = getTriangleCount(*mMeshInstance);
		TriangleData<int> tri_channels = { 0,0,0 };
		TriangleData<int> tri_subnets = { 0,0,0 };
		TriangleData<int> tri_universes = { 0,0,0 };
		for (int i = 0; i < tri_count; i++)
		{
			getTriangleValues<int>(*mMeshInstance, i, *mChannelAttribute, tri_channels);
			if (tri_channels[0] != tri_channels[1] || tri_channels[1] != tri_channels[2])
			{
				return errorState.check(false, "mesh: %s triangle: %d has inconsistent art net channel attribute", mPath.c_str(), i);
			}

			getTriangleValues<int>(*mMeshInstance, i, *mUniverseAttribute, tri_universes);
			if (tri_universes[0] != tri_universes[1] || tri_universes[1] != tri_universes[2])
			{
				return errorState.check(false, "mesh: %s triangle: %d has inconsistent art net universe attribute", mPath.c_str(), i);
			}

			getTriangleValues<int>(*mMeshInstance, i, *mSubnetAttribute, tri_subnets);
			if (tri_subnets[0] != tri_subnets[1] || tri_subnets[1] != tri_subnets[2])
			{
				return errorState.check(false, "mesh: %s triangle: %d has inconsistent art net subnet attribute", mPath.c_str(), i);
			}
			mAddresses.emplace(ArtNetController::createAddress(tri_subnets[0], tri_universes[0]));
		}

		// Get mesh bounding box
		nap::getBoundingBox(*mMeshInstance, mBoundingBox);
		
		// Initialize the mesh
		if (!errorState.check(mMeshInstance->init(errorState), "Unable to initialize mesh %s for resource %d", mPath.c_str(), mID.c_str()))
			return false;

		return true;
	}
}