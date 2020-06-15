#include "uniforms.h"
#include "imagefromfile.h"
#include "samplers.h"
#include "renderservice.h"


RTTI_BEGIN_ENUM(nap::EFilterMode)
	RTTI_ENUM_VALUE(nap::EFilterMode::Nearest, "Nearest"),
	RTTI_ENUM_VALUE(nap::EFilterMode::Linear, "Linear")
RTTI_END_ENUM

RTTI_BEGIN_ENUM(nap::EAddressMode)
	RTTI_ENUM_VALUE(nap::EAddressMode::Repeat,			"Repeat"),
	RTTI_ENUM_VALUE(nap::EAddressMode::MirroredRepeat,	"MirroredRepeat"),
	RTTI_ENUM_VALUE(nap::EAddressMode::ClampToEdge,		"ClampToEdge"),
	RTTI_ENUM_VALUE(nap::EAddressMode::ClampToBorder,	"ClampToBorder")
RTTI_END_ENUM

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::Sampler)
	RTTI_PROPERTY("Name",					&nap::Sampler::mName,					nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("MinFilter",				&nap::Sampler::mMinFilter,				nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("MaxFilter",				&nap::Sampler::mMaxFilter,				nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("MipMapMode",				&nap::Sampler::mMipMapMode,				nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("AddressModeVertical",	&nap::Sampler::mAddressModeVertical,	nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("AddressModeHorizontal",	&nap::Sampler::mAddressModeHorizontal,	nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("MaxLodLevel",			&nap::Sampler::mMaxLodLevel,			nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::SamplerArray)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::Sampler2D)
	RTTI_PROPERTY("Texture", &nap::Sampler2D::mTexture, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////

RTTI_BEGIN_CLASS(nap::Sampler2DArray)
	RTTI_PROPERTY("Textures", &nap::Sampler2DArray::mTextures, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

namespace nap
{
	VkFilter getFilter(EFilterMode filterMode)
	{
		switch (filterMode)
		{
		case EFilterMode::Nearest:
			return VK_FILTER_NEAREST;

		case EFilterMode::Linear:
			return VK_FILTER_LINEAR;
		}

		assert(false);
		return VK_FILTER_LINEAR;
	}

	VkSamplerMipmapMode getMipMapMode(EFilterMode filterMode)
	{
		switch (filterMode)
		{
		case EFilterMode::Nearest:
			return VK_SAMPLER_MIPMAP_MODE_NEAREST;

		case EFilterMode::Linear:
			return VK_SAMPLER_MIPMAP_MODE_LINEAR;
		}

		assert(false);
		return VK_SAMPLER_MIPMAP_MODE_LINEAR;
	}

	VkSamplerAddressMode getAddressMode(EAddressMode addressMode)
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
		}

		assert(false);
		return VK_SAMPLER_ADDRESS_MODE_REPEAT;
	}

	SamplerInstance::SamplerInstance(RenderService& renderService, const SamplerDeclaration& declaration, const Sampler* sampler, const SamplerChangedCallback& samplerChangedCallback) :
		mSampler(sampler),
		mDeclaration(&declaration),
		mRenderService(&renderService),
		mSamplerChangedCallback(samplerChangedCallback)
	{
	}

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
		samplerInfo.sType					= VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter				= mSampler == nullptr ? VK_FILTER_LINEAR : getFilter(mSampler->mMaxFilter);
		samplerInfo.minFilter				= mSampler == nullptr ? VK_FILTER_LINEAR : getFilter(mSampler->mMinFilter);
		samplerInfo.addressModeU			= mSampler == nullptr ? VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE : getAddressMode(mSampler->mAddressModeHorizontal);
		samplerInfo.addressModeV			= mSampler == nullptr ? VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE : getAddressMode(mSampler->mAddressModeVertical);
		samplerInfo.addressModeW			= VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		samplerInfo.anisotropyEnable		= VK_FALSE;
		samplerInfo.maxAnisotropy			= 16;
		samplerInfo.borderColor				= VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		samplerInfo.unnormalizedCoordinates = VK_FALSE;
		samplerInfo.compareEnable			= VK_FALSE;
		samplerInfo.compareOp				= VK_COMPARE_OP_ALWAYS;
		samplerInfo.mipmapMode				= mSampler == nullptr ? VK_SAMPLER_MIPMAP_MODE_LINEAR : getMipMapMode(mSampler->mMipMapMode);
		samplerInfo.maxLod					= mSampler == nullptr ? 0 : mSampler->mMaxLodLevel;

		return errorState.check(vkCreateSampler(mRenderService->getDevice(), &samplerInfo, nullptr, &mVulkanSampler) == VK_SUCCESS, "Could not initialize sampler");
	}

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

	Sampler2DArrayInstance::Sampler2DArrayInstance(RenderService& renderService, const SamplerDeclaration& declaration, const Sampler2DArray* sampler2DArray, const SamplerChangedCallback& samplerChangedCallback) :
		SamplerInstance(renderService, declaration, sampler2DArray, samplerChangedCallback)
	{
		if (sampler2DArray != nullptr)
			mTextures = sampler2DArray->mTextures;
	}

	void Sampler2DArrayInstance::setTexture(int index, Texture2D& texture)
	{
		mTextures[index] = &texture; 
		raiseChanged();
	}

} // End Namespace NAP
