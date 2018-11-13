// Local Includes
#include "renderabletextcomponent.h"
#include "materialutils.h"

// External Includes
#include <entity.h>
#include <transformcomponent.h>
#include <nap/core.h>
#include <renderservice.h>
#include <nap/logger.h>
#include <ndrawutils.h>

// nap::renderabletextcomponent run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::RenderableTextComponent)
	RTTI_PROPERTY("Text",				&nap::RenderableTextComponent::mText,						nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("Font",				&nap::RenderableTextComponent::mFont,						nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("GlyphUniform",		&nap::RenderableTextComponent::mGlyphUniform,				nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("MaterialInstance",	&nap::RenderableTextComponent::mMaterialInstanceResource,	nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

// nap::renderabletextcomponentInstance run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::RenderableTextComponentInstance)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	bool RenderableTextComponentInstance::init(utility::ErrorState& errorState)
	{
		// Get resource
		RenderableTextComponent* resource = getComponent<RenderableTextComponent>();
		
		// Extract font
		mFont = &(resource->mFont->getFontInstance());

		// Extract glyph uniform (texture slot in shader)
		mGlyphUniform = resource->mGlyphUniform;

		// Fetch transform
		mTransform = getEntityInstance()->findComponent<TransformComponentInstance>();

		// Create material instance
		if (!mMaterialInstance.init(resource->mMaterialInstanceResource, errorState))
			return false;
		
		// Ensure the uniform to set the glyph is available on the source material
		nap::Uniform* glyph_uniform = mMaterialInstance.getMaterial()->findUniform(mGlyphUniform);
		if (!errorState.check(glyph_uniform != nullptr, 
			"%s: Unable to bind font character, can't find texture uniform in shader: %s with name: %s", this->mID.c_str(), 
			mMaterialInstance.getMaterial()->mID.c_str(), mGlyphUniform.c_str() ))
			return false;

		// Setup the plane
		mPlane.mRows	= 1;
		mPlane.mColumns = 1;
		if (!mPlane.setup(errorState))
			return false;

		// Make sure we can write to it often 
		mPlane.getMeshInstance().setUsage(EMeshDataUsage::DynamicWrite);

		// Update the uv coordinates
		Vec3VertexAttribute* uv_attr = mPlane.getMeshInstance().findAttribute<glm::vec3>(VertexAttributeIDs::getUVName(0));
		if (!errorState.check(uv_attr != nullptr, "%s: unable to find uv vertex attribute on plane", mID.c_str()))
			return false;

		// Flip uv y axis (text is rendered flipped)
		uv_attr->getData()[0].y = 1.0f;
		uv_attr->getData()[1].y = 1.0f;
		uv_attr->getData()[2].y = 0.0f;
		uv_attr->getData()[3].y = 0.0f;

		// Initialize it on the GPU
		if (!mPlane.getMeshInstance().init(errorState))
			return false;

		// Get position attribute buffer, we will update the vertex positions of this plane
		mPositionAttr = mPlane.getMeshInstance().findAttribute<glm::vec3>(VertexAttributeIDs::getPositionName());
		if (!errorState.check(mPositionAttr != nullptr, "%s: unable to get plane vertex attribute handle", mID.c_str()))
			return false;

		// Create a render-able mesh based on our material and plane
		// Note that this mesh represents a valid binding between mesh and material, when this succeeds the characters can be rendered
		nap::RenderService* render_service = getEntityInstance()->getCore()->getService<nap::RenderService>();
		VAOHandle handle = render_service->acquireVertexArrayObject(*(mMaterialInstance.getMaterial()), mPlane, errorState);

		if (!errorState.check(handle.isValid(), "Failed to acquire VAO for RenderableTextComponent %s", getComponent()->mID.c_str()))
			return false;

		// Construct render-able mesh (TODO: Make a factory or something similar to create and verify render-able meshes!
		mRenderableMesh = RenderableMesh(mPlane, mMaterialInstance, handle);

		// Set text, needs to succeed on initialization
		if (!setText(resource->mText, errorState))
			return false;

		return true;
	}


	const nap::FontInstance& RenderableTextComponentInstance::getFont() const
	{
		assert(mFont != nullptr);
		return *mFont;
	}


	bool RenderableTextComponentInstance::setText(const std::string& text, utility::ErrorState& error)
	{
		// Clear Glyph handles
		mGlyphs.clear();
		mGlyphs.reserve(text.size());
		
		// Get or create a Glyph for every letter in the text
		bool success(true);
		for (const auto& letter : text)
		{
			// Fetch glyph
			uint gindex = mFont->getGlyphIndex(letter);
			RenderableGlyph* render_glyph = mFont->getOrCreateGlyphRepresentation<RenderableGlyph>(gindex, error);
			if (!error.check(render_glyph != nullptr, "%s: invalid character: %d, %s", mID.c_str(), letter, error.toString().c_str()))
			{
				success = false;
				continue;
			}

			// Store handle
			mGlyphs.emplace_back(render_glyph);
		}
		mText = text;
		mFont->getBoundingBox(mText, mTextBounds);
		return success;
	}


	nap::MaterialInstance& RenderableTextComponentInstance::getMaterialInstance()
	{
		return mMaterialInstance;
	}


	void RenderableTextComponentInstance::draw(const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix, const glm::mat4& modelMatrix)
	{
		// Ensure we can render the mesh / material combo
		if (!mRenderableMesh.isValid())
		{
			assert(false);
			return;
		}

		// Get material to bind
		Material* comp_mat = mMaterialInstance.getMaterial();
		comp_mat->bind();

		// Set uniform variables
		UniformMat4* projectionUniform = comp_mat->findUniform<UniformMat4>(projectionMatrixUniform);
		if (projectionUniform != nullptr)
			projectionUniform->setValue(projectionMatrix);

		UniformMat4* viewUniform = comp_mat->findUniform<UniformMat4>(viewMatrixUniform);
		if (viewUniform != nullptr)
			viewUniform->setValue(viewMatrix);

		UniformMat4* modelUniform = comp_mat->findUniform<UniformMat4>(modelMatrixUniform);
		if (modelUniform != nullptr)
			modelUniform->setValue(modelMatrix);

		// Prepare blending
		utility::setBlendMode(getMaterialInstance());

		// Bind vertex array object
		// The VAO handle works for all the registered render contexts
		mRenderableMesh.mVAOHandle.get().bind();

		// Get plane to draw
		MeshInstance& mesh_instance = mRenderableMesh.getMesh().getMeshInstance();

		// Location of active letter
		float x = 0.0f;
		float y = 0.0f;

		// Fetch uniform for setting character
		UniformTexture2D* glyph_uniform = comp_mat->findUniform<UniformTexture2D>(mGlyphUniform);
		assert(glyph_uniform != nullptr);

		// Get vertex position data (that we update in the loop
		std::vector<glm::vec3>& pos_data = mPositionAttr->getData();

		// GPU mesh representation
		const opengl::GPUMesh& gpu_mesh = mesh_instance.getGPUMesh();

		// Lines / Fill etc.
		GLenum draw_mode = getGLMode(mesh_instance.getShape(0).getDrawMode());

		// Fetch index buffer (holding drawing order
		const opengl::IndexBuffer& index_buffer = gpu_mesh.getIndexBuffer(0);
		GLsizei num_indices = static_cast<GLsizei>(index_buffer.getCount());
		nap::utility::ErrorState error;

		// Draw every letter in the text to screen
		for (auto& render_glyph : mGlyphs)
		{
			// Get width and height of character to draw
			float w = render_glyph->getSize().x;
			float h = render_glyph->getSize().y;

			// Compute x and y position
			float xpos = x + render_glyph->getOffsetLeft();
			float ypos = y - (h - render_glyph->getOffsetTop());

			// Set vertex positions of plane
			pos_data[0] = { xpos,		ypos,		0.0f };
			pos_data[1] = { xpos + w,	ypos,		0.0f };
			pos_data[2] = { xpos,		ypos + h,	0.0f };
			pos_data[3] = { xpos + w,	ypos + h,	0.0f };

			// Push vertex positions to GPU
			mesh_instance.update(error);

			// Set texture and push uniforms
			glyph_uniform->setTexture(render_glyph->getTexture());
			utility::pushUniforms(mMaterialInstance);

			// Bind and draw all the arrays
			index_buffer.bind();
			glDrawElements(draw_mode, num_indices, index_buffer.getType(), 0);
			index_buffer.unbind();

			// Update x
			x += render_glyph->getHorizontalAdvance();
		}

		// Unbind
		index_buffer.unbind();
		comp_mat->unbind();
		mRenderableMesh.mVAOHandle.get().unbind();
	}


	const math::Rect& RenderableTextComponentInstance::getBoundingBox() const
	{ 
		return mTextBounds;
	}
}