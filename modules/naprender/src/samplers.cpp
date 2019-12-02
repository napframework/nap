#include "uniforms.h"
#include "nglutils.h"
#include "imagefromfile.h"
#include "samplers.h"


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

	SamplerInstance::SamplerInstance(VkDevice device, const opengl::SamplerDeclaration& declaration) :
		mDeclaration(&declaration)
	{
		VkSamplerCreateInfo samplerInfo = {};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter = VK_FILTER_LINEAR;
		samplerInfo.minFilter = VK_FILTER_LINEAR;
		samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.anisotropyEnable = VK_FALSE;
		samplerInfo.maxAnisotropy = 16;
		samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		samplerInfo.unnormalizedCoordinates = VK_FALSE;
		samplerInfo.compareEnable = VK_FALSE;
		samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

		VkResult result = vkCreateSampler(device, &samplerInfo, nullptr, &mSampler);
		assert(result == VK_SUCCESS);
	}

	//////////////////////////////////////////////////////////////////////////

	Sampler2DInstance::Sampler2DInstance(VkDevice device, const opengl::SamplerDeclaration& declaration, const Sampler2D* sampler2D) :
		SamplerInstance(device, declaration)
	{
		if (sampler2D != nullptr)
			mTexture2D = sampler2D->mTexture;
	}

	//////////////////////////////////////////////////////////////////////////

	Sampler2DArrayInstance::Sampler2DArrayInstance(VkDevice device, const opengl::SamplerDeclaration& declaration, const Sampler2DArray* sampler2DArray) :
		SamplerInstance(device, declaration)
	{
		if (sampler2DArray != nullptr)
			mTextures = sampler2DArray->mTextures;
	}

} // End Namespace NAP
