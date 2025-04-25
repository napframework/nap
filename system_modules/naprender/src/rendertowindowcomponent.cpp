#include "rendertowindowcomponent.h"

// Local includes
#include "renderglobals.h"
#include "renderservice.h"
#include "orthocameracomponent.h"

// External includes
#include <entity.h>
#include <nap/core.h>

RTTI_BEGIN_ENUM(nap::RenderToWindowComponent::EScaleMode)
	RTTI_ENUM_VALUE(nap::RenderToWindowComponent::EScaleMode::Window, "Window"),
	RTTI_ENUM_VALUE(nap::RenderToWindowComponent::EScaleMode::Square, "Square"),
	RTTI_ENUM_VALUE(nap::RenderToWindowComponent::EScaleMode::Custom, "CustomRatio")
RTTI_END_ENUM

RTTI_BEGIN_CLASS(nap::RenderToWindowComponent)
	RTTI_PROPERTY("Window",				&nap::RenderToWindowComponent::mWindow,						nap::rtti::EPropertyMetaData::Required, "Target window")
	RTTI_PROPERTY("ScaleMode",			&nap::RenderToWindowComponent::mMode,						nap::rtti::EPropertyMetaData::Default,  "Canvas scaling method")
	RTTI_PROPERTY("Ratio",				&nap::RenderToWindowComponent::mRatio,						nap::rtti::EPropertyMetaData::Default,	"Ratio to use when mode is set to 'Custom'")
	RTTI_PROPERTY("MaterialInstance",	&nap::RenderToWindowComponent::mMaterialInstanceResource, nap::rtti::EPropertyMetaData::Required, "Render material, including overrides")
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::RenderToWindowComponentInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	static UniformMat4Instance* ensureUniform(const std::string&& uniformName, nap::UniformStructInstance& mvpStruct, utility::ErrorState& error)
	{
		auto* found_mat = mvpStruct.getOrCreateUniform<UniformMat4Instance>(uniformName);
		if (!error.check(found_mat != nullptr,
			"Unable to find uniform '%s' in struct '%s'", uniformName.c_str(), mvpStruct.getDeclaration().mName.c_str()))
			return nullptr;
		return found_mat;
	}


	static void computeModelMatrix(const nap::IRenderTarget& target, glm::vec2 sourceRatio, glm::mat4& outMatrix)
	{
		// Center
		glm::vec2 target_size = target.getBufferSize();
		outMatrix = glm::translate(glm::identity<glm::mat4>(),
			{
				target_size.x * 0.5f,
				target_size.y * 0.5f,
				0.0f
			});

		// Compute ratio
		float source_ratio = sourceRatio.x / sourceRatio.y;
		float target_ratio = target_size.x / target_size.y;

		// Compute ratio compensated scale
		target_size = target_ratio < source_ratio ?
			glm::vec2(target_size.x, target_size.x / source_ratio) :
			glm::vec2(target_size.y * source_ratio, target_size.y);

		// Scale
		outMatrix = glm::scale(outMatrix, { target_size.x, target_size.y, 1.0f });
	}


	RenderToWindowComponentInstance::RenderToWindowComponentInstance(EntityInstance& entity, Component& resource) :
		RenderableComponentInstance(entity, resource),
		mPlane(*entity.getCore())
	{ }


	bool RenderToWindowComponentInstance::init(utility::ErrorState& errorState)
	{
		if (!RenderableComponentInstance::init(errorState))
			return false;

		// Get resource
		auto* resource = getComponent<RenderToWindowComponent>();

		// Now create a plane and initialize it.
		// The model matrix is computed on draw and used to scale the model to fit target bounds.
		mPlane.mSize = glm::vec2(1.0f, 1.0f);
		mPlane.mPosition = glm::vec3(0.0f, 0.0f, 0.0f);
		mPlane.mUsage = EMemoryUsage::Static;
		mPlane.mCullMode = ECullMode::Back;
		mPlane.mColumns = 1;
		mPlane.mRows = 1;

		// Initialize plane geometry
		if (!mPlane.init(errorState))
			return false;

		// Get render service
		mService = getEntityInstance()->getCore()->getService<RenderService>();
		assert(mService != nullptr);

		// Create material instance
		if (!mMaterialInstance.init(*mService, resource->mMaterialInstanceResource, errorState))
			return false;

		// Ensure the mvp struct is available
		mMVPStruct = mMaterialInstance.getOrCreateUniform(uniform::mvpStruct);
		if (!errorState.check(mMVPStruct != nullptr, "%s: Unable to find uniform MVP struct: %s in material: %s",
			this->mID.c_str(), uniform::mvpStruct, mMaterialInstance.getMaterial().mID.c_str()))
			return false;

		// Get model matrix (required)
		mModelMatrixUniform = ensureUniform(uniform::modelMatrix, *mMVPStruct, errorState);
		if (mModelMatrixUniform == nullptr)
			return false;

		// Get projection matrix (required)
		mProjectMatrixUniform = ensureUniform(uniform::projectionMatrix, *mMVPStruct, errorState);
		if (mProjectMatrixUniform == nullptr)
			return false;

		// Get view matrix (optional)
		utility::ErrorState view_error;
		mViewMatrixUniform = ensureUniform(uniform::viewMatrix, *mMVPStruct, view_error);

		// Create the renderable mesh, which represents a valid mesh / material combination
		mRenderableMesh = mService->createRenderableMesh(mPlane, mMaterialInstance, errorState);
		if (!mRenderableMesh.isValid())
			return false;

		// Store resources
		mMode = resource->mMode;
		mRatio = resource->mRatio;
		mWindow = resource->mWindow.get();

		return true;
	}


	bool RenderToWindowComponentInstance::isSupported(nap::CameraComponentInstance& camera) const
	{
		return camera.get_type().is_derived_from(RTTI_OF(OrthoCameraComponentInstance));
	}


	void RenderToWindowComponentInstance::draw()
	{
		if (mService->beginRecording(*mWindow))
		{
			// Create projection matrix
			VkCommandBuffer command_buffer = mService->getCurrentCommandBuffer();
			glm::ivec2 size = mWindow->getBufferSize();
			glm::mat4 proj_matrix = OrthoCameraComponentInstance::createRenderProjectionMatrix(0.0f, (float)size.x, 0.0f, (float)size.y);

			// Draw
			mWindow->beginRendering();
			onDraw(*mWindow, command_buffer, glm::identity<glm::mat4>(), proj_matrix);
			mWindow->endRendering();
			mService->endRecording();
		}
	}


	void RenderToWindowComponentInstance::onDraw(IRenderTarget& renderTarget, VkCommandBuffer commandBuffer, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix)
	{
		// Update the model matrix so that the plane mesh is of the same size as the render target
		computeModelMatrix(renderTarget, getRatio(), mModelMatrix);
		mModelMatrixUniform->setValue(mModelMatrix);

		// Update matrices, projection and model are required
		mProjectMatrixUniform->setValue(projectionMatrix);

		// If view matrix exposed on shader, set it as well
		if (mViewMatrixUniform != nullptr)
			mViewMatrixUniform->setValue(viewMatrix);

		// Get valid descriptor set
		MaterialInstance& mat_instance = getMaterialInstance();
		const DescriptorSet& descriptor_set = mat_instance.update();

		// Gather draw info
		MeshInstance& mesh_instance = mRenderableMesh.getMesh().getMeshInstance();
		GPUMesh& mesh = mesh_instance.getGPUMesh();

		// Get pipeline to to render with
		utility::ErrorState error_state;
		RenderService::Pipeline pipeline = mService->getOrCreatePipeline(renderTarget, mRenderableMesh.getMesh(), mat_instance, error_state);
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.mPipeline);
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.mLayout, 0, 1, &descriptor_set.mSet, 0, nullptr);

		// Bind buffers and draw
		const std::vector<VkBuffer>& vertexBuffers = mRenderableMesh.getVertexBuffers();
		const std::vector<VkDeviceSize>& vertexBufferOffsets = mRenderableMesh.getVertexBufferOffsets();

		vkCmdBindVertexBuffers(commandBuffer, 0, vertexBuffers.size(), vertexBuffers.data(), vertexBufferOffsets.data());
		for (int index = 0; index < mesh_instance.getNumShapes(); ++index)
		{
			const IndexBuffer& index_buffer = mesh.getIndexBuffer(index);
			vkCmdBindIndexBuffer(commandBuffer, index_buffer.getBuffer(), 0, VK_INDEX_TYPE_UINT32);
			vkCmdDrawIndexed(commandBuffer, index_buffer.getCount(), 1, 0, 0, 0);
		}
	}


	glm::vec2 RenderToWindowComponentInstance::getRatio() const
	{
		switch (mMode)
		{
			case RenderToWindowComponent::EScaleMode::Window:
			{
				return mWindow->getSize();
			}
			case RenderToWindowComponent::EScaleMode::Square:
			{
				static const glm::vec2 sr = { 1.0f, 1.0f };
				return sr;
			}
			case RenderToWindowComponent::EScaleMode::Custom:
			{
				return mRatio;
			}
			default:
			{
				assert(false);
				return mWindow->getSize();
			}
		}
	}


	void RenderToWindowComponentInstance::setRatio(const glm::vec2& ratio)
	{
		mRatio = ratio;
		mMode = RenderToWindowComponent::EScaleMode::Custom;
	}
}
