#include "renderablemesh.h"
#include "renderablemeshcomponent.h"

namespace nap
{
	RenderableMesh::RenderableMesh(IMesh& mesh, MaterialInstance& materialInstance, VkPipelineLayout layout, VkPipeline pipeline) :
		mMaterialInstance(&materialInstance),
		mMesh(&mesh),
		mPipelineLayout(layout),
		mPipeline(pipeline)
	{
	}


	void RenderableMesh::bind()
	{
	}


	void RenderableMesh::unbind()
	{
	}

}

