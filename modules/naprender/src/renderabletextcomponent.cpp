/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "renderabletextcomponent.h"
#include "renderglobals.h"
#include "material.h"
#include "indexbuffer.h"
#include "fontshader.h"

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
	RTTI_PROPERTY("TextColor",			&nap::RenderableTextComponent::mColor,						nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

// nap::renderabletextcomponentInstance run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::RenderableTextComponentInstance)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	RenderableTextComponentInstance::RenderableTextComponentInstance(EntityInstance& entity, Component& resource) :
		RenderableComponentInstance(entity, resource),
		mRenderService(entity.getCore()->getService<RenderService>()),
        mPlane(*entity.getCore())
	{ }


	bool RenderableTextComponentInstance::init(utility::ErrorState& errorState)
	{
		if (!RenderableComponentInstance::init(errorState))
			return false;
		return true;
	}


	bool RenderableTextComponentInstance::setup(float scale, utility::ErrorState& errorState)
	{
		// Get resource and extract font and transform
		RenderableTextComponent* resource = getComponent<RenderableTextComponent>();
		mFont = &(resource->mFont->getFontInstance());
		mTransform = getEntityInstance()->findComponent<TransformComponentInstance>();

		// Get hard-coded font material
		Material* font_material = mRenderService->getOrCreateMaterial<FontShader>(errorState);
		if (!errorState.check(font_material!= nullptr, "%s: unable to get or create video material", resource->mID.c_str()))
			return false;

		// Create resource for the font material instance
		mMaterialInstanceResource.mBlendMode = EBlendMode::AlphaBlend;
		mMaterialInstanceResource.mDepthMode = EDepthMode::NoReadWrite;
		mMaterialInstanceResource.mMaterial = font_material;

		// Initialize font material instance
		if (!mMaterialInstance.init(*mRenderService, mMaterialInstanceResource, errorState))
			return false;

		// Ensure we can find the text ubo
		UniformStructInstance* ubo_struct = mMaterialInstance.getOrCreateUniform(uniform::font::uboStruct);
		if (!errorState.check(ubo_struct != nullptr, "%s: Unable to set color, unable to find uniform struct with name: %s in material: %s",
			resource->mID.c_str(), uniform::font::uboStruct, mMaterialInstance.getMaterial().mID.c_str()))
			return false;

		// Ensure the color color uniform is available and set it
		mColorUniform = ubo_struct->getOrCreateUniform<UniformVec3Instance>(uniform::font::textColor);
		if (!errorState.check(mColorUniform != nullptr, "%s: Unable to find uniform vec3 with name: %s in material: %s",
			resource->mID.c_str(), uniform::font::textColor, mMaterialInstance.getMaterial().mID.c_str()))
			return false;
		mColorUniform->setValue(resource->mColor.toVec3());

		// Ensure the uniform to set the glyph is available on the source material
		mGlyphUniform = mMaterialInstance.getOrCreateSampler<Sampler2DInstance>(uniform::font::glyphSampler);
		if (!errorState.check(mGlyphUniform != nullptr,
		 	"%s: Unable to bind font character, can't find 2DSampler uniform: %s in material: %s", resource->mID.c_str(), 
			uniform::font::glyphSampler, mMaterialInstance.getMaterial().mID.c_str()))
		 	return false;

		// Find MVP uniforms
		UniformStructInstance* mvp_uniform = mMaterialInstance.getOrCreateUniform(uniform::mvpStruct);
		if (mvp_uniform != nullptr)
		{
			mModelUniform		= mvp_uniform->getOrCreateUniform<UniformMat4Instance>(uniform::modelMatrix);
			mViewUniform		= mvp_uniform->getOrCreateUniform<UniformMat4Instance>(uniform::viewMatrix);
			mProjectionUniform	= mvp_uniform->getOrCreateUniform<UniformMat4Instance>(uniform::projectionMatrix);
		}

		// Make sure there's a model matrix
		if (!errorState.check(mModelUniform != nullptr, "%s: Unable to position character, no model matrix with name: %s found in UBO: %s in material %s",
			resource->mID.c_str(), uniform::modelMatrix, uniform::mvpStruct, mMaterialInstance.getMaterial().mID.c_str()))
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
		Vec3VertexAttribute* uv_attr = mPlane.getMeshInstance().findAttribute<glm::vec3>(vertexid::getUVName(0));
		if (!errorState.check(uv_attr != nullptr, "%s: unable to find uv vertex attribute on plane", resource->mID.c_str()))
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
		mPositionAttr = mPlane.getMeshInstance().findAttribute<glm::vec3>(vertexid::position);
		if (!errorState.check(mPositionAttr != nullptr, "%s: unable to get plane vertex attribute handle", mID.c_str()))
			return false;

		// Construct render-able mesh
		mRenderableMesh = mRenderService->createRenderableMesh(mPlane, mMaterialInstance, errorState);
		if (!mRenderableMesh.isValid())
			return false;

		// Set text, needs to succeed on initialization
		mFontScale = scale;
		if (!addLine(resource->mText, errorState))
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
		// Adding / changing text is not allowed during frame capture
		// This is because new characters might be uploaded
		NAP_ASSERT_MSG(!mRenderService->isRenderingFrame(), "Can't change or add text when rendering a frame");

		// Get cache to populate
		assert(mIndex < mGlyphCache.size());
		std::vector<RenderableGlyph*>& cur_cache = mGlyphCache[mIndex];
		cur_cache.clear();
		cur_cache.reserve(text.size());

		// Get or create a Glyph for every letter in the text
		bool success(true);
		for (const auto& letter : text)
		{
			// Fetch glyph.
			RenderableGlyph* glyph = getRenderableGlyph(mFont->getGlyphIndex(letter), mFontScale, error);
			if (!error.check(glyph != nullptr, "%s: unsupported character: %d, %s", mID.c_str(), letter, error.toString().c_str()))
			{
				success = false;
				continue;
			}
			// Store handle
			cur_cache.emplace_back(glyph);
		}

		// Set text and compute bounding box
		mLinesCache[mIndex]  = text;
		mFont->getBoundingBox(text, mFontScale, mTextBounds[mIndex]);
		return success;
	}


	bool RenderableTextComponentInstance::setText(int lineIndex, const std::string& text, utility::ErrorState& error)
	{
		setLineIndex(lineIndex);
		return setText(text, error);
	}


	void RenderableTextComponentInstance::setColor(const glm::vec3& color)
	{
		mColorUniform->setValue(color);
	}


	bool RenderableTextComponentInstance::addLine(const std::string& text, utility::ErrorState& error)
	{
		// Increase container size
		resize(mGlyphCache.size() + 1);

		// Set text, creating glyphs when required
		setLineIndex(mGlyphCache.size() - 1);
		return setText(text, error);
	}


	void RenderableTextComponentInstance::setLineIndex(int index)
	{
		assert(index < mGlyphCache.size());
		mIndex = index;
	}


	void RenderableTextComponentInstance::resize(int lines)
	{
		mGlyphCache.resize((size_t)lines);
		mTextBounds.resize((size_t)lines);
		mLinesCache.resize((size_t)lines);
	}


	int RenderableTextComponentInstance::getCount() const
	{
		return static_cast<int>(mGlyphCache.size());
	}


	void RenderableTextComponentInstance::clear()
	{
		mGlyphCache.clear();
		mTextBounds.clear();
		mLinesCache.clear();
		mIndex = 0;
	}


	void RenderableTextComponentInstance::draw(IRenderTarget& renderTarget, VkCommandBuffer commandBuffer, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix, const glm::mat4& modelMatrix)
	{
		// Ensure we can render the mesh / material combo
		if (!mRenderableMesh.isValid())
		{
			assert(false);
			return;
		}

		// If there is no cache, there's nothing to draw so bail.
		if (mGlyphCache.empty())
			return;
		assert(mIndex < mGlyphCache.size());

		// If the cache contains no characters, bail.
		std::vector<RenderableGlyph*> cur_cache = mGlyphCache[mIndex];
		if (cur_cache.empty())
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

		// Location of active letter
		float x = 0.0f;
		float y = 0.0f;

		// Get and bind pipeline
		utility::ErrorState error_state;
		RenderService::Pipeline pipeline = mRenderService->getOrCreatePipeline(renderTarget, mRenderableMesh.getMesh(), mMaterialInstance, error_state);
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.mPipeline);

		// Bind vertex and index buffers
		const std::vector<VkBuffer>& vertexBuffers = mRenderableMesh.getVertexBuffers();
		const std::vector<VkDeviceSize>& vertexBufferOffsets = mRenderableMesh.getVertexBufferOffsets();
		vkCmdBindVertexBuffers(commandBuffer, 0, vertexBuffers.size(), vertexBuffers.data(), vertexBufferOffsets.data());
		vkCmdBindIndexBuffer(commandBuffer, index_buffer.getBuffer(), 0, VK_INDEX_TYPE_UINT32);

		// Scissor rectangle
		VkRect2D scissor_rect {
			{0, 0},
			{(uint32_t)(renderTarget.getBufferSize().x), (uint32_t)(renderTarget.getBufferSize().y) }
		};
		vkCmdSetScissor(commandBuffer, 0, 1, &scissor_rect);

		// Draw individual glyphs
		for (auto& render_glyph : mGlyphCache[mIndex])
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

			// Bind descriptor set
			vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.mLayout, 0, 1, &descriptor_set, 0, nullptr);

			// Draw geometry
			vkCmdDrawIndexed(commandBuffer, index_buffer.getCount(), 1, 0, 0, 0);

			// Update x
			x += render_glyph->getHorizontalAdvance();
		}
	}


	const math::Rect& RenderableTextComponentInstance::getBoundingBox() const
	{
		const static math::Rect empty;
		return mIndex < mTextBounds.size() ? mTextBounds[mIndex] : empty;
	}


	const nap::math::Rect& RenderableTextComponentInstance::getBoundingBox(int index)
	{
		assert(index < mTextBounds.size());
		return mTextBounds[index];
	}


	const std::string& RenderableTextComponentInstance::getText()
	{
		const static std::string empty;
		return mIndex < mLinesCache.size() ? mLinesCache[mIndex] : empty;
	}


	const std::string& RenderableTextComponentInstance::getText(int index)
	{
		assert(index < mLinesCache.size());
		return mLinesCache[index];
	}

}
