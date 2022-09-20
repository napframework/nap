// Local Includes
#include "torusmesh.h"

// External Includes
#include <nap/core.h>
#include <meshutils.h>
#include <renderglobals.h>
#include <glm/gtc/constants.hpp>

// nap::TorusMesh run time class definition
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::TorusMesh)
	RTTI_CONSTRUCTOR(nap::Core&)
	RTTI_PROPERTY("Radius",			&nap::TorusMesh::mRadius,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("TubeRadius",		&nap::TorusMesh::mTubeRadius,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Segments",		&nap::TorusMesh::mSegments,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("TubeSegments",	&nap::TorusMesh::mTubeSegments,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("AngleOffset",	&nap::TorusMesh::mAngleOffset,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Usage",			&nap::TorusMesh::mUsage,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("PolygonMode",	&nap::TorusMesh::mPolygonMode,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("CullMode",		&nap::TorusMesh::mCullMode,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Color",			&nap::TorusMesh::mColor,			nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	//////////////////////////////////////////////////////////////////////////
	// Helpers
	//////////////////////////////////////////////////////////////////////////

	/**
	 * Creates a torus that consists out of the given number of segments. 
	 * The torus is considered closed, the last vertex is the one before the start.
	 * @param segmentCount the number of circle segments
	 * @param tubeSegmentCount the number of tubular segments
	 * @param radius the radius of the torus
	 * @param tubeRadius the radius of the tube segments
	 * @param angleOffset angle offset in degrees
	 * @param color vertex color
	 */
	static void createTorus(MeshInstance& meshInstance, uint segmentCount, uint tubeSegmentCount, float radius, float tubeRadius, float angleOffset, const RGBAColorFloat& color)
	{
		// Add an additional segment to generate seamless UVs around the torus
		uint vertex_count = segmentCount * tubeSegmentCount;
		meshInstance.setNumVertices(vertex_count);

		auto& pos_attr = meshInstance.getOrCreateAttribute<glm::vec3>(vertexid::position);
		pos_attr.resize(vertex_count);

		auto& normal_attr = meshInstance.getOrCreateAttribute<glm::vec3>(vertexid::normal);
		normal_attr.resize(vertex_count);

		auto& uv_attr = meshInstance.getOrCreateAttribute<glm::vec3>(vertexid::getUVName(0));
		uv_attr.resize(vertex_count);

		auto& color_attribute = meshInstance.getOrCreateAttribute<glm::vec4>(vertexid::getColorName(0));
		color_attribute.setData({ vertex_count, color.toVec4() });

		std::vector<glm::vec3>::iterator v = pos_attr.getData().begin();
		std::vector<glm::vec3>::iterator n = normal_attr.getData().begin();
		std::vector<glm::vec3>::iterator t = uv_attr.getData().begin();

		const float RX = 1.0f / static_cast<float>(segmentCount-1);
		const float RY = 1.0f / static_cast<float>(tubeSegmentCount-1);
		const float angle_offset_rad = math::radians(angleOffset);

		// Torus structure segment
		for (uint r = 0; r < segmentCount; r++)
		{
			const float pct0 = r * RX;
			const float theta = glm::two_pi<float>() * pct0;

			// Torus tube segment
			for (uint rt = 0; rt < tubeSegmentCount; rt++)
			{
				const float pct1 = rt * RY;
				const float phi = glm::two_pi<float>() * pct1 + angle_offset_rad;

				const glm::vec2 coord = { std::cos(theta), std::sin(theta) };
				const float x = coord.x * (radius + std::cos(phi) * tubeRadius);
				const float y = coord.y * (radius + std::cos(phi) * tubeRadius);
				const float z = std::sin(phi) * tubeRadius;

				// Set texture coordinates
				*t++ = { pct0, pct1, 0.0f };

				// Set vertex coordinates
				*v++ = { x, y, z };

				// Set normal coordinates
				const glm::vec3 norm = { coord.x * std::cos(phi), coord.y * std::cos(phi), std::sin(phi) };
				*n++ = glm::normalize(norm);
			}
		}

		// Torus indices
		uint i_segments = segmentCount - 1;
		uint i_tube_segments = tubeSegmentCount - 1;
		uint index_count = i_segments * i_tube_segments * 6;

		auto& indices = meshInstance.createShape().getIndices();
		indices.resize(index_count);
		std::vector<uint>::iterator i = indices.begin();

		for (uint s = 0; s < i_segments; s++)
		{
			const uint s_plus = (s + 1);

			for (uint ts = 0; ts < i_tube_segments; ts++)
			{
				const uint ts_plus = (ts + 1);

				// Triangle A
				*i++ = (s * tubeSegmentCount) + ts;
				*i++ = (s_plus * tubeSegmentCount) + ts_plus;
				*i++ = (s * tubeSegmentCount) + ts_plus;

				// Triangle B
				*i++ = (s * tubeSegmentCount) + ts;
				*i++ = (s_plus * tubeSegmentCount) + ts;
				*i++ = (s_plus * tubeSegmentCount) + ts_plus;
			}
		}
	}


	//////////////////////////////////////////////////////////////////////////
	// TorusMesh
	//////////////////////////////////////////////////////////////////////////

	TorusMesh::TorusMesh(nap::Core& core) : mRenderService(core.getService<nap::RenderService>()) {}


	bool TorusMesh::init(utility::ErrorState& errorState)
	{
		// Validate mesh
		if (!errorState.check(mSegments > 2 && mTubeSegments > 2, "The number of (tube) segments must be higher than 2"))
			return false;

		// Setup mesh
		if (!setup(errorState))
			return false;

		// Initialize the mesh instance
		return mMeshInstance->init(errorState);
	}


	bool TorusMesh::setup(utility::ErrorState& errorState)
	{
		// Create the mesh instance
		assert(mMeshInstance == nullptr);
		assert(mRenderService != nullptr);
		mMeshInstance = std::make_unique<MeshInstance>(*mRenderService);
		mMeshInstance->setDrawMode(EDrawMode::Triangles);
		mMeshInstance->setUsage(mUsage);
		mMeshInstance->setPolygonMode(mPolygonMode);
		mMeshInstance->setCullMode(mCullMode);

		// Create torus
		createTorus(*mMeshInstance, mSegments+1, mTubeSegments+1, mRadius, mTubeRadius, mAngleOffset, mColor);
		return true;
	}
}
