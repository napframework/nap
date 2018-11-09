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

		// Now check for the second color attribute
		// The Red color represents the group, from 0 to 9 as float value 0 to 1
		nap::VertexAttribute<glm::vec4>* index_colors = mMeshInstance->findAttribute<glm::vec4>(VertexAttributeIDs::GetColorName(1));
		if (errorState.check(index_colors == nullptr, "unable to find second color attribute, used to control idividual groups"))
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


		// Create index attribute and fill with bogus data
		mIndexAttribute = &(mMeshInstance->getOrCreateAttribute<int>("index"));
		std::vector<int> empty_index_data(index_colors->getCount(), 0);
		mIndexAttribute->setData(empty_index_data);
		std::vector<int>& index_data = mIndexAttribute->getData();

		// Map color red back to index and store in index attribute
		int index_count = 0;
		for (const auto& index_color : index_colors->getData())
		{
			index_data[index_count] = math::min<int>(static_cast<int>(index_color.r * 9.0f), 9);
			index_count++;
		}

		// Verify that our triangles all have the same channels as vertex attributes
		// Also store all the available artnet addresses this mesh hosts
		TriangleIterator triangle_iterator(*mMeshInstance);
		while (!triangle_iterator.isDone())
		{
			Triangle triangle = triangle_iterator.next();
			if (channel_data[triangle.firstIndex()] != channel_data[triangle.secondIndex()] || channel_data[triangle.secondIndex()] != channel_data[triangle.thirdIndex()])
			{
				errorState.fail("mesh: %s triangle: %d has inconsistent art net channel attribute", mPath.c_str(), triangle.getTriangleIndex());
				return false;
			}

			if (universe_data[triangle.firstIndex()] != universe_data[triangle.secondIndex()] || universe_data[triangle.secondIndex()] != universe_data[triangle.thirdIndex()])
			{
				errorState.fail("mesh: %s triangle: %d has inconsistent art net universe attribute", mPath.c_str(), triangle.getTriangleIndex());
				return false;
			}

			if (subnet_data[triangle.firstIndex()] != subnet_data[triangle.secondIndex()] || subnet_data[triangle.secondIndex()] != subnet_data[triangle.thirdIndex()])
			{
				errorState.fail("mesh: %s triangle: %d has inconsistent art net subnet attribute", mPath.c_str(), triangle.getTriangleIndex());
				return false;
			}

			if (index_data[triangle.firstIndex()] != index_data[triangle.secondIndex()] || index_data[triangle.secondIndex()] != index_data[triangle.thirdIndex()])
			{
				errorState.fail("mesh: %s triangle: %d has inconsistent index attribute", mPath.c_str(), triangle.getTriangleIndex());
				return false;
			}

			mAddresses.emplace(ArtNetController::createAddress(subnet_data[triangle.firstIndex()], universe_data[triangle.firstIndex()]));
		}

		// Get mesh bounding box
		utility::computeBoundingBox(*mMeshInstance, mBoundingBox);
		
		// Initialize the mesh
		if (!errorState.check(mMeshInstance->init(errorState), "Unable to initialize mesh %s for resource %d", mPath.c_str(), mID.c_str()))
			return false;

		return true;
	}
}