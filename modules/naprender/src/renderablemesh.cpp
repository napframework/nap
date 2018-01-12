#include "renderablemesh.h"
#include "renderablemeshcomponent.h"

namespace nap
{
	RenderableMesh::RenderableMesh(IMesh& mesh, MaterialInstance& materialInstance, const VAOHandle& vaoHandle) :
        mMesh(&mesh),
		mMaterialInstance(&materialInstance),
        mVAOHandle(vaoHandle) { }
}

