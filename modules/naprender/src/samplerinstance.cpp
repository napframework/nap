#include "samplerinstance.h"
#include "renderservice.h"

RTTI_DEFINE_BASE(nap::SamplerInstance)

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::Sampler2DInstance)
	RTTI_CONSTRUCTOR(nap::RenderService&, const nap::SamplerDeclaration&, const nap::Sampler2D*, const nap::SamplerChangedCallback&)
	RTTI_FUNCTION("setTexture", &nap::Sampler2DInstance::setTexture)
	RTTI_FUNCTION("hasTexture", &nap::Sampler2DInstance::hasTexture)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::Sampler2DArrayInstance)
	RTTI_CONSTRUCTOR(nap::RenderService&, const nap::SamplerDeclaration&, const nap::Sampler2DArray*, const nap::SamplerChangedCallback&)
	RTTI_FUNCTION("setTexture",		&nap::Sampler2DArrayInstance::setTexture)
	RTTI_FUNCTION("hasTexture",		&nap::Sampler2DArrayInstance::hasTexture)
	RTTI_FUNCTION("getNumElements",	&nap::Sampler2DArrayInstance::getNumElements)
RTTI_END_CLASS

namespace nap
{
	//////////////////////////////////////////////////////////////////////////
	// Static methods
	//////////////////////////////////////////////////////////////////////////

	static VkFilter getFilter(EFilterMode filterMode)
	{
		switch (filterMode)
		{
		case EFilterMode::Nearest:
			return VK_FILTER_NEAREST;
		case EFilterMode::Linear:
			return VK_FILTER_LINEAR;
		default:
			assert(false);
			return VK_FILTER_LINEAR;
		}
	}


	static VkSamplerMipmapMode getMipMapMode(EFilterMode filterMode)
	{
		switch (filterMode)
		{
		case EFilterMode::Nearest:
			return VK_SAMPLER_MIPMAP_MODE_NEAREST;
		case EFilterMode::Linear:
			return VK_SAMPLER_MIPMAP_MODE_LINEAR;
		default:
			assert(false);
			return VK_SAMPLER_MIPMAP_MODE_LINEAR;
		}
	}


	static VkSamplerAddressMode getAddressMode(EAddressMode addressMode)
	{
		switch (addressMode)
		{
		case EAddressMode::Repeat:
			return VK_SAMPLER_ADDRESS_MODE_REPEAT;
		case EAddressMode::MirroredRepeat:
			return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
		case EAddressMode::ClampToEdge:
			return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		case EAddressMode::ClampToBorder:
			return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
		default:
			assert(false);
			return VK_SAMPLER_ADDRESS_MODE_REPEAT;
		}
	}


	static float getAnisotropicSamples(const Sampler* sampler, const nap::RenderService& renderer)
	{
		// If there is no sampler or setting is derived from system default, use the global setting
		if (sampler == nullptr || sampler->mMaxAnisotropy == EAnisotropicSamples::Default)
			return renderer.getAnisotropicSamples();

		// Otherwise check if it is supported, if so override, otherwise set it to 1.0
		return renderer.anisotropicFilteringSupported() ? static_cast<float>(sampler->mMaxAnisotropy) : 1.0f;
	}


	//////////////////////////////////////////////////////////////////////////
	// SamplerInstance
	//////////////////////////////////////////////////////////////////////////

	SamplerInstance::SamplerInstance(RenderService& renderService, const SamplerDeclaration& declaration, const Sampler* sampler, const SamplerChangedCallback& samplerChangedCallback) :
		mSampler(sampler),
		mDeclaration(&declaration),
		mRenderService(&renderService),
		mSamplerChangedCallback(samplerChangedCallback)
	{ }


	SamplerInstance::~SamplerInstance()
	{
		mRenderService->queueVulkanObjectDestructor([sampler = mVulkanSampler](RenderService& renderService)
		{
			if (sampler != nullptr)
				vkDestroySampler(renderService.getDevice(), sampler, nullptr);
		});
	}


	bool SamplerInstance::init(utility::ErrorState& errorState)
	{
		VkSamplerCreateInfo samplerInfo = {};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter = mSampler == nullptr ? VK_FILTER_LINEAR : getFilter(mSampler->mMaxFilter);
		samplerInfo.minFilter = mSampler == nullptr ? VK_FILTER_LINEAR : getFilter(mSampler->mMinFilter);
		samplerInfo.addressModeU = mSampler == nullptr ? VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE : getAddressMode(mSampler->mAddressModeHorizontal);
		samplerInfo.addressModeV = mSampler == nullptr ? VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE : getAddressMode(mSampler->mAddressModeVertical);
		samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		samplerInfo.anisotropyEnable = mRenderService->anisotropicFilteringSupported() ? VK_TRUE : VK_FALSE;
		samplerInfo.maxAnisotropy = getAnisotropicSamples(mSampler, *mRenderService);
		samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		samplerInfo.unnormalizedCoordinates = VK_FALSE;
		samplerInfo.compareEnable = VK_FALSE;
		samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
		samplerInfo.mipmapMode = mSampler == nullptr ? VK_SAMPLER_MIPMAP_MODE_LINEAR : getMipMapMode(mSampler->mMipMapMode);
		samplerInfo.minLod = 0.0f;
		samplerInfo.maxLod = mSampler == nullptr ? VK_LOD_CLAMP_NONE : static_cast<float>(mSampler->mMaxLodLevel);
		samplerInfo.mipLodBias = 0.0f;

		return errorState.check(vkCreateSampler(mRenderService->getDevice(), &samplerInfo, nullptr, &mVulkanSampler) == VK_SUCCESS, "Could not initialize sampler");
	}


	//////////////////////////////////////////////////////////////////////////
	// Sampler2DInstance
	//////////////////////////////////////////////////////////////////////////

	Sampler2DInstance::Sampler2DInstance(RenderService& renderService, const SamplerDeclaration& declaration, const Sampler2D* sampler2D, const SamplerChangedCallback& samplerChangedCallback) :
		SamplerInstance(renderService, declaration, sampler2D, samplerChangedCallback)
	{
		if (sampler2D != nullptr)
			mTexture2D = sampler2D->mTexture;
	}


	void Sampler2DInstance::onTextureChanged(const Texture2D&)
	{
		raiseChanged();
	}


	void Sampler2DInstance::setTexture(Texture2D& texture)
	{
		mTexture2D = &texture;
		raiseChanged();
	}

	
	//////////////////////////////////////////////////////////////////////////
	// Sampler2DArrayInstance
	//////////////////////////////////////////////////////////////////////////

	Sampler2DArrayInstance::Sampler2DArrayInstance(RenderService& renderService, const SamplerDeclaration& declaration, const Sampler2DArray* sampler2DArray, const SamplerChangedCallback& samplerChangedCallback) :
		SamplerInstance(renderService, declaration, sampler2DArray, samplerChangedCallback)
	{
		if (sampler2DArray != nullptr)
			mTextures = sampler2DArray->mTextures;
	}


	void Sampler2DArrayInstance::setTexture(int index, Texture2D& texture)
	{
		assert(index < mTextures.size());
		mTextures[index] = &texture;
		raiseChanged();
	}
}