// Local Includes
#include "rendertarget.h"
#include "nap\core.h"
#include "renderservice.h"

RTTI_BEGIN_ENUM(nap::ERenderTargetFormat)
	RTTI_ENUM_VALUE(nap::ERenderTargetFormat::RGBA8,	"RGBA8"),
	RTTI_ENUM_VALUE(nap::ERenderTargetFormat::R8,		"R8"),	
	RTTI_ENUM_VALUE(nap::ERenderTargetFormat::Depth,	"Depth")
RTTI_END_ENUM


RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::RenderTarget)
	RTTI_CONSTRUCTOR(nap::Core&)
// 	RTTI_PROPERTY("mColorTexture",	&nap::RenderTarget::mColorTexture, nap::rtti::EPropertyMetaData::Required)
// 	RTTI_PROPERTY("mDepthTexture",	&nap::RenderTarget::mDepthTexture, nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Size",		&nap::RenderTarget::mSize,			nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("ClearColor",	&nap::RenderTarget::mClearColor,	nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

namespace nap
{
	namespace 
	{
		static bool createFramebuffers(VkDevice device, std::vector<VkFramebuffer>& framebuffers, std::vector<VkImageView>& colorImageViews, std::vector<VkImageView>& depthImageViews, VkRenderPass renderPass, VkExtent2D extent, utility::ErrorState& errorState)
		{
			framebuffers.resize(colorImageViews.size());

			for (size_t i = 0; i < colorImageViews.size(); i++)
			{
				std::array<VkImageView, 2> attachments = {
					colorImageViews[i],
					depthImageViews[i]
				};

				VkFramebufferCreateInfo framebufferInfo = {};
				framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
				framebufferInfo.renderPass = renderPass;
				framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
				framebufferInfo.pAttachments = attachments.data();
				framebufferInfo.width = extent.width;
				framebufferInfo.height = extent.height;
				framebufferInfo.layers = 1;

				if (!errorState.check(vkCreateFramebuffer(device, &framebufferInfo, nullptr, &framebuffers[i]) == VK_SUCCESS, "Failed to create framebuffer"))
					return false;
			}

			return true;
		}
	}

	RenderTarget::RenderTarget(Core& core) :
		mRenderService(core.getService<RenderService>())
	{
		for (int i = 0; i < 2; ++i)
		{
			mColorTextures[i] = std::make_unique<Texture2D>(core);
			mDepthTextures[i] = std::make_unique<Texture2D>(core);
		}
	}

	bool RenderTarget::init(utility::ErrorState& errorState)
	{
		SurfaceDescriptor color_descriptor;
		color_descriptor.mWidth = mSize.x;
		color_descriptor.mHeight = mSize.y;
		color_descriptor.mDataType = ESurfaceDataType::BYTE;
		color_descriptor.mColorSpace = EColorSpace::sRGB;
		color_descriptor.mChannels = ESurfaceChannels::BGRA;

		SurfaceDescriptor depth_descriptor = color_descriptor;
		depth_descriptor.mChannels = ESurfaceChannels::Depth;

		std::vector<VkImageView> color_image_views;
		std::vector<VkImageView> depth_image_views;
		for (int i = 0; i < mColorTextures.size(); ++i)
		{
			if (!mColorTextures[i]->init(color_descriptor, false, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, errorState))
				return false;

			color_image_views.push_back(mColorTextures[i]->getImageView());

			if (!mDepthTextures[i]->init(depth_descriptor, false, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, errorState))
				return false;

			depth_image_views.push_back(mDepthTextures[i]->getImageView());
		}

		mRenderPass = mRenderService->getOrCreateRenderPass(ERenderTargetFormat::RGBA8, false);

		VkExtent2D framebuffer_size;
		framebuffer_size.width = mSize.x;
		framebuffer_size.height = mSize.y;

		if (!createFramebuffers(mRenderService->getDevice(), mFramebuffers, color_image_views, depth_image_views, mRenderPass, framebuffer_size, errorState))
			return false;

		return true;
	}

	void RenderTarget::beginRendering()
	{
		VkRenderPassBeginInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = mRenderPass;
		renderPassInfo.framebuffer = mFramebuffers[mRenderService->getCurrentFrameIndex()];
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = { (uint32_t)mSize.x, (uint32_t)mSize.y };

		std::array<VkClearValue, 2> clearValues = {};
		clearValues[0].color = { mClearColor.r, mClearColor.g, mClearColor.b, mClearColor.a };
		clearValues[1].depthStencil = { 1.0f, 0 };

		renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
		renderPassInfo.pClearValues = clearValues.data();

		vkCmdBeginRenderPass(mRenderService->getCurrentCommandBuffer(), &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		VkRect2D rect;
		rect.offset.x = 0;
		rect.offset.y = 0;
		rect.extent.width = mSize.x;
		rect.extent.height = mSize.y;
		vkCmdSetScissor(mRenderService->getCurrentCommandBuffer(), 0, 1, &rect);

		VkViewport viewport = {};
		viewport.x = 0.0f;
		viewport.y = mSize.y;
		viewport.width = mSize.x;
		viewport.height = -mSize.y;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		vkCmdSetViewport(mRenderService->getCurrentCommandBuffer(), 0, 1, &viewport);
	}

	void RenderTarget::endRendering()
	{
		vkCmdEndRenderPass(mRenderService->getCurrentCommandBuffer());
	}

	Texture2D& RenderTarget::getColorTexture()
	{
		return *mColorTextures[mRenderService->getCurrentFrameIndex()];
	}

} // nap