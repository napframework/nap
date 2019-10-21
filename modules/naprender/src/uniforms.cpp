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

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::UniformTexture)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::UniformTextureArray)
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

RTTI_BEGIN_CLASS(nap::UniformTexture2D)
	RTTI_PROPERTY("Texture", &nap::UniformTexture2D::mTexture, nap::rtti::EPropertyMetaData::Required)
	RTTI_FUNCTION("setTexture", &nap::UniformTexture2D::setTexture)
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

RTTI_BEGIN_CLASS(nap::UniformTexture2DArray)
	RTTI_PROPERTY("Textures", &nap::UniformTexture2DArray::mTextures, nap::rtti::EPropertyMetaData::Required)
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

	void UniformInt::push(uint8_t* uniformBuffer, const opengl::UniformDeclaration& declaration) const
	{
		assert(sizeof(mValue) == declaration.mSize);
		memcpy(uniformBuffer + declaration.mOffset, &mValue, sizeof(mValue));
	}


	void UniformFloat::push(uint8_t* uniformBuffer, const opengl::UniformDeclaration& declaration) const
	{
		assert(sizeof(mValue) == declaration.mSize);
		memcpy(uniformBuffer + declaration.mOffset, &mValue, sizeof(mValue));
	}


	void UniformVec3::push(uint8_t* uniformBuffer, const opengl::UniformDeclaration& declaration) const
	{
		assert(sizeof(mValue) == declaration.mSize);
		memcpy(uniformBuffer + declaration.mOffset, &mValue, sizeof(mValue));
	}


	void UniformVec4::push(uint8_t* uniformBuffer, const opengl::UniformDeclaration& declaration) const
	{
		assert(sizeof(mValue) == declaration.mSize);
		memcpy(uniformBuffer + declaration.mOffset, &mValue, sizeof(mValue));
	}


	void UniformMat4::push(uint8_t* uniformBuffer, const opengl::UniformDeclaration& declaration) const
	{
		assert(sizeof(mValue) == declaration.mSize);
		memcpy(uniformBuffer + declaration.mOffset, &mValue, sizeof(mValue));
	}


	int UniformTexture2D::push(uint8_t* uniformBuffer, const opengl::UniformDeclaration& declaration, int textureUnit) const
	{
		return 0;

// 		if (mTexture == nullptr)
// 			return 0;
// 
// 		glActiveTexture(GL_TEXTURE0 + textureUnit);
// 		mTexture->bind();
// 		glUniform1iv(declaration.mLocation, declaration.mSize, static_cast<const GLint*>(&textureUnit));
// 
// 		return 1;
	}

	//////////////////////////////////////////////////////////////////////////

	void UniformIntArray::push(uint8_t* uniformBuffer, const opengl::UniformDeclaration& declaration) const
	{
		assert(declaration.mSize == mValues.size() * sizeof(int));
		memcpy(uniformBuffer + declaration.mOffset, mValues.data(), mValues.size() * sizeof(int));
	}


	void UniformFloatArray::push(uint8_t* uniformBuffer, const opengl::UniformDeclaration& declaration) const
	{
		assert(declaration.mSize == mValues.size() * sizeof(float));
		memcpy(uniformBuffer + declaration.mOffset, mValues.data(), mValues.size() * sizeof(float));
	}


	void UniformVec3Array::push(uint8_t* uniformBuffer, const opengl::UniformDeclaration& declaration) const
	{
		assert(declaration.mSize == mValues.size() * sizeof(glm::vec3));
		memcpy(uniformBuffer + declaration.mOffset, mValues.data(), mValues.size() * sizeof(glm::vec3));
	}


	void UniformVec4Array::push(uint8_t* uniformBuffer, const opengl::UniformDeclaration& declaration) const
	{
		assert(declaration.mSize == mValues.size() * sizeof(glm::vec4));
		memcpy(uniformBuffer + declaration.mOffset, mValues.data(), mValues.size() * sizeof(glm::vec4));
	}


	void UniformMat4Array::push(uint8_t* uniformBuffer, const opengl::UniformDeclaration& declaration) const
	{
		assert(declaration.mSize == mValues.size() * sizeof(glm::mat4));
		memcpy(uniformBuffer + declaration.mOffset, mValues.data(), mValues.size() * sizeof(glm::mat4));
	}
	

	int UniformTexture2DArray::push(uint8_t* uniformBuffer, const opengl::UniformDeclaration& declaration, int textureUnit) const
	{
		return 0;
// 		int num_bound = 0;
// 		mTextureUnits.clear();
// 		mTextureUnits.reserve(mTextures.size());
// 		
// 		// Iterate over every user declared uniform texture in the array.
// 		// Bind it to the right texture unit and store list of used units
// 		// to upload later on.
// 		for (int index = 0; index < mTextures.size(); ++index)
// 		{
// 			if (mTextures[index] == nullptr)
// 				continue;
// 
// 			int unit = textureUnit + num_bound++;
// 			glActiveTexture(GL_TEXTURE0 + unit);
// 			mTextures[index]->bind();
// 			mTextureUnits.emplace_back(unit);
// 		}
// 
// 		// Upload list of used texture units.
// 		glUniform1iv(declaration.mLocation, mTextureUnits.size(), static_cast<const GLint*>(mTextureUnits.data()));
// 		return num_bound;
	}

} // End Namespace NAP
