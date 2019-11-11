#include "uniforms.h"
#include "nglutils.h"
#include "imagefromfile.h"


RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::Uniform)
	RTTI_PROPERTY("Name", &nap::Uniform::mName, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::UniformValue)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::UniformValueArray)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::UniformSampler)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::UniformSamplerArray)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::UniformStruct)
	RTTI_PROPERTY("Uniforms", &nap::UniformStruct::mUniforms, nap::rtti::EPropertyMetaData::Embedded)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::UniformStructArray)
	RTTI_PROPERTY("Structs", &nap::UniformStructArray::mStructs, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::UniformInt)
	RTTI_PROPERTY("Value", &nap::UniformInt::mValue, nap::rtti::EPropertyMetaData::Required)
	RTTI_FUNCTION("setValue", &nap::UniformInt::setValue)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::UniformFloat)
	RTTI_PROPERTY("Value", &nap::UniformFloat::mValue, nap::rtti::EPropertyMetaData::Required)
	RTTI_FUNCTION("setValue", &nap::UniformFloat::setValue)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::UniformVec3)
	RTTI_PROPERTY("Value", &nap::UniformVec3::mValue, nap::rtti::EPropertyMetaData::Required)
	RTTI_FUNCTION("setValue", &nap::UniformVec3::setValue)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::UniformVec4)
	RTTI_PROPERTY("Value", &nap::UniformVec4::mValue, nap::rtti::EPropertyMetaData::Required)
	RTTI_FUNCTION("setValue", &nap::UniformVec4::setValue)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::UniformMat4)
	RTTI_PROPERTY("Value", &nap::UniformMat4::mValue, nap::rtti::EPropertyMetaData::Required)
	RTTI_FUNCTION("setValue", &nap::UniformMat4::setValue)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::UniformSampler2D)
	RTTI_PROPERTY("Texture", &nap::UniformSampler2D::mTexture, nap::rtti::EPropertyMetaData::Required)
	RTTI_FUNCTION("setTexture", &nap::UniformSampler2D::setTexture)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////

RTTI_BEGIN_CLASS(nap::UniformIntArray)
	RTTI_PROPERTY("Values", &nap::UniformIntArray::mValues, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::UniformFloatArray)
	RTTI_PROPERTY("Values", &nap::UniformFloatArray::mValues, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::UniformVec3Array)
	RTTI_PROPERTY("Values", &nap::UniformVec3Array::mValues, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::UniformVec4Array)
	RTTI_PROPERTY("Values", &nap::UniformVec4Array::mValues, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::UniformMat4Array)
	RTTI_PROPERTY("Values", &nap::UniformMat4Array::mValues, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::UniformSampler2DArray)
	RTTI_PROPERTY("Textures", &nap::UniformSampler2DArray::mTextures, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

namespace nap
{
	void UniformStruct::addUniform(Uniform& uniform)
	{
		mUniforms.push_back(&uniform);
	}

	Uniform* UniformStruct::findUniform(const std::string& name)
	{
		auto pos = std::find_if(mUniforms.begin(), mUniforms.end(), [name](auto& uniform)
		{
			return uniform->mName == name;
		});

		if (pos == mUniforms.end())
			return nullptr;

		return (*pos).get();
	}

	void UniformStructArray::insertStruct(int index, UniformStruct& uniformStruct)
	{
		if (mStructs.size() <= index)
			mStructs.resize(index + 1);
		
		mStructs[index] = &uniformStruct;
	}

	void UniformInt::push(uint8_t* uniformBuffer) const
	{
		assert(sizeof(mValue) == mDeclaration->mSize);
		memcpy(uniformBuffer + mDeclaration->mOffset, &mValue, sizeof(mValue));
	}


	void UniformFloat::push(uint8_t* uniformBuffer) const
	{
		assert(sizeof(mValue) == mDeclaration->mSize);
		memcpy(uniformBuffer + mDeclaration->mOffset, &mValue, sizeof(mValue));
	}


	void UniformVec3::push(uint8_t* uniformBuffer) const
	{
		assert(sizeof(mValue) == mDeclaration->mSize);
		memcpy(uniformBuffer + mDeclaration->mOffset, &mValue, sizeof(mValue));
	}


	void UniformVec4::push(uint8_t* uniformBuffer) const
	{
		assert(sizeof(mValue) == mDeclaration->mSize);
		memcpy(uniformBuffer + mDeclaration->mOffset, &mValue, sizeof(mValue));
	}


	void UniformMat4::push(uint8_t* uniformBuffer) const
	{
		assert(sizeof(mValue) == mDeclaration->mSize);
		memcpy(uniformBuffer + mDeclaration->mOffset, &mValue, sizeof(mValue));
	}

	UniformSampler::UniformSampler(VkDevice device, const opengl::UniformSamplerDeclaration& declaration) :
		mDeclaration(&declaration)
	{
		VkSamplerCreateInfo samplerInfo = {};
		samplerInfo.sType					= VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter				= VK_FILTER_LINEAR;
		samplerInfo.minFilter				= VK_FILTER_LINEAR;
		samplerInfo.addressModeU			= VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeV			= VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeW			= VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.anisotropyEnable		= VK_FALSE;
		samplerInfo.maxAnisotropy			= 16;
		samplerInfo.borderColor				= VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		samplerInfo.unnormalizedCoordinates = VK_FALSE;
		samplerInfo.compareEnable			= VK_FALSE;
		samplerInfo.compareOp				= VK_COMPARE_OP_ALWAYS;
		samplerInfo.mipmapMode				= VK_SAMPLER_MIPMAP_MODE_LINEAR;

		VkResult result = vkCreateSampler(device, &samplerInfo, nullptr, &mSampler);
		assert(result == VK_SUCCESS);
	}

	UniformSampler2D::UniformSampler2D(VkDevice device, const opengl::UniformSamplerDeclaration& declaration) :
		UniformSampler(device, declaration)
	{
	}


	//////////////////////////////////////////////////////////////////////////

	void UniformIntArray::push(uint8_t* uniformBuffer) const
	{
		assert(mDeclaration->mSize == mValues.size() * sizeof(int));
		memcpy(uniformBuffer + mDeclaration->mOffset, mValues.data(), mValues.size() * sizeof(int));
	}


	void UniformFloatArray::push(uint8_t* uniformBuffer) const
	{
		assert(mDeclaration->mSize == mValues.size() * sizeof(float));
		memcpy(uniformBuffer + mDeclaration->mOffset, mValues.data(), mValues.size() * sizeof(float));
	}


	void UniformVec3Array::push(uint8_t* uniformBuffer) const
	{
		assert(mDeclaration->mSize == mValues.size() * sizeof(glm::vec3));
		memcpy(uniformBuffer + mDeclaration->mOffset, mValues.data(), mValues.size() * sizeof(glm::vec3));
	}


	void UniformVec4Array::push(uint8_t* uniformBuffer) const
	{
		assert(mDeclaration->mSize == mValues.size() * sizeof(glm::vec4));
		memcpy(uniformBuffer + mDeclaration->mOffset, mValues.data(), mValues.size() * sizeof(glm::vec4));
	}


	void UniformMat4Array::push(uint8_t* uniformBuffer) const
	{
		assert(mDeclaration->mSize == mValues.size() * sizeof(glm::mat4));
		memcpy(uniformBuffer + mDeclaration->mOffset, mValues.data(), mValues.size() * sizeof(glm::mat4));
	}


} // End Namespace NAP
