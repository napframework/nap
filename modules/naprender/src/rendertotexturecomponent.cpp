#include "rendertotexturecomponent.h"

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


	RenderToTextureComponentInstance::~RenderToTextureComponentInstance()
	{

	}


	bool RenderToTextureComponentInstance::init(utility::ErrorState& errorState)
	{
		// Get resource
		RenderToTextureComponent* resource = getComponent<RenderToTextureComponent>();

		// Create settings for depth 2D texture
		mDepthTexture.mFormat = RenderTexture2D::EFormat::Depth;
		mDepthTexture.mWidth  = resource->mOutputTexture->getWidth();
		mDepthTexture.mHeight = resource->mOutputTexture->getHeight();

		// Initialize internally managed depth texture
		mDepthTexture.mParameters.mMaxFilter = EFilterMode::Linear;
		mDepthTexture.mParameters.mMinFilter = EFilterMode::Linear;
		mDepthTexture.mParameters.mMaxLodLevel = 1;
		if (!mDepthTexture.init(errorState))
			return false;

		// Create the render target
		mTarget.mClearColor = glm::vec4(resource->mClearColor.convert<RGBColorFloat>().toVec3(), 1.0f);

		// Bind textures to target
		mTarget.mColorTexture = resource->mOutputTexture;
		mTarget.mDepthTexture = &mDepthTexture;

		// Initialize target
		if (!mTarget.init(errorState))
			return false;

		// Now create a plane and initialize it
		// The plane is positioned on update based on current texture output size
		mPlane.mSize = glm::vec2(1.0f, 1.0f);
		mPlane.mPosition = glm::vec3(0.0f, 0.0f, 0.0f);
		if (!mPlane.init(errorState))
			return false;

		// Create material instance
		if (!mMaterialInstance.init(resource->mMaterialInstanceResource, errorState))
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


	opengl::RenderTarget& RenderToTextureComponentInstance::getTarget()
	{
		return mTarget.getTarget();
	}


	nap::Texture2D& RenderToTextureComponentInstance::getOutputTexture()
	{
		return mTarget.getColorTexture();
	}


	bool RenderToTextureComponentInstance::switchOutputTexture(nap::Texture2D& texture, utility::ErrorState& error)
	{
		mDirty = true;
		if (mTarget.switchColorTexture(texture, error))
			return true;
		return false;
	}


	void RenderToTextureComponentInstance::draw()
	{
		// Create orthographic projection matrix
		glm::ivec2 size = mTarget.getTarget().getSize();
		glm::mat4 proj_matrix = glm::ortho(0.0f, (float)size.x, 0.0f, (float)size.y);

		// Clear target
		mService->clearRenderTarget(mTarget.getTarget());

		// Bind render target
		mTarget.getTarget().bind();

		// Ensure correct render state
		mService->pushRenderState();

		// Call on draw
		onDraw(sIdentityMatrix, proj_matrix);

		// Unbind render target
		mTarget.getTarget().unbind();
	}


	bool RenderToTextureComponentInstance::isSupported(nap::CameraComponentInstance& camera) const
	{
		return camera.get_type().is_derived_from(RTTI_OF(OrthoCameraComponentInstance));
	}


	nap::MaterialInstance& RenderToTextureComponentInstance::getMaterialInstance()
	{
		return mMaterialInstance;
	}


	void RenderToTextureComponentInstance::onDraw(const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix)
	{
		// Ensure we can render the mesh / material combo
		if (!mRenderableMesh.isValid())
		{
			assert(false);
			return;
		}

		// Bind material
		mMaterialInstance.bind();

		// Get the parent material
		Material* comp_mat = mMaterialInstance.getMaterial();

		// Push matrices
		UniformMat4& projectionUniform = mMaterialInstance.getOrCreateUniform<UniformMat4>(mProjectMatrixUniform);
		projectionUniform.setValue(projectionMatrix);

		computeModelMatrix();
		UniformMat4& modelUniform = mMaterialInstance.getOrCreateUniform<UniformMat4>(mModelMatrixUniform);
		modelUniform.setValue(mModelMatrix);

		// Prepare blending
		mMaterialInstance.pushBlendMode();

		// Push all uniforms now
		mMaterialInstance.pushUniforms();

		// Bind vertex array object
		// The VAO handle works for all the registered render contexts
		mRenderableMesh.bind();

		// Gather draw info
		MeshInstance& mesh_instance = mRenderableMesh.getMesh().getMeshInstance();
		const opengl::GPUMesh& mesh = mesh_instance.getGPUMesh();

		// Draw all shapes associated with the mesh
		for (int index = 0; index < mesh_instance.getNumShapes(); ++index)
		{
			MeshShape& shape = mesh_instance.getShape(index);
			const opengl::IndexBuffer& index_buffer = mesh.getIndexBuffer(index);

			GLenum draw_mode = getGLMode(shape.getDrawMode());
			GLsizei num_indices = static_cast<GLsizei>(index_buffer.getCount());

			index_buffer.bind();
			glDrawElements(draw_mode, num_indices, index_buffer.getType(), 0);
			index_buffer.unbind();
		}

		// Unbind all GL resources
		mRenderableMesh.unbind();
		mMaterialInstance.unbind();
	}


	bool RenderToTextureComponentInstance::ensureUniform(const std::string& uniformName, utility::ErrorState& error)
	{
		// Same applies for the matrices
		if (!error.check(mMaterialInstance.getMaterial()->findUniform(uniformName) != nullptr,
			"%s: unable to find uniform: %s in material: %s", this->mID.c_str(), uniformName.c_str(),
			mMaterialInstance.getMaterial()->mID.c_str()))
			return false;
		return true;
	}


	void RenderToTextureComponentInstance::computeModelMatrix()
	{
		if (mDirty)
		{
			// Transform to middle of target
			glm::ivec2 tex_size = mTarget.getColorTexture().getSize();
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