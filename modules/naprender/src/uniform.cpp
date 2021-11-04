/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "uniform.h"

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::Uniform)
	RTTI_PROPERTY("Name", &nap::Uniform::mName, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::UniformValue)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::UniformValueArray)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::UniformValueBuffer)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::UniformStruct)
	RTTI_PROPERTY("Uniforms", &nap::UniformStruct::mUniforms, nap::rtti::EPropertyMetaData::Embedded)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::UniformStructArray)
	RTTI_PROPERTY("Structs", &nap::UniformStructArray::mStructs, nap::rtti::EPropertyMetaData::Embedded)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::UniformInt)
	RTTI_PROPERTY("Value", &nap::UniformInt::mValue, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::UniformFloat)
	RTTI_PROPERTY("Value", &nap::UniformFloat::mValue, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::UniformVec2)
	RTTI_PROPERTY("Value", &nap::UniformVec2::mValue, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::UniformVec3)
	RTTI_PROPERTY("Value", &nap::UniformVec3::mValue, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::UniformVec4)
	RTTI_PROPERTY("Value", &nap::UniformVec4::mValue, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::UniformMat4)
	RTTI_PROPERTY("Value", &nap::UniformMat4::mValue, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::UniformIntArray)
	RTTI_PROPERTY("Values", &nap::UniformIntArray::mValues, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::UniformFloatArray)
	RTTI_PROPERTY("Values", &nap::UniformFloatArray::mValues, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::UniformVec2Array)
	RTTI_PROPERTY("Values", &nap::UniformVec2Array::mValues, nap::rtti::EPropertyMetaData::Required)
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

RTTI_BEGIN_CLASS(nap::UniformIntBuffer)
	RTTI_PROPERTY("Buffer", &nap::UniformIntBuffer::mBuffer, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::UniformFloatBuffer)
	RTTI_PROPERTY("Buffer", &nap::UniformFloatBuffer::mBuffer, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::UniformVec2Buffer)
	RTTI_PROPERTY("Buffer", &nap::UniformVec2Buffer::mBuffer, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::UniformVec3Buffer)
	RTTI_PROPERTY("Buffer", &nap::UniformVec3Buffer::mBuffer, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::UniformVec4Buffer)
	RTTI_PROPERTY("Buffer", &nap::UniformVec4Buffer::mBuffer, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::UniformMat4Buffer)
	RTTI_PROPERTY("Buffer", &nap::UniformMat4Buffer::mBuffer, nap::rtti::EPropertyMetaData::Default)
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
}
