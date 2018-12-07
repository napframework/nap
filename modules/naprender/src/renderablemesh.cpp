#include "renderablemesh.h"
#include "renderablemeshcomponent.h"

namespace nap
{
	RenderableMesh::RenderableMesh(IMesh& mesh, MaterialInstance& materialInstance, const VAOHandle& vaoHandle) :
		mMaterialInstance(&materialInstance),
        mMesh(&mesh),
        mVAOHandle(vaoHandle) { }


	void RenderableMesh::bind()
	{
		assert(mVAOHandle.isValid());
		mVAOHandle.get().bind();
	}


	void RenderableMesh::unbind()
	{
		assert(mVAOHandle.isValid());
		mVAOHandle.get().unbind();
	}

}

