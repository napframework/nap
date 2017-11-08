#include "renderablemesh.h"
#include "renderablemeshcomponent.h"

namespace nap
{
	RenderableMesh::RenderableMesh(IMesh& mesh, MaterialInstance& materialInstance, const VAOHandle& vaoHandle) :
		mMaterialInstance(&materialInstance),
        mMesh(&mesh),
        mVAOHandle(vaoHandle) { }
}

