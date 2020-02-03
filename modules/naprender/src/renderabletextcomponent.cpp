// Local Includes
#include "renderabletextcomponent.h"
#include "renderglobals.h"

// External Includes
#include <entity.h>
#include <transformcomponent.h>
#include <nap/core.h>
#include <renderservice.h>
#include <nap/logger.h>
#include "nindexbuffer.h"

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
		if (!mMaterialInstance.init(*getEntityInstance()->getCore()->getService<RenderService>(), resource->mMaterialInstanceResource, errorState))
			return false;
		
		// Ensure the uniform to set the glyph is available on the source material
// 		nap::Uniform* glyph_uniform = mMaterialInstance.getMaterial()->findUniform(mGlyphUniform);
// 		if (!errorState.check(glyph_uniform != nullptr, 
// 			"%s: Unable to bind font character, can't find 2Dtexture uniform in shader: %s with name: %s", this->mID.c_str(), 
// 			mMaterialInstance.getMaterial()->mID.c_str(), mGlyphUniform.c_str() ))
// 			return false;

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

		// Construct render-able mesh (TODO: Make a factory or something similar to create and verify render-able meshes!
		nap::RenderService* render_service = getEntityInstance()->getCore()->getService<nap::RenderService>();
		mRenderableMesh = render_service->createRenderableMesh(mPlane, mMaterialInstance, errorState);
		if (!mRenderableMesh.isValid())
			return false;

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
			// Fetch glyph.
			RenderableGlyph* glyph = getRenderableGlyph(mFont->getGlyphIndex(letter), error);
			if (!error.check(glyph != nullptr, "%s: unsupported character: %d, %s", mID.c_str(), letter, error.toString().c_str()))
			{
				success = false;
				continue;
			}
			// Store handle
			mGlyphs.emplace_back(glyph);
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

		// Get the parent material and set uniform values if present
// 		Material* comp_mat = mMaterialInstance.getMaterial();
// 		UniformMat4* projectionUniform = comp_mat->findUniform<UniformMat4>(projectionMatrixUniform);
// 		if (projectionUniform != nullptr)
// 			projectionUniform->setValue(projectionMatrix);
// 
// 		UniformMat4* viewUniform = comp_mat->findUniform<UniformMat4>(viewMatrixUniform);
// 		if (viewUniform != nullptr)
// 			viewUniform->setValue(viewMatrix);
// 
// 		UniformMat4* modelUniform = comp_mat->findUniform<UniformMat4>(modelMatrixUniform);
// 		if (modelUniform != nullptr)
// 			modelUniform->setValue(modelMatrix);

		// Prepare blending
		//mMaterialInstance.update(0); // todo: frame_index

		// Fetch uniform for setting character
		//UniformSampler2D& glyph_uniform = mMaterialInstance.getOrCreateUniform<UniformSampler2D>(mGlyphUniform);

		// Get vertex position data (that we update in the loop
		std::vector<glm::vec3>& pos_data = mPositionAttr->getData();

		// Get plane to draw
		MeshInstance& mesh_instance = mRenderableMesh.getMesh().getMeshInstance();

		// GPU mesh representation of plane
		GPUMesh& gpu_mesh = mesh_instance.getGPUMesh();

		// Lines / Fill etc.
		//GLenum draw_mode = getGLMode(mesh_instance.getShape(0).getDrawMode());

		// Fetch index buffer (holding drawing order
		const IndexBuffer& index_buffer = gpu_mesh.getIndexBuffer(0);
		GLsizei num_indices = static_cast<GLsizei>(index_buffer.getCount());
		nap::utility::ErrorState error;

		// Get uniforms to push in loop
// 		const nap::UniformSampler2D& glyph_binding = mMaterialInstance.getUniform<UniformSampler2D>(glyph_uniform.mName);
// 		int texture_unit = mMaterialInstance.getTextureUnit(glyph_uniform);
// 		assert(texture_unit > -1);

		// Push all uniforms now
		//mMaterialInstance.update(0);		// TODO: correct frame index

		// Location of active letter
		float x = 0.0f;
		float y = 0.0f;

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
			mesh_instance.update(*mPositionAttr, error);

			// Set texture and push uniforms
			//glyph_uniform.setTexture(render_glyph->getTexture());
			//glyph_uniform.push(nullptr, glyph_binding.getDeclaration(), texture_unit); // TODO: UBO

			// Bind and draw all the arrays
// 			index_buffer.bind();
// 			glDrawElements(draw_mode, num_indices, index_buffer.getType(), 0);
// 			index_buffer.unbind();

			// Update x
			x += render_glyph->getHorizontalAdvance();
		}
	}


	const math::Rect& RenderableTextComponentInstance::getBoundingBox() const
	{ 
		return mTextBounds;
	}
}
