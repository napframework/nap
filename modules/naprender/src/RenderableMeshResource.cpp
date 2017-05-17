// Local Includes
#include "RenderableMeshResource.h"
#include "RenderService.h"

// External Includes
#include <nap/logger.h>
#include <nap/fileutils.h>
#include "fbxconverter.h"
#include "material.h"
#include "meshresource.h"

RTTI_BEGIN_CLASS(nap::RenderableMeshResource)
	RTTI_PROPERTY("Material",	 &nap::RenderableMeshResource::mMaterialResource,	nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Mesh",		 &nap::RenderableMeshResource::mMeshResource,		nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

namespace nap
{
	RenderableMeshResource::~RenderableMeshResource()
	{
		// VAO is allocated if object had init() call
		if (mVAO != nullptr)
		{
			assert (mRenderService != nullptr);
			mRenderService->queueResourceForDestruction(std::move(mVAO));
		}
	}


	bool RenderableMeshResource::init(utility::ErrorState& errorState)
	{
		mPrevVAO = std::move(mVAO);
		mVAO = std::make_unique<opengl::VertexArrayObject>();

		for (auto& kvp : mMaterialResource->getShader()->getShader().getAttributes())
		{
			const opengl::VertexAttribute* shader_vertex_attribute = kvp.second.get();

			const Material::VertexAttributeBinding* material_binding = mMaterialResource->findVertexAttributeBinding(kvp.first);
			if (!errorState.check(material_binding != nullptr, "Unable to find binding %s for shader %s in material %s", kvp.first.c_str(), mMaterialResource->getShader()->mVertPath.c_str(), mMaterialResource->mID.c_str()))
				return false;
			
			const opengl::VertexAttributeBuffer* vertex_buffer = mMeshResource->getMesh().findVertexAttributeBuffer(material_binding->mMeshAttributeID);
			if (!errorState.check(shader_vertex_attribute != nullptr, "Unable to find vertex attribute %s in mesh %s", material_binding->mMeshAttributeID.c_str(), mMeshResource->mPath.c_str()))
				return false;

			mVAO->addVertexBuffer(shader_vertex_attribute->mLocation, *vertex_buffer);
		}
		
 		return true;
	}


	void RenderableMeshResource::finish(Resource::EFinishMode mode)
	{
		if (mode == Resource::EFinishMode::COMMIT)
		{
			mRenderService->queueResourceForDestruction(std::move(mPrevVAO)); 
		}
		else
		{
			assert(mode == Resource::EFinishMode::ROLLBACK);
			mRenderService->queueResourceForDestruction(std::move(mVAO)); 
			mVAO = std::move(mPrevVAO);
		}
	}


	const std::string RenderableMeshResource::getDisplayName() const
	{
		return "ModelResource";
	}
}

RTTI_DEFINE(nap::RenderableMeshResource)