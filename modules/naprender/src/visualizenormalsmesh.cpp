/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "visualizenormalsmesh.h"
#include "meshutils.h"
#include "renderservice.h"
#include "renderglobals.h"

// External Includes
#include <rtti/rtti.h>
#include <nap/logger.h>
#include <nap/core.h>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::VisualizeNormalsMesh)
	RTTI_CONSTRUCTOR(nap::Core&)
	RTTI_PROPERTY("Usage",			&nap::VisualizeNormalsMesh::mUsage,			nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("ReferenceMesh",	&nap::VisualizeNormalsMesh::mReferenceMesh, nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Length",			&nap::VisualizeNormalsMesh::mNormalLength,	nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

namespace nap
{
	VisualizeNormalsMesh::VisualizeNormalsMesh(Core& core) : mRenderService(core.getService<nap::RenderService>())
	{ }


	bool VisualizeNormalsMesh::init(utility::ErrorState& errorState)
	{
		if (!createMeshInstance(errorState))
			return false;

		if (mReferenceMesh != nullptr)
		{
			if (!setReferenceMesh(*mReferenceMesh, errorState))
				return false;

			if (!calculateNormals(errorState, false))
				return false;
		}

		if (!mMeshInstance->init(errorState))
			return false;

		return true;
	}


	bool VisualizeNormalsMesh::calculateNormals(utility::ErrorState& error, bool push)
	{
		assert(mCurrentReferenceMesh != nullptr);

		const nap::MeshInstance& reference_mesh = mCurrentReferenceMesh->getMeshInstance();

		// Get reference normals and vertices
		const std::vector<glm::vec3>& ref_normals  = reference_mesh.getAttribute<glm::vec3>(vertexid::normal).getData();
		const std::vector<glm::vec3>& ref_vertices = reference_mesh.getAttribute<glm::vec3>(vertexid::position).getData();

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
			const Vec3VertexAttribute* ref_uv_attr = reference_mesh.findAttribute<glm::vec3>(vertexid::getUVName(i));
			if (!error.check(ref_uv_attr != nullptr, "unable to find uv attribute on reference mesh: %s with index: %d", mCurrentReferenceMesh->mID.c_str(), i))
				return false;

			ref_uvs.emplace_back(&(ref_uv_attr->getData()));
			tar_uvs.emplace_back(&(mUvAttrs[i]->getData()));
		}

		// Query color data
		for (int i = 0; i < mColorAttrs.size(); i++)
		{
			const Vec4VertexAttribute* ref_clr_attr = reference_mesh.findAttribute<glm::vec4>(vertexid::getColorName(i));
			if (!error.check(ref_clr_attr!= nullptr, "unable to find uv attribute on reference mesh: %s with index: %d", mCurrentReferenceMesh->mID.c_str(), i))
				return false;

			ref_clrs.emplace_back(&(ref_clr_attr->getData()));
			tar_clrs.emplace_back(&(mColorAttrs[i]->getData()));
		}

		// Get single buffers to populate
		std::vector<glm::vec3>& target_vertices = mPositionAttr->getData();
		std::vector<glm::vec3>& target_normals = mNormalsAttr->getData();
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

			// Normalize reference normal
			glm::vec3 nnormal = glm::normalize(ref_normal) * mNormalLength;

			// Set vertices
			target_vertices[target_idx] = ref_vertex;
			target_vertices[target_idx + 1] = ref_vertex + nnormal;

			// Set normals
			target_normals[target_idx + 0] = nnormal;
			target_normals[target_idx + 1] = nnormal;

			// Set tip values
			target_tips[target_idx + 0] = 1.0f;
			target_tips[target_idx + 1] = 0.0f;

			// Copy over the uv coordinate for every uv set in the reference mesh
            for (auto uv_idx = 0; uv_idx < mUvAttrs.size(); uv_idx++)
			{
				(*(tar_uvs[uv_idx]))[target_idx + 0] = (*(ref_uvs[uv_idx]))[i];
				(*(tar_uvs[uv_idx]))[target_idx + 1] = (*(ref_uvs[uv_idx]))[i];
			}

			// Copy over the color value for every color set in the reference mesh
            for (auto clr_idx = 0; clr_idx < mColorAttrs.size(); clr_idx++)
			{
				(*(tar_clrs[clr_idx]))[target_idx + 0] = (*(ref_clrs[clr_idx]))[i];
				(*(tar_clrs[clr_idx]))[target_idx + 1] = (*(ref_clrs[clr_idx]))[i];
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


	bool VisualizeNormalsMesh::setReferenceMesh(IMesh& mesh, nap::utility::ErrorState& error)
	{
		mCurrentReferenceMesh = &mesh;
		if (!setup(error))
			return false;

		return true;
	}


	bool VisualizeNormalsMesh::createMeshInstance(nap::utility::ErrorState& error)
	{
		// Make sure we're not creating a new mesh mesh
		if (!error.check(mMeshInstance == nullptr, "%s already has a mesh!", mID.c_str()))
			return false;

		// Create the mesh that will hold the normals
		assert(mRenderService != nullptr);
		mMeshInstance = std::make_unique<MeshInstance>(*mRenderService);
		mMeshInstance->setUsage(mUsage);
		mMeshInstance->setDrawMode(EDrawMode::Lines);
		mMeshInstance->setPolygonMode(EPolygonMode::Fill);

		// Create shape that holds the normals
		mMeshInstance->createShape();

		return true;
	}


	bool VisualizeNormalsMesh::setup(utility::ErrorState& error)
	{
		assert(mCurrentReferenceMesh != nullptr);

		// Make sure the reference mesh has normals
		if (mCurrentReferenceMesh->getMeshInstance().findAttribute<glm::vec3>(vertexid::normal) == nullptr)
			return error.check(false, "reference mesh has no normals");

		// Create position and vertex attribute
		mPositionAttr = &(mMeshInstance->getOrCreateAttribute<glm::vec3>(vertexid::position));

		// Create tip attribute
		mTipAttr = &(mMeshInstance->getOrCreateAttribute<float>("Tip"));

		// Create normals attribute
		mNormalsAttr = &(mMeshInstance->getOrCreateAttribute<glm::vec3>(vertexid::normal));

		// Sample all uv sets
		mUvAttrs.clear();
		int uv_idx = 0;
		while (true)
		{
			const Vec3VertexAttribute* ref_uv_attr = mCurrentReferenceMesh->getMeshInstance().findAttribute<glm::vec3>(vertexid::getUVName(uv_idx));
			if (ref_uv_attr != nullptr)
			{
				mUvAttrs.emplace_back(&(mMeshInstance->getOrCreateAttribute<glm::vec3>(vertexid::getUVName(uv_idx))));
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
			const Vec4VertexAttribute* ref_clr_attr = mCurrentReferenceMesh->getMeshInstance().findAttribute<glm::vec4>(vertexid::getColorName(clr_idx));
			if (ref_clr_attr != nullptr)
			{
				mColorAttrs.emplace_back(&(mMeshInstance->getOrCreateAttribute<glm::vec4>(vertexid::getColorName(clr_idx))));
				clr_idx++;
				continue;
			}
			break;
		}

		// Total number of vertices associated with reference mesh
		int vertex_count = mCurrentReferenceMesh->getMeshInstance().getNumVertices();

		// Create initial position data
		std::vector<glm::vec3> vertices(vertex_count * 2, { 0.0f, 0.0f, 0.0f });
		mPositionAttr->setData(vertices);

		// Create initial normal data
		std::vector<glm::vec3> normals(vertex_count * 2, { 0.0f,1.0f,0.0f });
		mNormalsAttr->setData(normals);

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

		// Draw normals as lines
		MeshShape& shape = mMeshInstance->getShape(0);

		// Automatically generate indices
		utility::generateIndices(shape, vertex_count * 2, false);

		return true;
	}

}
