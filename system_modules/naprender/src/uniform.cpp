/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "uniform.h"

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::Uniform, "Shader input variable")
	RTTI_PROPERTY("Name", &nap::Uniform::mName, nap::rtti::EPropertyMetaData::Default, "Name of the input variable")
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::UniformValue)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::UniformValueArray)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::UniformStruct)
	RTTI_PROPERTY(nap::uniform::uniforms, &nap::UniformStruct::mUniforms, nap::rtti::EPropertyMetaData::Embedded, "Input values")
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::UniformStructArray)
	RTTI_PROPERTY(nap::uniform::structs, &nap::UniformStructArray::mStructs, nap::rtti::EPropertyMetaData::Embedded, "Input structures")
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::UniformUInt)
	RTTI_PROPERTY(nap::uniform::value, &nap::UniformUInt::mValue, nap::rtti::EPropertyMetaData::Required, "Unsigned integer input value")
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::UniformIVec4)
	RTTI_PROPERTY(nap::uniform::value, &nap::UniformIVec4::mValue, nap::rtti::EPropertyMetaData::Required, "Vector (vec4) input value")
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::UniformUVec4)
	RTTI_PROPERTY(nap::uniform::value, &nap::UniformUVec4::mValue, nap::rtti::EPropertyMetaData::Required, "Vector (uvec4) input value")
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::UniformInt)
	RTTI_PROPERTY(nap::uniform::value, &nap::UniformInt::mValue, nap::rtti::EPropertyMetaData::Required, "Integer input value")
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::UniformFloat)
	RTTI_PROPERTY(nap::uniform::value, &nap::UniformFloat::mValue, nap::rtti::EPropertyMetaData::Required, "Float input value")
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::UniformVec2)
	RTTI_PROPERTY(nap::uniform::value, &nap::UniformVec2::mValue, nap::rtti::EPropertyMetaData::Required, "Vector (vec2) input value")
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::UniformVec3)
	RTTI_PROPERTY(nap::uniform::value, &nap::UniformVec3::mValue, nap::rtti::EPropertyMetaData::Required, "Vector (vec3) input value")
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::UniformVec4)
	RTTI_PROPERTY(nap::uniform::value, &nap::UniformVec4::mValue, nap::rtti::EPropertyMetaData::Required, "Vector (vec3) input value")
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::UniformMat4)
	RTTI_PROPERTY(nap::uniform::value, &nap::UniformMat4::mValue, nap::rtti::EPropertyMetaData::Required, "Matrix (4x4) shader input value")
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::UniformUIntArray)
	RTTI_PROPERTY(nap::uniform::values, &nap::UniformUIntArray::mValues, nap::rtti::EPropertyMetaData::Required, "Unsigned integer array input value")
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::UniformIntArray)
	RTTI_PROPERTY(nap::uniform::values, &nap::UniformIntArray::mValues, nap::rtti::EPropertyMetaData::Required, "Integer array input value")
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::UniformFloatArray)
	RTTI_PROPERTY(nap::uniform::values, &nap::UniformFloatArray::mValues, nap::rtti::EPropertyMetaData::Required, "Float array input value")
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::UniformVec2Array)
	RTTI_PROPERTY(nap::uniform::values, &nap::UniformVec2Array::mValues, nap::rtti::EPropertyMetaData::Required, "Vector (vec2) array input value")
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::UniformVec3Array)
	RTTI_PROPERTY(nap::uniform::values, &nap::UniformVec3Array::mValues, nap::rtti::EPropertyMetaData::Required, "Vector (vec3) array input value")
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::UniformVec4Array)
	RTTI_PROPERTY(nap::uniform::values, &nap::UniformVec4Array::mValues, nap::rtti::EPropertyMetaData::Required, "Vector (vec4) array input value")
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::UniformIVec4Array)
	RTTI_PROPERTY(nap::uniform::values, &nap::UniformIVec4Array::mValues, nap::rtti::EPropertyMetaData::Required, "Vector (ivec4) array input value")
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::UniformUVec4Array)
	RTTI_PROPERTY(nap::uniform::values, &nap::UniformUVec4Array::mValues, nap::rtti::EPropertyMetaData::Required, "Vector (uvec4) array input value")
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::UniformMat4Array)
	RTTI_PROPERTY(nap::uniform::values, &nap::UniformMat4Array::mValues, nap::rtti::EPropertyMetaData::Required, "Matrix (4x4) array input value")
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


	const Uniform* UniformStruct::findUniform(const std::string& name) const
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
