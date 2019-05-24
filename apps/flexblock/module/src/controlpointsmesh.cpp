#include "controlpointsmesh.h"
#include "meshutils.h"

// External Includes
#include <mathutils.h>

RTTI_BEGIN_CLASS(nap::ControlPointsMesh)
	RTTI_PROPERTY("ReferenceMesh", &nap::ControlPointsMesh::mReferenceMesh, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	bool ControlPointsMesh::init(utility::ErrorState& errorState)
	{
		// Make sure the reference mesh contains triangles
		bool contains_triangles = false;
		for (int i = 0; i < mReferenceMesh->getMeshInstance().getNumShapes(); i++)
		{
			if (utility::isTriangleMesh(mReferenceMesh->getMeshInstance().getShape(i)))
				contains_triangles = true;
		}

		if (!errorState.check(contains_triangles, "reference mesh doesn't contain any triangles: %s", this->mID.c_str()))
			return false;

		// Create mesh instance
		if (!createMeshInstance(errorState))
			return false;

		// Setup instance (allocate vertex attributes)
		if (!setup(errorState))
			return false;

		return true;
	}


	bool ControlPointsMesh::createMeshInstance(nap::utility::ErrorState& error)
	{
		if (!error.check(mMeshInstance == nullptr, "unable to create new mesh, already assigned: %s", this->mID.c_str()))
			return false;

		// There is only 1 shape associated with the scatter mesh
		mMeshInstance = std::make_unique<MeshInstance>();
		mMeshInstance->createShape();

		return true;
	}

	void ControlPointsMesh::setControlPoints(std::vector<glm::vec3> controlPoints)
	{
		mPositionAttr->setData(controlPoints);

		utility::ErrorState error;
		mMeshInstance->update(error);
	}

	bool ControlPointsMesh::setup(nap::utility::ErrorState& error)
	{
		assert(mMeshInstance != nullptr);
		mMeshInstance->setNumVertices(8);

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

		auto box = mReferenceMesh->getBox();

		// Create initial position data
		auto verts = std::vector<glm::vec3>(8, { 0.0f, 0.0f, 0.0f });

		const glm::vec3& min = box.getMin();
		const glm::vec3& max = box.getMax();

		verts[0] = { min.x, min.y, max.z };	//< Front Lower left
		verts[1] = { max.x, min.y, max.z };	//< Front Lower right
		verts[2] = { min.x, max.y, max.z };	//< Front Top left
		verts[3] = { max.x, max.y, max.z };	//< Front Top right

		verts[4] = { max.x, min.y, min.z };	//< Back Lower left
		verts[5] = { min.x, min.y, min.z };	//< Back lower right
		verts[6] = { max.x, max.y, min.z }; //< Back Top left
		verts[7] = { min.x, max.y, min.z };	//< Back Top right

		mPositionAttr->setData(verts);

		// Create initial normal data
		std::vector<glm::vec3> normals(8, { 0.0f, 1.0f,0.0f });
		mNormalsAttr->setData(normals);

		// Create initial uv data
		std::vector<glm::vec3> uvs(8, { 0.0f, 0.0f, 0.0f });
		for (auto& uv_attr : mUvAttrs)
			uv_attr->setData(uvs);

		// Create initial color data
		std::vector<glm::vec4> colors(8, { 1.0f, 0.0f, 0.0f, 1.0f });
		for (auto& color_attr : mColorAttrs)
			color_attr->setData(colors);

		// Set number of vertices
		mMeshInstance->setNumVertices(8);

		// Draw as points
		MeshShape& shape = mMeshInstance->getShape(0);
		shape.setDrawMode(opengl::EDrawMode::POINTS);

		// Automatically generate indices
		utility::generateIndices(shape, 8);

		return mMeshInstance->init(error);
	}
}