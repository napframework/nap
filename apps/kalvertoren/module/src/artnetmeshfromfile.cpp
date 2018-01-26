#include "artnetmeshfromfile.h"
#include "fbxconverter.h"
#include "meshutils.h"
#include <mathutils.h>
#include <triangleiterator.h>

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
		mMeshColorAttribute = mMeshInstance->findAttribute<glm::vec4>(VertexAttributeIDs::GetColorName(0));
		if (!errorState.check(mMeshColorAttribute != nullptr, "unable to find color attribute: %s on mesh: %s", VertexAttributeIDs::GetColorName(0).c_str(), mPath.c_str()))
			return false;

		// Get position
		mPositionAttribute = mMeshInstance->findAttribute<glm::vec3>(VertexAttributeIDs::getPositionName());
		assert(mPositionAttribute != nullptr);

		// Get uv
		mUVAttribute = mMeshInstance->findAttribute<glm::vec3>(VertexAttributeIDs::getUVName(0));
		if (!errorState.check(mUVAttribute != nullptr, "unable to find uv attribute: %s on mesh: %s", VertexAttributeIDs::getUVName(0).c_str(), mPath.c_str()))
			return false;

		// Create the color attributes
		mColorAttribute = &mMeshInstance->getOrCreateAttribute<glm::vec4>("Color");
		std::vector<glm::vec4> empty_color_data(mMeshColorAttribute->getCount(), {0.0,0.0,0.0,1.0});
		mColorAttribute->setData(empty_color_data);

		mArtnetColorAttribute = &mMeshInstance->getOrCreateAttribute<glm::vec4>("ArtnetColor");
		std::vector<glm::vec4> empty_artnet_data(mMeshInstance->getNumVertices(), { 0.0,0.0,0.0,0.0 });
		mArtnetColorAttribute->setData(empty_artnet_data);

		// Extract the channels from the color where R = channel, G = universe and B = subnet
		mChannelAttribute = &mMeshInstance->getOrCreateAttribute<int>("channel");
		mSubnetAttribute = &mMeshInstance->getOrCreateAttribute<int>("subnet");
		mUniverseAttribute = &mMeshInstance->getOrCreateAttribute<int>("universe");

		std::vector<int> channel_data(mMeshColorAttribute->getCount(), 0);
		std::vector<int> subnet_data(mMeshColorAttribute->getCount(), 0);
		std::vector<int> universe_data(mMeshColorAttribute->getCount(), 0);

		// Set extracted art net attributes from color
		int count = 0;
		for (const auto& color : mMeshColorAttribute->getData())
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
		TriangleShapeIterator shape_iterator(*mMeshInstance);
		while (!shape_iterator.isDone())
		{
			glm::ivec3 indices = shape_iterator.next();
			if (channel_data[indices[0]] != channel_data[indices[1]] || channel_data[indices[1]] != channel_data[indices[2]])
			{
				errorState.fail("mesh: %s triangle: %d has inconsistent art net channel attribute", mPath.c_str(), shape_iterator.getCurrentTriangleIndex());
				return false;
			}

			if (universe_data[indices[0]] != universe_data[indices[1]] || universe_data[indices[1]] != universe_data[indices[2]])
			{
				errorState.fail("mesh: %s triangle: %d has inconsistent art net universe attribute", mPath.c_str(), shape_iterator.getCurrentTriangleIndex());
				return false;
			}

			if (subnet_data[indices[0]] != subnet_data[indices[1]] || subnet_data[indices[1]] != subnet_data[indices[2]])
			{
				errorState.fail("mesh: %s triangle: %d has inconsistent art net subnet attribute", mPath.c_str(), shape_iterator.getCurrentTriangleIndex());
				return false;
			}

			mAddresses.emplace(ArtNetController::createAddress(subnet_data[indices[0]], universe_data[indices[0]]));
		}

		// Get mesh bounding box
		utility::computeBoundingBox(*mMeshInstance, mBoundingBox);
		
		// Initialize the mesh
		if (!errorState.check(mMeshInstance->init(errorState), "Unable to initialize mesh %s for resource %d", mPath.c_str(), mID.c_str()))
			return false;

		return true;
	}
}