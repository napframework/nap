#pragma once

#include "irendertarget.h"

namespace nap
{
	class RenderWindow;

	/**
	* Render target that represents a backbuffer.
	*/
	class BackbufferRenderTarget : public IRenderTarget
	{
	public:
		BackbufferRenderTarget(RenderWindow& window);

		virtual void beginRendering() override;
		virtual void endRendering() override;

		/**
		* @param size Size of the render target
		*/
		void setSize(const glm::ivec2& size) { mSize = size; }

		/**
		* @return size of render target.
		*/
		virtual const glm::ivec2 getSize() const override { return mSize; }
		virtual void setClearColor(const glm::vec4& color) override { mClearColor = color; }
		virtual const glm::vec4& getClearColor() const override { return mClearColor; }
		virtual ECullWindingOrder getWindingOrder() const override { return ECullWindingOrder::CounterClockwise; }
		virtual VkRenderPass getRenderPass() const;
		virtual VkFormat getColorFormat() const override;
		virtual VkFormat getDepthFormat() const override;
		virtual VkSampleCountFlagBits getSampleCount() const override;
		virtual bool getSampleShadingEnabled() const override;

	private:
		RenderWindow&	mWindow;
		glm::ivec2		mSize;
		glm::vec4		mClearColor;			// Clear color, used for clearing the color buffer
	};

} // nap
