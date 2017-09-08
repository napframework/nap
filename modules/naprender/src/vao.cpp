// Local Includes
#include "vao.h"
#include "renderservice.h"

namespace nap
{
	VAOKey::VAOKey(const Material& material, const MeshInstance& meshResource) :
		mMaterial(material),
		mMeshResource(meshResource)
	{
	}


	std::unique_ptr<VAOHandle> VAOHandle::create(RenderService& renderService, opengl::VertexArrayObject* object)
	{
		VAOHandle* handle = new VAOHandle(renderService, object);
		return std::unique_ptr<VAOHandle>(handle);
	}


	VAOHandle::VAOHandle(RenderService& renderService, opengl::VertexArrayObject* object) :
		mRenderService(renderService),
		mObject(object)
	{
	}


	VAOHandle::~VAOHandle()
	{
		mRenderService.releaseVertexArrayObject(mObject);
	}
}