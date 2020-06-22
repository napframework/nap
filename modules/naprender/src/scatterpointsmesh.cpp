#include "scatterpointsmesh.h"
#include "meshutils.h"
#include "renderservice.h"

// External Includes
#include <mathutils.h>
#include <nap/core.h>

// nap::scatterpointsmesh run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::ScatterPointsMesh)
	RTTI_CONSTRUCTOR(nap::Core&)
	RTTI_PROPERTY("Usage",			&nap::ScatterPointsMesh::mUsage,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("ReferenceMesh",	&nap::ScatterPointsMesh::mReferenceMesh,	nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("PointCount",		&nap::ScatterPointsMesh::mNumberOfPoints,	nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{

	ScatterPointsMesh::ScatterPointsMesh(Core& core) : 
		mRenderService(core.getService<RenderService>())
	{ }

	bool ScatterPointsMesh::init(utility::ErrorState& errorState)
	{
		// make sure we have at least 1 point to scatter
		if (!errorState.check(mNumberOfPoints >= 0, 
			"invalid number of points: %s, need at least 1 point for scatter operation", this->mID.c_str()))
			return false;

		// Make sure the reference mesh contains triangles
		bool contains_triangles = utility::isTriangleMesh(mReferenceMesh->getMeshInstance());
		if (!errorState.check(contains_triangles, "reference mesh doesn't contain any triangles: %s", this->mID.c_str()))
			return false;

		// Create mesh instance
		if (!createMeshInstance(errorState))
			return false;

		// Setup instance (allocate vertex attributes)
		if (!setup(errorState))
			return false;

		// Scatter points
		if (!scatterPoints(errorState))
			return false;

		return true;
	}


	bool ScatterPointsMesh::createMeshInstance(nap::utility::ErrorState& error)
	{
		if (!error.check(mMeshInstance == nullptr, "unable to create new mesh, already assigned: %s", this->mID.c_str()))
			return false;

		// There is only 1 shape associated with the scatter mesh
		assert(mRenderService != nullptr);
		mMeshInstance = std::make_unique<MeshInstance>(*mRenderService);
		mMeshInstance->createShape();
		mMeshInstance->setUsage(mUsage);
		mMeshInstance->setDrawMode(EDrawMode::Points);
		return true;
	}


	bool ScatterPointsMesh::setup(nap::utility::ErrorState& error)
	{
		assert(mMeshInstance != nullptr);
		mMeshInstance->setNumVertices(mNumberOfPoints);
		mMeshInstance->setDrawMode(EDrawMode::Points);

		// Create position attribute
		mPositionAttr = &(mMeshInstance->getOrCreateAttribute<glm::vec3>(VertexAttributeIDs::getPositionName()));

		// Create normal attribute
		mNormalsAttr = &(mMeshInstance->getOrCreateAttribute<glm::vec3>(VertexAttributeIDs::getNormalName()));

		// Sample all uv sets
		mUvAttrs.clear();
		int uv_idx = 0;
		while (true)
		{
			const Vec3VertexAttribute* ref_uv_attr = mReferenceMesh->getMeshInstance().findAttribute<glm::vec3>(VertexAttributeIDs::getUVName(uv_idx));
			if (ref_uv_attr != nullptr)
			{
				mUvAttrs.emplace_back(&(mMeshInstance->getOrCreateAttribute<glm::vec3>(VertexAttributeIDs::getUVName(uv_idx))));
				uv_idx++;
				continue;
			}
			break;
		}

		// Sample all color sets
		mColorAttrs.clear();
		int clr_idx = 0;
		while (true)
		{
			const Vec4VertexAttribute* ref_clr_attr = mReferenceMesh->getMeshInstance().findAttribute<glm::vec4>(VertexAttributeIDs::GetColorName(clr_idx));
			if (ref_clr_attr != nullptr)
			{
				mColorAttrs.emplace_back(&(mMeshInstance->getOrCreateAttribute<glm::vec4>(VertexAttributeIDs::GetColorName(clr_idx))));
				clr_idx++;
				continue;
			}
			break;
		}

		// Create initial position data
		std::vector<glm::vec3> vertices(mNumberOfPoints, { 0.0f, 0.0f, 0.0f });
		mPositionAttr->setData(vertices);

		// Create initial normal data
		std::vector<glm::vec3> normals(mNumberOfPoints, { 0.0f,1.0f,0.0f });
		mNormalsAttr->setData(normals);

		// Create initial uv data
		std::vector<glm::vec3> uvs(mNumberOfPoints, { 0.0f, 0.0f, 0.0f });
		for (auto& uv_attr : mUvAttrs)
			uv_attr->setData(uvs);

		// Create initial color data
		std::vector<glm::vec4> colors(mNumberOfPoints, { 1.0f, 1.0f, 1.0f, 1.0f });
		for (auto& color_attr : mColorAttrs)
			color_attr->setData(colors);

		// Set number of vertices
		mMeshInstance->setNumVertices(mNumberOfPoints);

		// Automatically generate indices
		MeshShape& shape = mMeshInstance->getShape(0);
		utility::generateIndices(shape, mNumberOfPoints, false);

		return true;
	}


	bool ScatterPointsMesh::scatterPoints(nap::utility::ErrorState& error)
	{
		assert(mMeshInstance != nullptr);
		assert(mMeshInstance->getNumVertices() == mNumberOfPoints);

		// Get reference vertex positions
		const VertexAttribute<glm::vec3>* ref_pos = mReferenceMesh->getMeshInstance().findAttribute<glm::vec3>(VertexAttributeIDs::getPositionName());
		assert(ref_pos != nullptr);

		const VertexAttribute<glm::vec3>* ref_nor = mReferenceMesh->getMeshInstance().findAttribute<glm::vec3>(VertexAttributeIDs::getNormalName());

		// Get reference uvs
		std::vector<const Vec3VertexAttribute*> ref_uvs;
		ref_uvs.reserve(mUvAttrs.size());

		// Get reference clrs
		std::vector<const Vec4VertexAttribute*> ref_clrs;
		ref_clrs.reserve(mColorAttrs.size());

		// Query uv data
		for (int i = 0; i < mUvAttrs.size(); i++)
		{
			const Vec3VertexAttribute* ref_uv_attr = mReferenceMesh->getMeshInstance().findAttribute<glm::vec3>(VertexAttributeIDs::getUVName(i));
			if (!error.check(ref_uv_attr != nullptr, "unable to find uv attribute on reference mesh: %s with index: %d", mReferenceMesh->mID.c_str(), i))
				return false;
			ref_uvs.emplace_back(ref_uv_attr);
		}

		// Query color data
		for (int i = 0; i < mColorAttrs.size(); i++)
		{
			const Vec4VertexAttribute* ref_clr_attr = mReferenceMesh->getMeshInstance().findAttribute<glm::vec4>(VertexAttributeIDs::GetColorName(i));
			if (!error.check(ref_clr_attr != nullptr, "unable to find uv attribute on reference mesh: %s with index: %d", mReferenceMesh->mID.c_str(), i))
				return false;
			ref_clrs.emplace_back(ref_clr_attr);
		}

		// Compute total area of mesh
		std::vector<float> triangle_areas;
		TriangleAreaMap area_map;
		float total_area = computeArea(area_map);

		// Scatter points randomly
		for (int i = 0; i < mNumberOfPoints; i++)
		{
			// Generate random number for point
			// Used for placement
			float rnumber = math::random<float>(0.0f, total_area);

			auto it = area_map.lower_bound(rnumber);
			assert(it != area_map.end());
			Triangle& triangle = it->second;

			// Extract vertex positions for triangle
			nap::TriangleData<glm::vec3> tri_pos = triangle.getVertexData<glm::vec3>(*ref_pos);

			// Extract colors for triangle
			glm::vec2 rand_v = math::random<glm::vec2>({ 0.0f, 0.0f }, {1.0f, 1.0f});
			if (rand_v.x + rand_v.y >= 1)
			{
				rand_v.x = 1.0f - rand_v.x;
				rand_v.y = 1.0f - rand_v.y;
			}
			glm::vec3 bary_coords = { rand_v, 0.0f };

			// New point based on random bary coordinates
			glm::vec3 point_pos = utility::interpolateVertexAttr(tri_pos, bary_coords);
			mPositionAttr->getData()[i] = point_pos;

			// New Normal based on random bary coordinates
			if (ref_nor != nullptr)
			{
				glm::vec3 normal_val = utility::interpolateVertexAttr(triangle.getVertexData<glm::vec3>(*ref_nor), bary_coords);
				mNormalsAttr->getData()[i] = normal_val;
			}

			// Copy over the uv coordinate for every uv set in the reference mesh
			for (auto uv_attr = 0; uv_attr < mUvAttrs.size(); uv_attr++)
			{
				mUvAttrs[uv_attr]->getData()[i] = 
					utility::interpolateVertexAttr(triangle.getVertexData<glm::vec3>(*(ref_uvs[uv_attr])), bary_coords);
			}

			// Copy over the uv coordinate for every uv set in the reference mesh
			for (auto clr_attr = 0; clr_attr < mColorAttrs.size(); clr_attr++)
			{
				mColorAttrs[clr_attr]->getData()[i] = 
					utility::interpolateVertexAttr(triangle.getVertexData<glm::vec4>(*(ref_clrs[clr_attr])), bary_coords);
			}
		}

		return mMeshInstance->init(error);
	}


	float ScatterPointsMesh::computeArea(TriangleAreaMap& areaMap)
	{
		MeshInstance& mesh = mReferenceMesh->getMeshInstance();
		const VertexAttribute<glm::vec3>* ref_pos = mesh.findAttribute<glm::vec3>(VertexAttributeIDs::getPositionName());
		assert(ref_pos != nullptr);

		// Clear list
		areaMap.clear();

		// Iterate and compute area for each triangle in the mesh
		TriangleIterator iterator(mesh);
		float total = 0.0f;
		while (!iterator.isDone())
		{
			Triangle triangle = iterator.next();
			const TriangleData<glm::vec3>& triangleData = triangle.getVertexData<glm::vec3>(*ref_pos);
			total += utility::computeTriangleArea(triangleData);
			areaMap.emplace(std::make_pair(total, triangle));
		}
		return total;
	}
}