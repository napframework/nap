#include "visualizenormalsmesh.h"
#include <rtti/rtti.h>
#include "meshutils.h"
#include <nap/logger.h>

RTTI_BEGIN_CLASS(nap::VisualizeNormalsMesh)
	RTTI_PROPERTY("ReferenceMesh", &nap::VisualizeNormalsMesh::mReferenceMesh, nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Length", &nap::VisualizeNormalsMesh::mNormalLength, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

namespace nap
{
	bool VisualizeNormalsMesh::init(utility::ErrorState& errorState)
	{
		if (!setup(errorState))
			return false;

		if (!mMeshInstance->init(errorState))
			return false;

		return true;
	}


	bool VisualizeNormalsMesh::updateNormals(utility::ErrorState& error, bool push)
	{
		const nap::MeshInstance& reference_mesh = mReferenceMesh.get()->getMeshInstance();

		// Get reference normals and vertices
		const std::vector<glm::vec3>& ref_normals  = reference_mesh.getAttribute<glm::vec3>(VertexAttributeIDs::getNormalName()).getData();
		const std::vector<glm::vec3>& ref_vertices = reference_mesh.getAttribute<glm::vec3>(VertexAttributeIDs::getPositionName()).getData();
		
		// Get reference uvs
		std::vector<const std::vector<glm::vec3>*> ref_uvs;
		std::vector<std::vector<glm::vec3>*> tar_uvs;
		ref_uvs.reserve(mUvAttrs.size());
		tar_uvs.reserve(mUvAttrs.size());

		// Get reference clrs
		std::vector<const std::vector<glm::vec4>*> ref_clrs;
		std::vector<std::vector<glm::vec4>*> tar_clrs;
		ref_clrs.reserve(mColorAttrs.size());
		tar_clrs.reserve(mColorAttrs.size());

		// Query uv data
		for (int i = 0; i < mUvAttrs.size(); i++)
		{
			const Vec3VertexAttribute* ref_uv_attr = reference_mesh.findAttribute<glm::vec3>(VertexAttributeIDs::getUVName(i));
			if (!error.check(ref_uv_attr != nullptr, "unable to find uv attribute on reference mesh: %s with index: %d", mReferenceMesh->mID.c_str(), i))
				return false;

			ref_uvs.emplace_back(&(ref_uv_attr->getData()));
			tar_uvs.emplace_back(&(mUvAttrs[i]->getData()));
		}

		// Query color data
		for (int i = 0; i < mColorAttrs.size(); i++)
		{
			const Vec4VertexAttribute* ref_clr_attr = reference_mesh.findAttribute<glm::vec4>(VertexAttributeIDs::GetColorName(i));
			if (!error.check(ref_clr_attr!= nullptr, "unable to find uv attribute on reference mesh: %s with index: %d", mReferenceMesh->mID.c_str(), i))
				return false;

			ref_clrs.emplace_back(&(ref_clr_attr->getData()));
			tar_clrs.emplace_back(&(mColorAttrs[i]->getData()));
		}

		// Get single buffers to populate
		std::vector<glm::vec3>& target_vertices = mPositionAttr->getData();
		std::vector<float>& target_tips = mTipAttr->getData();

		int vert_count = reference_mesh.getNumVertices();
		int target_idx = 0;

		for (int i = 0; i < vert_count; i++)
		{
			// get current position and normal
			const glm::vec3& ref_vertex = ref_vertices[i];
			const glm::vec3& ref_normal = ref_normals[i];

			// Ensure the normal has length
			assert(glm::length(ref_normal) > 0);

			// Set vertices
			target_vertices[target_idx] = ref_vertex;
			target_vertices[target_idx + 1] = ref_vertex + (glm::normalize(ref_normal) * mNormalLength);

			// Set tip values
			target_tips[target_idx + 0] = 1.0f;
			target_tips[target_idx + 1] = 0.0f;

			// Copy over the uv coordinate for every uv set in the reference mesh
			int uv_idx = 0;
			for (auto& uv_attr : mUvAttrs)
			{
				(*(tar_uvs[uv_idx]))[target_idx + 0] = (*(ref_uvs[uv_idx]))[i];
				(*(tar_uvs[uv_idx]))[target_idx + 1] = (*(ref_uvs[uv_idx]))[i];
				uv_idx++;
			}

			// Copy over the color value for every color set in the reference mesh
			int clr_idx = 0;
			for (auto& clr_attr : mColorAttrs)
			{
				(*(tar_clrs[clr_idx]))[target_idx + 0] = (*(ref_clrs[clr_idx]))[i];
				(*(tar_clrs[clr_idx]))[target_idx + 1] = (*(ref_clrs[clr_idx]))[i];
				clr_idx++;
			}

			// Increment write index
			target_idx += 2;
		}

		if (push)
		{
			return mMeshInstance->update(error);
		}
		return true;
	}


	bool VisualizeNormalsMesh::setup(utility::ErrorState& error)
	{
		// Create the mesh that will hold the normals
		mMeshInstance = std::make_unique<MeshInstance>();

		nap::IMesh* reference_mesh = mReferenceMesh.get();

		// Make sure the reference mesh has normals
		if (reference_mesh->getMeshInstance().findAttribute<glm::vec3>(VertexAttributeIDs::getNormalName()) == nullptr)
			return error.check(false, "reference mesh has no normals");

		// Create position and vertex attribute
		mPositionAttr = &(mMeshInstance->getOrCreateAttribute<glm::vec3>(VertexAttributeIDs::getPositionName()));

		// Create tip attribute
		mTipAttr = &(mMeshInstance->getOrCreateAttribute<float>("Tip"));

		// Sample all uv sets
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

		// Total number of vertices associated with reference mesh
		int vertex_count = reference_mesh->getMeshInstance().getNumVertices();

		// Create initial position data
		std::vector<glm::vec3> vertices(vertex_count * 2, { 0.0f, 0.0f, 0.0f });
		mPositionAttr->setData(vertices);

		// Create initial tip data
		std::vector<float> tips(vertex_count * 2, 1.0f);
		mTipAttr->setData(tips);

		// Create initial uv data
		std::vector<glm::vec3> uvs(vertex_count * 2, { 0.0f, 0.0f, 0.0f });
		for (auto& uv_attr : mUvAttrs)
			uv_attr->setData(uvs);

		// Create initial color data
		std::vector<glm::vec4> colors(vertex_count * 2, { 1.0f, 1.0f, 1.0f, 1.0f });
		for (auto& color_attr : mColorAttrs)
			color_attr->setData(colors);

		// Set number of vertices
		mMeshInstance->setNumVertices(vertex_count * 2);

		// Update normals (ie -> calculate position, clr, tip value etc and push to gpu)
		updateNormals(error, false);

		// Draw normals as lines
		MeshShape& shape = mMeshInstance->createShape();
		shape.setDrawMode(opengl::EDrawMode::LINES);

		utility::generateIndices(shape, vertex_count * 2);

		return true;
	}

}