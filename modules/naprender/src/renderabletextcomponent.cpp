// Local Includes
#include "renderabletextcomponent.h"
#include "renderglobals.h"
#include "material.h"
#include "indexbuffer.h"

// External Includes
#include <entity.h>
#include <transformcomponent.h>
#include <nap/core.h>
#include <renderservice.h>
#include <nap/logger.h>
#include <glm/gtc/matrix_transform.hpp>
#include <nap/assert.h>

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
	RenderableTextComponentInstance::RenderableTextComponentInstance(EntityInstance& entity, Component& resource) :
		RenderableComponentInstance(entity, resource),
		mPlane(*entity.getCore()),
		mRenderService(entity.getCore()->getService<RenderService>())
	{
	}

	bool RenderableTextComponentInstance::init(utility::ErrorState& errorState)
	{
		// Get resource
		RenderableTextComponent* resource = getComponent<RenderableTextComponent>();
		
		// Extract font
		mFont = &(resource->mFont->getFontInstance());

		// Extract glyph uniform (texture slot in shader)
		mGlyphUniformName = resource->mGlyphUniform;

		// Fetch transform
		mTransform = getEntityInstance()->findComponent<TransformComponentInstance>();

		// Create material instance
		if (!mMaterialInstance.init(*getEntityInstance()->getCore()->getService<RenderService>(), resource->mMaterialInstanceResource, errorState))
			return false;

		// Ensure the uniform to set the glyph is available on the source material
		mGlyphUniform = mMaterialInstance.getOrCreateSampler<Sampler2DInstance>(mGlyphUniformName);
		if (!errorState.check(mGlyphUniform != nullptr,
		 	"%s: Unable to bind font character, can't find 2DSampler uniform: %s in material: %s", this->mID.c_str(), 
			mGlyphUniformName.c_str() , mMaterialInstance.getMaterial().mID.c_str()))
		 	return false;

		// Find MVP uniforms
		UniformStructInstance* mvp_uniform = mMaterialInstance.getOrCreateUniform(mvpStructUniform);
		if (mvp_uniform != nullptr)
		{
			mModelUniform		= mvp_uniform->getOrCreateUniform<UniformMat4Instance>(modelMatrixUniform);
			mViewUniform		= mvp_uniform->getOrCreateUniform<UniformMat4Instance>(viewMatrixUniform);
			mProjectionUniform	= mvp_uniform->getOrCreateUniform<UniformMat4Instance>(projectionMatrixUniform);
		}

		// Make sure there's a model matrix
		if (!errorState.check(mModelUniform != nullptr, "%s: Unable to position character, no model matrix with name: %s found in UBO: %s in material %s",
			mID.c_str(), modelMatrixUniform, 
			mvpStructUniform, mMaterialInstance.getMaterial().mID.c_str()))
			return false;

		// Setup the plane, 1x1 with lower left corner at origin {0, 0}
		mPlane.mRows	= 1;
		mPlane.mColumns = 1;
		mPlane.mPosition = { 0.5f, 0.5f };
		mPlane.mSize = { 1.0f, 1.0f };
		mPlane.mUsage = EMeshDataUsage::Static;
		mPlane.mCullMode = ECullMode::Back;
		if (!mPlane.setup(errorState))
			return false;

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

		// Construct render-able mesh
		mRenderableMesh = mRenderService->createRenderableMesh(mPlane, mMaterialInstance, errorState);
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
		// Clear and add text
		clearText();
		return addText(text, error);
	}


	bool RenderableTextComponentInstance::addText(const std::string& text, utility::ErrorState& error)
	{
		// Adding / changing text is not allowed during frame capture
		// This is because new characters might be uploaded
		NAP_ASSERT_MSG(!mRenderService->isRenderingFrame(), "Can't change or add text when rendering a frame");

		// Create new glyph set
		std::vector<RenderableGlyph*> new_glyphs;
		new_glyphs.reserve(text.size());

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
			new_glyphs.emplace_back(glyph);
		}

		// Get bounding box and add entries into cache
		math::Rect new_bounds;
		mTextCache.emplace_back(text);
		mFont->getBoundingBox(mTextCache.back(), new_bounds);
		mGlyphCache.emplace_back(std::move(new_glyphs));
		mTextBounds.emplace_back(std::move(new_bounds));

		return success;
	}


	void RenderableTextComponentInstance::setLineIndex(int index)
	{
		assert(index < mGlyphCache.size());
		mCacheIndex = index;
	}


	void RenderableTextComponentInstance::clearText()
	{
		mGlyphCache.clear();
		mTextBounds.clear();
		mCacheIndex = 0;
	}


	void RenderableTextComponentInstance::draw(IRenderTarget& renderTarget, VkCommandBuffer commandBuffer, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix, const glm::mat4& modelMatrix)
	{
		// Ensure we can render the mesh / material combo
		if (!mRenderableMesh.isValid())
		{
			assert(false);
			return;
		}

		// If index is invalid, return
		if (mCacheIndex >= mGlyphCache.size())
			return;

		// Update view uniform
		if (mViewUniform != nullptr)
			mViewUniform->setValue(viewMatrix);

		// Update projection uniform
		if (mProjectionUniform != nullptr)
			mProjectionUniform->setValue(projectionMatrix);

		// Get plane to draw
		MeshInstance& mesh_instance = mRenderableMesh.getMesh().getMeshInstance();

		// Fetch index buffer (holding drawing order
		const IndexBuffer& index_buffer = mesh_instance.getGPUMesh().getIndexBuffer(0);

		utility::ErrorState error_state;

		// Location of active letter
		float x = 0.0f;
		float y = 0.0f;

		// Get pipeline
		RenderService::Pipeline pipeline = mRenderService->getOrCreatePipeline(renderTarget, mRenderableMesh.getMesh(), mMaterialInstance, error_state);
		
		// Scissor rectangle
		VkRect2D scissor_rect {
			{0, 0},
			{(uint32_t)(renderTarget.getBufferSize().x), (uint32_t)(renderTarget.getBufferSize().y) }
		};

		// Viewport
		VkViewport viewport = 
		{
			0.0f, 0.0f,
			(float)(renderTarget.getBufferSize().x), 
			(float)(renderTarget.getBufferSize().y),
			0.0f, 1.0f
		};

		// Draw individual glyphs
		for (auto& render_glyph : mGlyphCache[mCacheIndex])
		{
			// Don't draw empty glyphs (spaces)
			if (render_glyph->empty())
			{
				x += render_glyph->getHorizontalAdvance();
				continue;
			}

			// Get width and height of character to draw
			float w = render_glyph->getSize().x;
			float h = render_glyph->getSize().y;

			// Compute x and y position
			float xpos = x + render_glyph->getOffsetLeft();
			float ypos = y - (h - render_glyph->getOffsetTop());;
			
			// Compute local model matrix
			glm::mat4 plane_loc = glm::translate(glm::mat4(), glm::vec3(xpos, ypos, 0.0f));
			plane_loc = glm::scale(plane_loc, glm::vec3(w, h, 1.0f));

			// Update model matrix and glyph
			mModelUniform->setValue(modelMatrix * plane_loc);
			mGlyphUniform->setTexture(render_glyph->getTexture());

			// Get new descriptor set that contains the updated settings and bind pipeline
			VkDescriptorSet descriptor_set = mMaterialInstance.update();
			vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.mPipeline);
			vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

			// Bind descriptor set
			vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.mLayout, 0, 1, &descriptor_set, 0, nullptr);

			// Bind vertex buffers
			const std::vector<VkBuffer>& vertexBuffers = mRenderableMesh.getVertexBuffers();
			const std::vector<VkDeviceSize>& vertexBufferOffsets = mRenderableMesh.getVertexBufferOffsets();
			vkCmdBindVertexBuffers(commandBuffer, 0, vertexBuffers.size(), vertexBuffers.data(), vertexBufferOffsets.data());
			vkCmdSetScissor(commandBuffer, 0, 1, &scissor_rect);

			// Draw geometry
			vkCmdBindIndexBuffer(commandBuffer, index_buffer.getBuffer(), 0, VK_INDEX_TYPE_UINT32);
			vkCmdDrawIndexed(commandBuffer, index_buffer.getCount(), 1, 0, 0, 0);

			// Update x
			x += render_glyph->getHorizontalAdvance();
		}
	}


	const math::Rect& RenderableTextComponentInstance::getBoundingBox() const
	{
		const static math::Rect empty;
		return mCacheIndex < mTextBounds.size() ? mTextBounds[mCacheIndex] : empty;
	}


	const std::string& RenderableTextComponentInstance::getText()
	{
		const static std::string empty;
		return mCacheIndex < mTextCache.size() ? mTextCache[mCacheIndex] : empty;
	}
}
