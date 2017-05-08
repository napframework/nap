// Local Includes
#include "modelresource.h"

// External Includes
#include <nap/logger.h>
#include <nap/fileutils.h>
#include "fbxconverter.h"
#include "material.h"
#include "meshresource.h"
#include "shaderutils.h"

RTTI_BEGIN_CLASS(nap::ModelResource)
	RTTI_PROPERTY_REQUIRED("Material", &nap::ModelResource::mMaterialResource)
	RTTI_PROPERTY_REQUIRED("Mesh", &nap::ModelResource::mMeshResource)
RTTI_END_CLASS

namespace nap
{
	bool ModelResource::init(InitResult& initResult)
	{
		mVAO.setDrawMode(opengl::DrawMode::TRIANGLES);

		for (auto& kvp : mMaterialResource->getShader()->getShader().getAttributes())
		{
			const opengl::VertexAttribute* shader_vertex_attribute = kvp.second.get();

			const Material::VertexAttributeBinding* material_binding = mMaterialResource->findVertexAttributeBinding(kvp.first);
			if (!initResult.check(material_binding != nullptr, "Unable to find binding %s for shader %s in material %s", kvp.first.c_str(), mMaterialResource->getShader()->mVertPath.c_str(), mMaterialResource->mID.c_str()))
				return false;
			
			const opengl::VertexBuffer* vertex_buffer = mMeshResource->getMesh().findVertexAttributeBuffer(material_binding->mMeshAttributeID);
			if (!initResult.check(shader_vertex_attribute != nullptr, "Unable to find vertex attribute %s in mesh %s", material_binding->mMeshAttributeID.c_str(), mMeshResource->mPath.c_str()))
				return false;

			mVAO.addVertexBuffer(shader_vertex_attribute->mLocation, *vertex_buffer);
		}
		
		mVAO.setIndexBuffer(*mMeshResource->getMesh().getIndexBuffer());
		mVAO.setNumVertices(mMeshResource->getMesh().getVertCount());
 
 		return true;
	}

	void ModelResource::finish(Resource::EFinishMode mode)
	{
// 		if (mode == Resource::EFinishMode::COMMIT)
// 		{
// 			mPrevMesh = nullptr;
// 		}
// 		else
// 		{
// 			assert(mode == Resource::EFinishMode::ROLLBACK);
// 			mMesh = std::move(mPrevMesh);
// 		}
	}

	void ModelResource::draw()
	{
		mVAO.draw();
	}

	const std::string ModelResource::getDisplayName() const
	{
		return "ModelResource";
	}
}

RTTI_DEFINE(nap::MeshResource)