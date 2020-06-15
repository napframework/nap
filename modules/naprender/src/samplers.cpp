#include "uniforms.h"
#include "imagefromfile.h"
#include "samplers.h"
#include "renderservice.h"


RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::Sampler)
	RTTI_PROPERTY("Name", &nap::Sampler::mName, nap::rtti::EPropertyMetaData::Default)
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

	SamplerInstance::SamplerInstance(RenderService& renderService, const SamplerDeclaration& declaration, const SamplerChangedCallback& samplerChangedCallback) :
		mDeclaration(&declaration),
		mRenderService(&renderService),
		mSamplerChangedCallback(samplerChangedCallback)
	{
	}

	SamplerInstance::~SamplerInstance()
	{
		mRenderService->queueVulkanObjectDestructor([sampler = mSampler](RenderService& renderService)
		{
			if (sampler != nullptr)
				vkDestroySampler(renderService.getDevice(), sampler, nullptr);
		});
	}

	bool SamplerInstance::init(utility::ErrorState& errorState)
	{
		VkSamplerCreateInfo samplerInfo = {};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter = VK_FILTER_LINEAR;
		samplerInfo.minFilter = VK_FILTER_LINEAR;
		samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		samplerInfo.anisotropyEnable = VK_FALSE;
		samplerInfo.maxAnisotropy = 16;
		samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		samplerInfo.unnormalizedCoordinates = VK_FALSE;
		samplerInfo.compareEnable = VK_FALSE;
		samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

		return errorState.check(vkCreateSampler(mRenderService->getDevice(), &samplerInfo, nullptr, &mSampler) == VK_SUCCESS, "Could not initialize sampler");
	}

	//////////////////////////////////////////////////////////////////////////

	Sampler2DInstance::Sampler2DInstance(RenderService& renderService, const SamplerDeclaration& declaration, const Sampler2D* sampler2D, const SamplerChangedCallback& samplerChangedCallback) :
		SamplerInstance(renderService, declaration, samplerChangedCallback)
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
		SamplerInstance(renderService, declaration, samplerChangedCallback)
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
