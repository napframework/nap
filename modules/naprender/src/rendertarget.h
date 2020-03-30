#pragma once

// External Includes
#include <nap/resource.h>
#include "irendertarget.h"
#include "texture2d.h"

namespace nap
{
	enum class ERenderTargetFormat
	{
//		Backbuffer,		///< The current native format of the color backbuffer
		RGBA8,			///< RGBA8 4 components, 8 bytes per component
		R8,				///< R8	1 components, 8 bytes per component
		Depth			///< Depth Texture used for binding to depth buffer
	};

	/**
	 * A resource that is used to render objects to an off screen surface (set of textures).
	 * This objects requires a link to a color and depth texture and internally manages an opengl render target.
	 * The result of the render step is stored in the linked textures.
	 */
	class NAPAPI RenderTarget : public Resource, public IRenderTarget
	{
		RTTI_ENABLE(Resource)
	public:
		RenderTarget(Core& core);

		/**
		* Creates internal OpengL render target, bound to color and depth textures.
		*/
		virtual bool init(utility::ErrorState& errorState) override;

		virtual void beginRendering() override;
		virtual void endRendering() override;

		virtual const glm::ivec2 getSize() const { return mSize; }

		virtual void setClearColor(const glm::vec4& color) override { mClearColor = color; }
		virtual const glm::vec4& getClearColor() const override { return mClearColor; }

		virtual ECullWindingOrder getWindingOrder() const override { return ECullWindingOrder::Clockwise; }

		Texture2D& getColorTexture();

	public:
		glm::vec4	mClearColor;			// Clear color, used for clearing the color buffer
		glm::ivec2	mSize;

	private:
		using TextureArray = std::array<std::unique_ptr<Texture2D>, 2>;
		using FramebufferList = std::vector<VkFramebuffer>;

		RenderService*	mRenderService;
		TextureArray	mColorTextures;
		TextureArray	mDepthTextures;
		FramebufferList	mFramebuffers;
		VkRenderPass	mRenderPass = nullptr;
	};
}
