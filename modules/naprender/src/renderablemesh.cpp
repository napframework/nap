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
		mMaterialInstance->pipelineStateChanged.connect(mPipelineStateChangedSlot);
	}

	RenderableMesh::RenderableMesh(const RenderableMesh& renderableMesh) :
		mMaterialInstance(renderableMesh.mMaterialInstance),
		mMesh(renderableMesh.mMesh),
		mPipelineLayout(renderableMesh.mPipelineLayout),
		mPipeline(renderableMesh.mPipeline)
	{
		mMaterialInstance->pipelineStateChanged.connect(mPipelineStateChangedSlot);
	}

	RenderableMesh& RenderableMesh::operator=(const RenderableMesh& renderableMesh)
	{
		if (this != &renderableMesh)
		{
			mMaterialInstance = renderableMesh.mMaterialInstance;
			mMesh = renderableMesh.mMesh;
			mPipelineLayout = renderableMesh.mPipelineLayout;
			mPipeline = renderableMesh.mPipeline;

			mMaterialInstance->pipelineStateChanged.connect(mPipelineStateChangedSlot);
		}
		return *this;
	}

	void RenderableMesh::onPipelineStateChanged(const MaterialInstance& materialInstance, RenderService& renderService)
	{
		renderService.recreatePipeline(*this, mPipelineLayout, mPipeline);;
	}
}

