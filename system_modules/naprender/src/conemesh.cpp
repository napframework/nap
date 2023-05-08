// Local Includes
#include "conemesh.h"

// External Includes
#include <nap/core.h>
#include <meshutils.h>
#include <renderglobals.h>
#include <glm/gtc/constants.hpp>

// nap::ConeMesh run time class definition
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::ConeMesh)
	RTTI_CONSTRUCTOR(nap::Core&)
	RTTI_PROPERTY("Height",			&nap::ConeMesh::mHeight,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("BottomRadius",	&nap::ConeMesh::mBottomRadius,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("TopRadius",		&nap::ConeMesh::mTopRadius,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Segments",		&nap::ConeMesh::mSegments,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("AngleOffset",	&nap::ConeMesh::mAngleOffset,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Usage",			&nap::ConeMesh::mUsage,				nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("PolygonMode",	&nap::ConeMesh::mPolygonMode,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("CullMode",		&nap::ConeMesh::mCullMode,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Color",			&nap::ConeMesh::mColor,				nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("FlipNormals",	&nap::ConeMesh::mFlipNormals,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Vec4Mode",		&nap::ConeMesh::mVec4Mode,			nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	//////////////////////////////////////////////////////////////////////////
	// Helpers
	//////////////////////////////////////////////////////////////////////////

	/**
	 * Creates a cone that consists out of the given number of segments. 
	 * The cone is considered closed, the last vertex is the one before the start.
	 * @param outMeshInstance the mesh instance to create the vertex attribute buffers for
	 * @param height the height of the cone
	 * @param segmentCount the number of horizontal segments, i.e. cone resolution
	 * @param topRadius the radius of the cone tip
	 * @param bottomRadius  the radius of the cone base
	 * @param angleOffset angle offset in degrees
	 * @param color vertex color
	 * @param flipNormals whether to flip the cone normals
	 */
	static void createCone(MeshInstance& outMeshInstance, uint segmentCount, float height, float bottomRadius, float topRadius, float angleOffset, const RGBAColorFloat& color, bool flipNormals)
	{
		assert(segmentCount > 0);

		// Add an additional segment to generate seamless UVs around the cone
		uint vertex_count = segmentCount * 4;
		outMeshInstance.setNumVertices(vertex_count);

		auto& pos_attr = outMeshInstance.getOrCreateAttribute<glm::vec3>(vertexid::position);
		pos_attr.resize(vertex_count);

		auto& normal_attr = outMeshInstance.getOrCreateAttribute<glm::vec3>(vertexid::normal);
		normal_attr.resize(vertex_count);

		auto& uv_attr = outMeshInstance.getOrCreateAttribute<glm::vec3>(vertexid::getUVName(0));
		uv_attr.resize(vertex_count);

		auto& color_attribute = outMeshInstance.getOrCreateAttribute<glm::vec4>(vertexid::getColorName(0));
		color_attribute.setData({ vertex_count, color.toVec4() });

		std::vector<glm::vec3>::iterator v = pos_attr.getData().begin();
		std::vector<glm::vec3>::iterator n = normal_attr.getData().begin();
		std::vector<glm::vec3>::iterator t = uv_attr.getData().begin();

		const float R = 1.0f / static_cast<float>(segmentCount);
		const float angle_offset_rad = math::radians(angleOffset);

		std::vector<glm::vec2> base_coords;
		base_coords.reserve(segmentCount);

		std::vector<glm::vec2> face_center_coords;
		face_center_coords.reserve(segmentCount);

		for (uint r = 0; r < segmentCount; r++)
		{
			float pct = (r%segmentCount) * R;
			float phi = glm::two_pi<float>() * pct + angle_offset_rad;
			glm::vec2 coord = { std::cos(phi), std::sin(phi) };
			base_coords.emplace_back(coord);

			pct = (static_cast<float>(r % segmentCount) + 0.5f) * R;
			phi = glm::two_pi<float>() * pct + angle_offset_rad;
			coord = { std::cos(phi), std::sin(phi) };
			face_center_coords.emplace_back(coord);
		}

		// Cone structure segment
		const float slope = ((bottomRadius - topRadius) * bottomRadius) / height;
		const float flip = flipNormals ? -1.0f : 1.0f;
		const uint verts_per_segment = 4;

		for (uint r = 0; r < segmentCount; r++)
		{
			const glm::vec2& c0 = base_coords[r];
			const glm::vec2& c1 = base_coords[(r+1)%segmentCount];

			// Set vertex coordinates
			*v++ = { c0.x * bottomRadius,	0.0f,	c0.y * bottomRadius };
			*v++ = { c0.x * topRadius,		height, c0.y * topRadius };
			*v++ = { c1.x * bottomRadius,	0.0f,	c1.y * bottomRadius };
			*v++ = { c1.x * topRadius,		height, c1.y * topRadius };

			// Set normal coordinates
			const glm::vec2& cn = face_center_coords[r];
			const glm::vec3 norm = glm::normalize(glm::vec3(cn.x, slope, cn.y)) * flip;
			for (uint i = 0; i < verts_per_segment; i++)
				*n++ = norm;

			// Set texture coordinates
			*t++ = { c0.x, 0.0f, c0.y };
			*t++ = { c0.x, 1.0f, c0.y };
			*t++ = { c1.x, 0.0f, c1.y };
			*t++ = { c1.x, 1.0f, c1.y };
		}

		// Cone indices
		const uint indices_per_segment = 6;
		const uint index_count = segmentCount * indices_per_segment;

		auto& indices = outMeshInstance.createShape().getIndices();
		indices.resize(index_count);
		std::vector<uint>::iterator i = indices.begin();

		for (uint s = 0; s < segmentCount; s++)
		{
			uint base_index = s * verts_per_segment;

			// Triangle A
			*i++ = base_index + 0;
			*i++ = base_index + 1;
			*i++ = base_index + 2;

			// Triangle B
			*i++ = base_index + 3;
			*i++ = base_index + 2;
			*i++ = base_index + 1;
		}
	}


	static void createConeVec4(MeshInstance& outMeshInstance, uint segmentCount, float height, float bottomRadius, float topRadius, float angleOffset, const RGBAColorFloat& color, bool flipNormals)
	{
		assert(segmentCount > 0);

		// Add an additional segment to generate seamless UVs around the cone
		uint vertex_count = segmentCount * 4;
		outMeshInstance.setNumVertices(vertex_count);

		auto& pos_attr = outMeshInstance.getOrCreateAttribute<glm::vec4>(vertexid::position);
		pos_attr.resize(vertex_count);

		auto& normal_attr = outMeshInstance.getOrCreateAttribute<glm::vec4>(vertexid::normal);
		normal_attr.resize(vertex_count);

		auto& uv_attr = outMeshInstance.getOrCreateAttribute<glm::vec4>(vertexid::getUVName(0));
		uv_attr.resize(vertex_count);

		auto& color_attribute = outMeshInstance.getOrCreateAttribute<glm::vec4>(vertexid::getColorName(0));
		color_attribute.setData({ vertex_count, color.toVec4() });

		std::vector<glm::vec4>::iterator v = pos_attr.getData().begin();
		std::vector<glm::vec4>::iterator n = normal_attr.getData().begin();
		std::vector<glm::vec4>::iterator t = uv_attr.getData().begin();

		const float R = 1.0f / static_cast<float>(segmentCount);
		const float angle_offset_rad = math::radians(angleOffset);

		std::vector<glm::vec2> base_coords;
		base_coords.reserve(segmentCount);

		std::vector<glm::vec2> face_center_coords;
		face_center_coords.reserve(segmentCount);

		for (uint r = 0; r < segmentCount; r++)
		{
			float pct = (r % segmentCount) * R;
			float phi = glm::two_pi<float>() * pct + angle_offset_rad;
			glm::vec2 coord = { std::cos(phi), std::sin(phi) };
			base_coords.emplace_back(coord);

			pct = (static_cast<float>(r % segmentCount) + 0.5f) * R;
			phi = glm::two_pi<float>() * pct + angle_offset_rad;
			coord = { std::cos(phi), std::sin(phi) };
			face_center_coords.emplace_back(coord);
		}

		// Cone structure segment
		const float slope = ((bottomRadius - topRadius) * bottomRadius) / height;
		const float flip = flipNormals ? -1.0f : 1.0f;
		const uint verts_per_segment = 4;

		for (uint r = 0; r < segmentCount; r++)
		{
			const glm::vec2& c0 = base_coords[r];
			const glm::vec2& c1 = base_coords[(r + 1) % segmentCount];

			// Set vertex coordinates
			*v++ = { c0.x * bottomRadius,	0.0f,	c0.y * bottomRadius,	0.0f };
			*v++ = { c0.x * topRadius,		height, c0.y * topRadius,		0.0f };
			*v++ = { c1.x * bottomRadius,	0.0f,	c1.y * bottomRadius,	0.0f };
			*v++ = { c1.x * topRadius,		height, c1.y * topRadius,	0.0f };

			// Set normal coordinates
			const glm::vec2& cn = face_center_coords[r];
			const glm::vec4 norm = { glm::normalize(glm::vec3(cn.x, slope, cn.y)) * flip, 0.0f };
			for (uint i = 0; i < verts_per_segment; i++)
				*n++ = norm;

			// Set texture coordinates
			*t++ = { c0.x, 0.0f, c0.y, 0.0f };
			*t++ = { c0.x, 1.0f, c0.y, 0.0f };
			*t++ = { c1.x, 0.0f, c1.y, 0.0f };
			*t++ = { c1.x, 1.0f, c1.y, 0.0f };
		}

		// Cone indices
		const uint indices_per_segment = 6;
		const uint index_count = segmentCount * indices_per_segment;

		auto& indices = outMeshInstance.createShape().getIndices();
		indices.resize(index_count);
		std::vector<uint>::iterator i = indices.begin();

		for (uint s = 0; s < segmentCount; s++)
		{
			uint base_index = s * verts_per_segment;

			// Triangle A
			*i++ = base_index + 0;
			*i++ = base_index + 1;
			*i++ = base_index + 2;

			// Triangle B
			*i++ = base_index + 3;
			*i++ = base_index + 2;
			*i++ = base_index + 1;
		}
	}


	//////////////////////////////////////////////////////////////////////////
	// ConeMesh
	//////////////////////////////////////////////////////////////////////////

	ConeMesh::ConeMesh(nap::Core& core) : mRenderService(core.getService<nap::RenderService>()) {}


	bool ConeMesh::init(utility::ErrorState& errorState)
	{
		// Validate mesh
		if (!errorState.check(mSegments > 2, "The number of (cone) segments must be higher than 2"))
			return false;

		if (!errorState.check(mHeight > 0.0f, "The height must be higher than zero"))
			return false;

		// Setup mesh
		if (!setup(errorState))
			return false;

		// Initialize the mesh instance
		return mMeshInstance->init(errorState);
	}


	bool ConeMesh::setup(utility::ErrorState& errorState)
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
		if (mVec4Mode)
			createConeVec4(*mMeshInstance, mSegments, mHeight, mBottomRadius, mTopRadius, mAngleOffset, mColor, mFlipNormals);
		else
			createCone(*mMeshInstance, mSegments, mHeight, mBottomRadius, mTopRadius, mAngleOffset, mColor, mFlipNormals);

		return true;
	}
}
