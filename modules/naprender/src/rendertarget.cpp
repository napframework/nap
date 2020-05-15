// Local Includes
#include "rendertarget.h"
#include "nap\core.h"
#include "renderservice.h"


RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::RenderTarget)
	RTTI_CONSTRUCTOR(nap::Core&)
 	RTTI_PROPERTY("ColorTexture",	&nap::RenderTarget::mColorTexture,	nap::rtti::EPropertyMetaData::Required)
 	RTTI_PROPERTY("DepthTexture",	&nap::RenderTarget::mDepthTexture,	nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("ClearColor",		&nap::RenderTarget::mClearColor,	nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

namespace nap
{
	namespace 
	{
		bool createRenderPass(VkDevice device, VkFormat colorFormat, VkFormat depthFormat, VkRenderPass& renderPass, utility::ErrorState& errorState)
		{
			VkAttachmentDescription colorAttachment = {};
			colorAttachment.format = colorFormat;
			colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
			colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			colorAttachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

			VkAttachmentDescription depthAttachment = {};
			depthAttachment.format = depthFormat;
			depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
			depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

			VkAttachmentReference colorAttachmentRef = {};
			colorAttachmentRef.attachment = 0;
			colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

			VkAttachmentReference depthAttachmentRef = {};
			depthAttachmentRef.attachment = 1;
			depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

			VkSubpassDescription subpass = {};
			subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
			subpass.colorAttachmentCount = 1;
			subpass.pColorAttachments = &colorAttachmentRef;
			subpass.pDepthStencilAttachment = &depthAttachmentRef;

			std::array<VkSubpassDependency, 2> dependencies;
			dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
			dependencies[0].dstSubpass = 0;
			dependencies[0].srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
			dependencies[0].srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
			dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
			
			dependencies[1].srcSubpass = 0;
			dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
			dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
			dependencies[1].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			dependencies[1].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
			dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
	
			std::array<VkAttachmentDescription, 2> attachments = { colorAttachment, depthAttachment };
			VkRenderPassCreateInfo renderPassInfo = {};
			renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
			renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
			renderPassInfo.pAttachments = attachments.data();
			renderPassInfo.subpassCount = 1;
			renderPassInfo.pSubpasses = &subpass;
			renderPassInfo.dependencyCount = dependencies.size();
			renderPassInfo.pDependencies = dependencies.data();

			if (!errorState.check(vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass) == VK_SUCCESS, "Failed to create render pass"))
				return false;

			return true;
		}
	}

	RenderTarget::RenderTarget(Core& core) :
		mRenderService(core.getService<RenderService>())
	{
	}

	RenderTarget::~RenderTarget()
	{
		if (mFramebuffer != nullptr)
			vkDestroyFramebuffer(mRenderService->getDevice(), mFramebuffer, nullptr);
	
		if (mRenderPass != nullptr)
			vkDestroyRenderPass(mRenderService->getDevice(), mRenderPass, nullptr);
	}

	bool RenderTarget::init(utility::ErrorState& errorState)
	{
		if (!errorState.check(mColorTexture->mUsage == ETextureUsage::RenderTarget, "The color texture used by a RenderTarget must have a usage of 'RenderTarget' set."))
			return false;

		if (!errorState.check(mDepthTexture->mUsage == ETextureUsage::RenderTarget, "The depth texture used by a RenderTarget must have a usage of 'RenderTarget' set."))
			return false;

		if (!errorState.check(mColorTexture->getSize() == mDepthTexture->getSize(), "The color & depth textures used by a RenderTarget must have the same size."))
			return false;

		glm::ivec2 size = mColorTexture->getSize();

		if (!createRenderPass(mRenderService->getDevice(), mColorTexture->getVulkanFormat(), mDepthTexture->getVulkanFormat(), mRenderPass, errorState))
			return false;

		VkExtent2D framebuffer_size;
		framebuffer_size.width = size.x;
		framebuffer_size.height = size.y;

		std::array<VkImageView, 2> attachments = 
		{
			mColorTexture->getImageView(),
			mDepthTexture->getImageView()
		};

		VkFramebufferCreateInfo framebufferInfo = {};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = mRenderPass;
		framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		framebufferInfo.pAttachments = attachments.data();
		framebufferInfo.width = framebuffer_size.width;
		framebufferInfo.height = framebuffer_size.height;
		framebufferInfo.layers = 1;

		if (!errorState.check(vkCreateFramebuffer(mRenderService->getDevice(), &framebufferInfo, nullptr, &mFramebuffer) == VK_SUCCESS, "Failed to create framebuffer"))
			return false;

		return true;
	}

	void RenderTarget::beginRendering()
	{
		glm::ivec2 size = mColorTexture->getSize();

		VkRenderPassBeginInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = mRenderPass;
		renderPassInfo.framebuffer = mFramebuffer;
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = { (uint32_t)size.x, (uint32_t)size.y };

		std::array<VkClearValue, 2> clearValues = {};
		clearValues[0].color = { mClearColor.r, mClearColor.g, mClearColor.b, mClearColor.a };
		clearValues[1].depthStencil = { 1.0f, 0 };

		renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
		renderPassInfo.pClearValues = clearValues.data();

		vkCmdBeginRenderPass(mRenderService->getCurrentCommandBuffer(), &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		VkRect2D rect;
		rect.offset.x = 0;
		rect.offset.y = 0;
		rect.extent.width = size.x;
		rect.extent.height = size.y;
		vkCmdSetScissor(mRenderService->getCurrentCommandBuffer(), 0, 1, &rect);

		VkViewport viewport = {};
		viewport.x = 0.0f;
		viewport.y = size.y;
		viewport.width = size.x;
		viewport.height = -size.y;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		vkCmdSetViewport(mRenderService->getCurrentCommandBuffer(), 0, 1, &viewport);
	}

	void RenderTarget::endRendering()
	{
		vkCmdEndRenderPass(mRenderService->getCurrentCommandBuffer());
	}

	const glm::ivec2 RenderTarget::getSize() const
	{
		return mColorTexture->getSize();
	}

	RenderTexture2D& RenderTarget::getColorTexture()
	{
		return *mColorTexture;
	}

	VkFormat RenderTarget::getColorFormat() const
	{
		return mColorTexture->getVulkanFormat();
	}

	VkFormat RenderTarget::getDepthFormat() const
	{
		return mDepthTexture->getVulkanFormat();
	}
} // nap