#include "renderfftcomponent.h"

// External Includes
#include <entity.h>
#include <rendertotexturecomponent.h>
#include <computecomponent.h>

// nap::RenderFFTComponent run time class definition 
RTTI_BEGIN_CLASS(nap::RenderFFTComponent)
	//RTTI_PROPERTY("Ambient",			&nap::RenderFFTComponent::mAmbient,			nap::rtti::EPropertyMetaData::Required)
	//RTTI_PROPERTY("Diffuse",			&nap::RenderFFTComponent::mDiffuse,			nap::rtti::EPropertyMetaData::Required)
	//RTTI_PROPERTY("Specular",			&nap::RenderFFTComponent::mSpecular,			nap::rtti::EPropertyMetaData::Required)
	//RTTI_PROPERTY("Shininess",		&nap::RenderFFTComponent::mShininess,			nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("FFT",				&nap::RenderFFTComponent::mFFT,				nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

// nap::RenderFFTComponentInstance run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::RenderFFTComponentInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	void RenderFFTComponent::getDependentComponents(std::vector<rtti::TypeInfo>& components) const
	{
		components.emplace_back(RTTI_OF(RenderToTextureComponent));
		components.emplace_back(RTTI_OF(ComputeComponent));
	}


	bool RenderFFTComponentInstance::init(utility::ErrorState& errorState)
	{
		mResource = getComponent<RenderFFTComponent>();

		// Fetch dependent components
		mRenderComponent = &getEntityInstance()->getComponent<RenderToTextureComponentInstance>();
		if (!errorState.check(mRenderComponent != nullptr, "Missing `nap::RenderToTextureComponentInstance`"))
			return false;

		mComputeInstance = &getEntityInstance()->getComponent<ComputeComponentInstance>();
		if (!errorState.check(mComputeInstance != nullptr, "Missing `nap::ComputeComponentInstance`"))
			return false;

		// Validate material
		auto* uni = mRenderComponent->getMaterialInstance().getOrCreateUniform("UBO");
		if (!errorState.check(uni != nullptr, "Missing uniform struct with name `UBO`"))
			return false;

		mAmpsUniform = uni->getOrCreateUniform<UniformFloatArrayInstance>("amps");
		if (!errorState.check(mAmpsUniform != nullptr, "Missing uniform member with name `amps`"))
			return false;

		auto* res = uni->getOrCreateUniform<UniformVec2Instance>("resolution");
		if (!errorState.check(res != nullptr, "Missing uniform member with name `resolution`"))
			return false;

		res->setValue(mRenderComponent->getOutputTexture().getSize());

		const auto& amps = mFFT->getFFTBuffer().getAmplitudeSpectrum();
		if (!errorState.check(amps.size() == mAmpsUniform->getNumElements(), "Audio buffer size mismatch, must be %d elements", mAmpsUniform->getNumElements()))
			return false;

		return true;
	}


	void RenderFFTComponentInstance::update(double deltaTime)
	{
		const auto& amps = mFFT->getFFTBuffer().getAmplitudeSpectrum();
		assert(amps.size() == mAmpsUniform->getNumElements());
		mAmpsUniform->setValues(amps);
	}


	void RenderFFTComponentInstance::onDraw(IRenderTarget& renderTarget, VkCommandBuffer commandBuffer, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix)
	{
		// Get material to work with
		if (!mRenderableMesh.isValid())
		{
			assert(false);
			return;
		}

		// Set mvp matrices if present in material
		if (mProjectMatUniform != nullptr)
			mProjectMatUniform->setValue(projectionMatrix);

		if (mViewMatUniform != nullptr)
			mViewMatUniform->setValue(viewMatrix);

		if (mModelMatUniform != nullptr)
			mModelMatUniform->setValue(mTransformComponent->getGlobalTransform());

		if (mNormalMatrixUniform != nullptr)
			mNormalMatrixUniform->setValue(glm::transpose(glm::inverse(mTransformComponent->getGlobalTransform())));

		if (mCameraWorldPosUniform != nullptr)
			mCameraWorldPosUniform->setValue(math::extractPosition(glm::inverse(viewMatrix)));

		// Acquire new / unique descriptor set before rendering
		MaterialInstance& mat_instance = getMaterialInstance();
		const DescriptorSet& descriptor_set = mat_instance.update();

		// Fetch and bind pipeline
		utility::ErrorState error_state;
		RenderService::Pipeline pipeline = mRenderService->getOrCreatePipeline(renderTarget, mRenderableMesh.getMesh(), mat_instance, error_state);
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.mPipeline);

		// Bind shader descriptors
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.mLayout, 0, 1, &descriptor_set.mSet, 0, nullptr);

		// Bind vertex buffers
		const std::vector<VkBuffer>& vertexBuffers = mRenderableMesh.getVertexBuffers();
		const std::vector<VkDeviceSize>& vertexBufferOffsets = mRenderableMesh.getVertexBufferOffsets();
		vkCmdBindVertexBuffers(commandBuffer, 0, vertexBuffers.size(), vertexBuffers.data(), vertexBufferOffsets.data());

		// TODO: move to push/pop cliprect on RenderTarget once it has been ported
		bool has_clip_rect = mClipRect.hasWidth() && mClipRect.hasHeight();
		if (has_clip_rect)
		{
			VkRect2D rect;
			rect.offset.x = mClipRect.getMin().x;
			rect.offset.y = mClipRect.getMin().y;
			rect.extent.width = mClipRect.getWidth();
			rect.extent.height = mClipRect.getHeight();
			vkCmdSetScissor(commandBuffer, 0, 1, &rect);
		}

		// Set line width
		vkCmdSetLineWidth(commandBuffer, mLineWidth);

		// Draw meshes
		MeshInstance& mesh_instance = getMeshInstance();
		GPUMesh& mesh = mesh_instance.getGPUMesh();
		for (int index = 0; index < mesh_instance.getNumShapes(); ++index)
		{
			const IndexBuffer& index_buffer = mesh.getIndexBuffer(index);
			vkCmdBindIndexBuffer(commandBuffer, index_buffer.getBuffer(), 0, VK_INDEX_TYPE_UINT32);
			vkCmdDrawIndexed(commandBuffer, index_buffer.getCount(), 1, 0, 0, 0);
		}

		// Restore line width
		vkCmdSetLineWidth(commandBuffer, 1.0f);

		// Restore clipping
		if (has_clip_rect)
		{
			VkRect2D rect;
			rect.offset.x = 0;
			rect.offset.y = 0;
			rect.extent.width = renderTarget.getBufferSize().x;
			rect.extent.height = renderTarget.getBufferSize().y;
			vkCmdSetScissor(commandBuffer, 0, 1, &rect);
		}
	}
}
