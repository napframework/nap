// Local Includes
#include "rendertotexturecomponent.h"
#include "rendertarget.h"
#include "renderservice.h"
#include "indexbuffer.h"
#include "renderglobals.h"
#include "uniforminstance.h"
#include "renderglobals.h"

// External Includes
#include <entity.h>
#include <glm/gtc/matrix_transform.hpp>
#include <entity.h>
#include <nap/core.h>
#include <orthocameracomponent.h>

// nap::rendertotexturecomponent run time class definition 
RTTI_BEGIN_CLASS(nap::RenderToTextureComponent)
	RTTI_PROPERTY("OutputTexture",				&nap::RenderToTextureComponent::mOutputTexture,				nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("MaterialInstance",			&nap::RenderToTextureComponent::mMaterialInstanceResource,	nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("ClearColor",					&nap::RenderToTextureComponent::mClearColor,				nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("ProjectionMatrixUniform",	&nap::RenderToTextureComponent::mProjectMatrixUniform,		nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("ModelMatrixUniform",			&nap::RenderToTextureComponent::mModelMatrixUniform,		nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

// nap::rendertotexturecomponentInstance run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::RenderToTextureComponentInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	// Some statics used by this component
	static const glm::mat4x4 sIdentityMatrix = glm::mat4x4();


	void RenderToTextureComponent::getDependentComponents(std::vector<rtti::TypeInfo>& components) const
	{

	}


	RenderToTextureComponentInstance::RenderToTextureComponentInstance(EntityInstance& entity, Component& resource) :
		RenderableComponentInstance(entity, resource),
		mPlane(*entity.getCore()),
		mTarget(*entity.getCore())
	{
	}


	RenderToTextureComponentInstance::~RenderToTextureComponentInstance()
	{

	}


	bool RenderToTextureComponentInstance::init(utility::ErrorState& errorState)
	{
		// Get resource
		RenderToTextureComponent* resource = getComponent<RenderToTextureComponent>();

		// Create the render target
		mTarget.mClearColor = glm::vec4(resource->mClearColor.convert<RGBColorFloat>().toVec3(), 1.0f);

		// Bind textures to target
		mTarget.mColorTexture = resource->mOutputTexture;

		// Initialize target
		if (!mTarget.init(errorState))
			return false;

		// Now create a plane and initialize it
		// The plane is positioned on update based on current texture output size
		mPlane.mSize = glm::vec2(1.0f, 1.0f);
		mPlane.mPosition = glm::vec3(0.0f, 0.0f, 0.0f);
		if (!mPlane.init(errorState))
			return false;

		RenderService* render_service = getEntityInstance()->getCore()->getService<RenderService>();

		// Create material instance
		if (!mMaterialInstance.init(*render_service, resource->mMaterialInstanceResource, errorState))
			return false;

		// Ensure the matrices are present on the material
		mProjectMatrixUniform = resource->mProjectMatrixUniform;
		if (!ensureUniform(mProjectMatrixUniform, errorState))
			return false;

		mModelMatrixUniform = resource->mModelMatrixUniform;
		if (!ensureUniform(mModelMatrixUniform, errorState))
			return false;

		// Create the renderable mesh, which represents a valid mesh / material combination
		mService = getEntityInstance()->getCore()->getService<RenderService>();
		assert(mService != nullptr);
		mRenderableMesh = mService->createRenderableMesh(mPlane, mMaterialInstance, errorState);
		if (!mRenderableMesh.isValid())
			return false;

		return true;
	}


	void RenderToTextureComponentInstance::update(double deltaTime)
	{

	}


	IRenderTarget& RenderToTextureComponentInstance::getTarget()
	{
		return mTarget;
	}


	nap::Texture2D& RenderToTextureComponentInstance::getOutputTexture()
	{
		return mTarget.getColorTexture();
	}

	void RenderToTextureComponentInstance::draw()
	{
		VkCommandBuffer command_buffer = mService->getCurrentCommandBuffer();

		// Create orthographic projection matrix
		glm::ivec2 size = mTarget.getBufferSize();

		// Create projection matrix
		glm::mat4 proj_matrix = OrthoCameraComponentInstance::createRenderProjectionMatrix(0.0f, (float)size.x, 0.0f, (float)size.y);

		mTarget.beginRendering();

		// Call on draw
		onDraw(mTarget, command_buffer, sIdentityMatrix, proj_matrix);

		mTarget.endRendering();
	}


	bool RenderToTextureComponentInstance::isSupported(nap::CameraComponentInstance& camera) const
	{
		return camera.get_type().is_derived_from(RTTI_OF(OrthoCameraComponentInstance));
	}


	nap::MaterialInstance& RenderToTextureComponentInstance::getMaterialInstance()
	{
		return mMaterialInstance;
	}


	void RenderToTextureComponentInstance::onDraw(IRenderTarget& renderTarget, VkCommandBuffer commandBuffer, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix)
	{
		// Ensure we can render the mesh / material combo
		if (!mRenderableMesh.isValid())
		{
			assert(false);
			return;
		}

		// Get the parent material
		Material& comp_mat = mMaterialInstance.getMaterial();		 

		// Update the model matrix so that the plane mesh is of the same size as the render target
		computeModelMatrix();

		UniformStructInstance* nap_uniform = mMaterialInstance.getOrCreateUniform(uniform::mvpStruct);
		if (nap_uniform != nullptr)
		{
			// Set projection uniform in shader
			UniformMat4Instance* projection_uniform = nap_uniform->getOrCreateUniform<UniformMat4Instance>(mProjectMatrixUniform);
			if (projection_uniform != nullptr)
				projection_uniform->setValue(projectionMatrix);

			// Set model matrix uniform in shader
			UniformMat4Instance* model_uniform = nap_uniform->getOrCreateUniform<UniformMat4Instance>(mModelMatrixUniform);
			if (model_uniform != nullptr)
				model_uniform->setValue(mModelMatrix);
		}

		VkDescriptorSet descriptor_set = mMaterialInstance.update();

		// Gather draw info
		MeshInstance& mesh_instance = mRenderableMesh.getMesh().getMeshInstance();
		GPUMesh& mesh = mesh_instance.getGPUMesh();

		utility::ErrorState error_state;
		RenderService::Pipeline pipeline = mService->getOrCreatePipeline(renderTarget, mRenderableMesh.getMesh(), mMaterialInstance, error_state);
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.mPipeline);

		Material& material = mRenderableMesh.getMaterialInstance().getMaterial();

		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.mLayout, 0, 1, &descriptor_set, 0, nullptr);

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


	bool RenderToTextureComponentInstance::ensureUniform(const std::string& uniformName, utility::ErrorState& error)
	{
		// Same applies for the matrices
		UniformInstance* found_uniform = nullptr;
		UniformStructInstance* nap_uniform = mMaterialInstance.getMaterial().findUniform("nap");
		if (nap_uniform != nullptr)
			found_uniform = nap_uniform->findUniform(uniformName);

		if (!error.check(found_uniform != nullptr,
			"%s: unable to find uniform: %s in material: %s", this->mID.c_str(), uniformName.c_str(),
			mMaterialInstance.getMaterial().mID.c_str()))
			return false;
		return true;
	}


	void RenderToTextureComponentInstance::computeModelMatrix()
	{
		if (mDirty)
		{
			// Transform to middle of target
			glm::ivec2 tex_size = mTarget.getBufferSize();
			mModelMatrix = glm::translate(sIdentityMatrix, glm::vec3(
				tex_size.x / 2.0f,
				tex_size.y / 2.0f,
				0.0f));

			// Scale to fit target
			mModelMatrix = glm::scale(mModelMatrix, glm::vec3(tex_size.x, tex_size.y, 1.0f));
			mDirty = false;
		}
	}
}