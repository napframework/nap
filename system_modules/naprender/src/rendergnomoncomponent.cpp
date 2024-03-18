#include "rendergnomoncomponent.h"

// Local Includes
#include "gnomonshader.h"
#include "renderservice.h"
#include "renderglobals.h"

// External Includes
#include <entity.h>
#include <nap/core.h>
#include <nap/logger.h>

// nap::rendergnomoncomponent run time class definition 
RTTI_BEGIN_CLASS(nap::RenderGnomonComponent)
	RTTI_PROPERTY("Gnomon",		&nap::RenderGnomonComponent::mMesh,			nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("LineWidth",	&nap::RenderGnomonComponent::mLineWidth,	nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("DepthMode",	&nap::RenderGnomonComponent::mDepthMode,	nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

// nap::rendergnomoncomponentInstance run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::RenderGnomonComponentInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	void RenderGnomonComponent::getDependentComponents(std::vector<rtti::TypeInfo>& components) const
	{
		components.emplace_back(RTTI_OF(nap::TransformComponent));
	}


	bool RenderGnomonComponentInstance::init(utility::ErrorState& errorState)
	{
		// Init base class
		if (!RenderableComponentInstance::init(errorState))
			return false;

		// Get resource that created this instance
		RenderGnomonComponent* resource = getComponent<RenderGnomonComponent>();

		// Gnomon mesh is required
		if (!errorState.check(resource->mMesh != nullptr, "%s: missing Gnomon mesh", resource->mID.c_str()))
			return false;

		// Get transform
		mTransformComponent = getEntityInstance()->findComponent<TransformComponentInstance>();
		if (!errorState.check(mTransformComponent != nullptr, "%s: missing transform component", resource->mID.c_str()))
			return false;

		// Extract render service
		mRenderService = getEntityInstance()->getCore()->getService<RenderService>();
		assert(mRenderService != nullptr);

		// Get gnomon material
		Material* gnomon_material = mRenderService->getOrCreateMaterial<GnomonShader>(errorState);
		if (!errorState.check(gnomon_material != nullptr, "%s: unable to get or create gnomon material", resource->mID.c_str()))
			return false;

		// Create resource for the gnomon material instance
		mMaterialInstanceResource.mBlendMode = EBlendMode::Opaque;
		mMaterialInstanceResource.mDepthMode = resource->mDepthMode;
		mMaterialInstanceResource.mMaterial = gnomon_material;

		// Create gnomon material instance
		if (!mMaterialInstance.init(*mRenderService, mMaterialInstanceResource, errorState))
			return false;

		// Since the material can't be changed at run-time, cache the matrices to set on draw
		// If the struct is found, we expect the matrices with those names to be there
		// Ensure the mvp struct is available
		mMVPStruct = mMaterialInstance.getOrCreateUniform(uniform::mvpStruct);
		if (!errorState.check(mMVPStruct != nullptr, "%s: Unable to find uniform MVP struct: %s in material: %s",
			this->mID.c_str(), uniform::mvpStruct, mMaterialInstance.getMaterial().mID.c_str()))
			return false;

		// Get all matrices
		mModelMatUniform = ensureUniform(uniform::modelMatrix, errorState);
		mProjectMatUniform = ensureUniform(uniform::projectionMatrix, errorState);
		mViewMatUniform = ensureUniform(uniform::viewMatrix, errorState);

		if (mModelMatUniform == nullptr || mProjectMatUniform == nullptr || mViewMatUniform == nullptr)
			return false;

		// Create mesh / material combo that can be rendered to target
		mRenderableMesh = mRenderService->createRenderableMesh(*resource->mMesh, mMaterialInstance, errorState);
		if (!mRenderableMesh.isValid())
			return false;

		// Copy line width, ensure it's supported
		mLineWidth = resource->mLineWidth;
		if (mLineWidth > 1.0f && !mRenderService->getWideLinesSupported())
		{
			nap::Logger::warn("Unsupported line width: %.02f", mLineWidth);
			mLineWidth = 1.0f;
		}

		return true;
	}


	void RenderGnomonComponentInstance::onDraw(IRenderTarget& renderTarget, VkCommandBuffer commandBuffer, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix)
	{
		// Set mvp matrices
		mProjectMatUniform->setValue(projectionMatrix);
		mViewMatUniform->setValue(viewMatrix);
		mModelMatUniform->setValue(mTransformComponent->getGlobalTransform());

		// Acquire new / unique descriptor set before rendering
		const DescriptorSet& descriptor_set = mRenderableMesh.getMaterialInstance().update();

		// Fetch and bind pipeline
		utility::ErrorState error_state;
		RenderService::Pipeline pipeline = mRenderService->getOrCreatePipeline(renderTarget, mRenderableMesh, error_state);
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.mPipeline);

		// Bind shader descriptors
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.mLayout, 0, 1, &descriptor_set.mSet, 0, nullptr);

		// Bind vertex buffers
		const std::vector<VkBuffer>& vertexBuffers = mRenderableMesh.getVertexBuffers();
		const std::vector<VkDeviceSize>& vertexBufferOffsets = mRenderableMesh.getVertexBufferOffsets();
		vkCmdBindVertexBuffers(commandBuffer, 0, vertexBuffers.size(), vertexBuffers.data(), vertexBufferOffsets.data());

		// Set line width
		vkCmdSetLineWidth(commandBuffer, mLineWidth);

		// Draw meshes
		MeshInstance& mesh_instance = mRenderableMesh.getMesh().getMeshInstance();
		GPUMesh& mesh = mesh_instance.getGPUMesh();
		for (int index = 0; index < mesh_instance.getNumShapes(); ++index)
		{
			const IndexBuffer& index_buffer = mesh.getIndexBuffer(index);
			vkCmdBindIndexBuffer(commandBuffer, index_buffer.getBuffer(), 0, VK_INDEX_TYPE_UINT32);
			vkCmdDrawIndexed(commandBuffer, index_buffer.getCount(), 1, 0, 0, 0);
		}

		// Restore line width
		vkCmdSetLineWidth(commandBuffer, 1.0f);
	}


	nap::UniformMat4Instance* RenderGnomonComponentInstance::ensureUniform(const std::string& uniformName, utility::ErrorState& error)
	{
		assert(mMVPStruct != nullptr);
		UniformMat4Instance* found_uniform = mMVPStruct->getOrCreateUniform<UniformMat4Instance>(uniformName);
		if (!error.check(found_uniform != nullptr,
			"%s: unable to find uniform: %s in material: %s", this->mID.c_str(), uniformName.c_str(),
			mMaterialInstance.getMaterial().mID.c_str()))
			return nullptr;
		return found_uniform;
	}
}
