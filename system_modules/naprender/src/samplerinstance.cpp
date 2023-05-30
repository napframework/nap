/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "samplerinstance.h"
#include "renderservice.h"

RTTI_DEFINE_BASE(nap::SamplerInstance)

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::Sampler2DInstance)
	RTTI_CONSTRUCTOR(nap::RenderService&, const nap::SamplerDeclaration&, const nap::Sampler2D*, const nap::SamplerChangedCallback&)
	RTTI_FUNCTION("setTexture",		&nap::Sampler2DInstance::setTexture)
	RTTI_FUNCTION("hasTexture",		&nap::Sampler2DInstance::hasTexture)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::Sampler2DArrayInstance)
	RTTI_CONSTRUCTOR(nap::RenderService&, const nap::SamplerDeclaration&, const nap::Sampler2DArray*, const nap::SamplerChangedCallback&)
	RTTI_FUNCTION("setTexture",		&nap::Sampler2DArrayInstance::setTexture)
	RTTI_FUNCTION("hasTexture",		&nap::Sampler2DArrayInstance::hasTexture)
	RTTI_FUNCTION("getNumElements",	&nap::Sampler2DArrayInstance::getNumElements)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::SamplerCubeInstance)
	RTTI_CONSTRUCTOR(nap::RenderService&, const nap::SamplerDeclaration&, const nap::SamplerCube*, const nap::SamplerChangedCallback&)
	RTTI_FUNCTION("setTexture",		&nap::SamplerCubeInstance::setTexture)
	RTTI_FUNCTION("hasTexture",		&nap::SamplerCubeInstance::hasTexture)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::SamplerCubeArrayInstance)
	RTTI_CONSTRUCTOR(nap::RenderService&, const nap::SamplerDeclaration&, const nap::SamplerCubeArray*, const nap::SamplerChangedCallback&)
	RTTI_FUNCTION("setTexture",		&nap::SamplerCubeArrayInstance::setTexture)
	RTTI_FUNCTION("hasTexture",		&nap::SamplerCubeArrayInstance::hasTexture)
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


	/**
	 * @return depth compare operation
	 */
	static VkCompareOp getDepthCompareOp(EDepthCompareMode compareMode)
	{
		switch (compareMode)
		{
		case EDepthCompareMode::Never:
			return VK_COMPARE_OP_NEVER;
		case EDepthCompareMode::Less:
			return VK_COMPARE_OP_LESS;
		case EDepthCompareMode::Equal:
			return VK_COMPARE_OP_EQUAL;
		case EDepthCompareMode::LessOrEqual:
			return VK_COMPARE_OP_LESS_OR_EQUAL;
		case EDepthCompareMode::Greater:
			return VK_COMPARE_OP_GREATER;
		case EDepthCompareMode::NotEqual:
			return VK_COMPARE_OP_NOT_EQUAL;
		case EDepthCompareMode::GreaterOrEqual:
			return VK_COMPARE_OP_GREATER_OR_EQUAL;
		case EDepthCompareMode::Always:
			return VK_COMPARE_OP_ALWAYS;
		default:
			assert(false);
			return VK_COMPARE_OP_LESS_OR_EQUAL;
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
		mRenderService(&renderService),
        mSampler(sampler),
        mDeclaration(&declaration),
		mSamplerChangedCallback(samplerChangedCallback)
	{ }


	SamplerInstance::~SamplerInstance()
	{
		mRenderService->queueVulkanObjectDestructor([sampler = mVulkanSampler](RenderService& renderService)
		{
			if (sampler != VK_NULL_HANDLE)
				vkDestroySampler(renderService.getDevice(), sampler, nullptr);
		});
	}


	bool SamplerInstance::init(utility::ErrorState& errorState)
	{
		VkSamplerCreateInfo samplerInfo		= {};
		samplerInfo.sType					= VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter				= mSampler == nullptr ? VK_FILTER_LINEAR : getFilter(mSampler->mMaxFilter);
		samplerInfo.minFilter				= mSampler == nullptr ? VK_FILTER_LINEAR : getFilter(mSampler->mMinFilter);
		samplerInfo.addressModeU			= mSampler == nullptr ? VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE : getAddressMode(mSampler->mAddressModeHorizontal);
		samplerInfo.addressModeV			= mSampler == nullptr ? VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE : getAddressMode(mSampler->mAddressModeVertical);
		samplerInfo.addressModeW			= VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		samplerInfo.anisotropyEnable		= mRenderService->anisotropicFilteringSupported() ? VK_TRUE : VK_FALSE;
		samplerInfo.maxAnisotropy			= getAnisotropicSamples(mSampler, *mRenderService);
		samplerInfo.borderColor				= VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		samplerInfo.unnormalizedCoordinates = VK_FALSE;
		samplerInfo.compareEnable			= mSampler == nullptr ? VK_FALSE : mSampler->mEnableCompare ? VK_TRUE : VK_FALSE;
		samplerInfo.compareOp				= mSampler == nullptr ? VK_COMPARE_OP_LESS_OR_EQUAL : getDepthCompareOp(mSampler->mCompareMode);
		samplerInfo.mipmapMode				= mSampler == nullptr ? VK_SAMPLER_MIPMAP_MODE_LINEAR : getMipMapMode(mSampler->mMipMapMode);
		samplerInfo.minLod					= 0.0f;
		samplerInfo.maxLod					= mSampler == nullptr ? VK_LOD_CLAMP_NONE : static_cast<float>(mSampler->mMaxLodLevel);
		samplerInfo.mipLodBias				= 0.0f;

		return errorState.check(vkCreateSampler(mRenderService->getDevice(), &samplerInfo, nullptr, &mVulkanSampler) == VK_SUCCESS, "Could not initialize sampler");
	}


	//////////////////////////////////////////////////////////////////////////
	// SamplerArrayInstance
	//////////////////////////////////////////////////////////////////////////

	SamplerArrayInstance::SamplerArrayInstance(RenderService& renderService, const SamplerDeclaration& declaration, const Sampler* sampler, const SamplerChangedCallback& samplerChangedCallback) :
		SamplerInstance(renderService, declaration, sampler, samplerChangedCallback)
	{ }


	SamplerArrayInstance::~SamplerArrayInstance()
	{ }


	//////////////////////////////////////////////////////////////////////////
	// Sampler2DInstance
	//////////////////////////////////////////////////////////////////////////

	Sampler2DInstance::Sampler2DInstance(RenderService& renderService, const SamplerDeclaration& declaration, const Sampler2D* sampler2D, const SamplerChangedCallback& samplerChangedCallback) :
		SamplerInstance(renderService, declaration, sampler2D, samplerChangedCallback)
	{
		if (sampler2D != nullptr)
			mTexture2D = sampler2D->mTexture;
	}


	void Sampler2DInstance::setTexture(Texture2D& texture)
	{
		if (mTexture2D != nullptr)
			mTexture2D->textureDestroyed.disconnect(textureDestroyedSlot);

		mTexture2D = &texture;
		mTexture2D->textureDestroyed.connect(textureDestroyedSlot);

		raiseChanged();
	}


	void Sampler2DInstance::onTextureDestroyed()
	{
		// Check if the current texture is different from the resource, if so, reset it
		if (mSampler != nullptr)
		{
			const auto* sampler_2d = static_cast<const Sampler2D*>(mSampler);
			if (mTexture2D != sampler_2d->mTexture.get())
				setTexture(*sampler_2d->mTexture);
		}
		else
		{
			setTexture(mRenderService->getEmptyTexture2D());
		}
	}

	
	//////////////////////////////////////////////////////////////////////////
	// Sampler2DArrayInstance
	//////////////////////////////////////////////////////////////////////////

	Sampler2DArrayInstance::Sampler2DArrayInstance(RenderService& renderService, const SamplerDeclaration& declaration, const Sampler2DArray* sampler2DArray, const SamplerChangedCallback& samplerChangedCallback) :
		SamplerArrayInstance(renderService, declaration, sampler2DArray, samplerChangedCallback)
	{
		if (sampler2DArray != nullptr)
		{
			assert(sampler2DArray->mTextures.size() <= declaration.mNumArrayElements);
			mTextures = sampler2DArray->mTextures;
		}

		// Ensure we create array entries for all textures
		mTextures.reserve(declaration.mNumArrayElements);
		uint count = mTextures.size();
		for (uint i = count; i < declaration.mNumArrayElements; i++)
			mTextures.emplace_back();
	}


	void Sampler2DArrayInstance::setTexture(int index, Texture2D& texture)
	{
		assert(index < mTextures.size());
		assert(index < mDeclaration->mNumArrayElements);
		mTextures[index] = &texture;
		raiseChanged(index);
	}


	//////////////////////////////////////////////////////////////////////////
	// SamplerCubeInstance
	//////////////////////////////////////////////////////////////////////////

	SamplerCubeInstance::SamplerCubeInstance(RenderService& renderService, const SamplerDeclaration& declaration, const SamplerCube* samplerCube, const SamplerChangedCallback& samplerChangedCallback) :
		SamplerInstance(renderService, declaration, samplerCube, samplerChangedCallback)
	{
		if (samplerCube != nullptr)
			mTextureCube = samplerCube->mTextureCube;
	}


	void SamplerCubeInstance::setTexture(TextureCube& textureCube)
	{
		if (mTextureCube != nullptr)
			mTextureCube->textureDestroyed.disconnect(textureDestroyedSlot);

		mTextureCube = &textureCube;
		mTextureCube->textureDestroyed.connect(textureDestroyedSlot);

		raiseChanged();
	}


	void SamplerCubeInstance::onTextureDestroyed()
	{
		// Check if the current texture is different from the resource, if so, reset it
		if (mSampler != nullptr)
		{
			const auto* sampler_cube = static_cast<const SamplerCube*>(mSampler);
			if (mTextureCube != sampler_cube->mTextureCube.get())
				setTexture(*sampler_cube->mTextureCube);
		}
		else
		{
			setTexture(mRenderService->getEmptyTextureCube());
		}
	}


	//////////////////////////////////////////////////////////////////////////
	// SamplerCubeArrayInstance
	//////////////////////////////////////////////////////////////////////////

	SamplerCubeArrayInstance::SamplerCubeArrayInstance(RenderService& renderService, const SamplerDeclaration& declaration, const SamplerCubeArray* samplerCubeArray, const SamplerChangedCallback& samplerChangedCallback) :
		SamplerArrayInstance(renderService, declaration, samplerCubeArray, samplerChangedCallback)
	{
		if (samplerCubeArray != nullptr)
		{
			assert(samplerCubeArray->mTextures.size() <= declaration.mNumArrayElements);
			mTextures = samplerCubeArray->mTextures;
		}

		// Ensure we create array entries for all textures
		mTextures.reserve(declaration.mNumArrayElements);
		uint count = mTextures.size();
		for (uint i = count; i < declaration.mNumArrayElements; i++)
			mTextures.emplace_back();
	}


	void SamplerCubeArrayInstance::setTexture(int index, TextureCube& textureCube)
	{
		assert(index < mTextures.size());
		assert(index < mDeclaration->mNumArrayElements);
		mTextures[index] = &textureCube;
		raiseChanged(index);
	}
}
