/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "bufferfillpolicy.h"
#include "uniform.h"

// External Includes
#include <nap/core.h>

RTTI_DEFINE_BASE(nap::BaseValueBufferFillPolicy)
RTTI_DEFINE_BASE(nap::BaseStructBufferFillPolicy)

RTTI_DEFINE_BASE(nap::IntBufferFillPolicy)
RTTI_DEFINE_BASE(nap::FloatBufferFillPolicy)
RTTI_DEFINE_BASE(nap::Vec2BufferFillPolicy)
RTTI_DEFINE_BASE(nap::Vec3BufferFillPolicy)
RTTI_DEFINE_BASE(nap::Vec4BufferFillPolicy)
RTTI_DEFINE_BASE(nap::Mat4BufferFillPolicy)


//////////////////////////////////////////////////////////////////////////
// UniformRandomBufferFillPolicy
//////////////////////////////////////////////////////////////////////////

RTTI_BEGIN_CLASS(nap::UniformRandomIntBufferFillPolicy)
	RTTI_PROPERTY("Lowerbound", &nap::UniformRandomIntBufferFillPolicy::mLowerBound, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("UpperBound", &nap::UniformRandomIntBufferFillPolicy::mUpperBound, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::UniformRandomFloatBufferFillPolicy)
	RTTI_PROPERTY("Lowerbound", &nap::UniformRandomFloatBufferFillPolicy::mLowerBound, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("UpperBound", &nap::UniformRandomFloatBufferFillPolicy::mUpperBound, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::UniformRandomIntBufferFillPolicy)
	RTTI_PROPERTY("Lowerbound", &nap::UniformRandomIntBufferFillPolicy::mLowerBound, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("UpperBound", &nap::UniformRandomIntBufferFillPolicy::mUpperBound, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::UniformRandomVec2BufferFillPolicy)
	RTTI_PROPERTY("Lowerbound", &nap::UniformRandomVec2BufferFillPolicy::mLowerBound, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("UpperBound", &nap::UniformRandomVec2BufferFillPolicy::mUpperBound, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::UniformRandomVec3BufferFillPolicy)
	RTTI_PROPERTY("Lowerbound", &nap::UniformRandomVec3BufferFillPolicy::mLowerBound, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("UpperBound", &nap::UniformRandomVec3BufferFillPolicy::mUpperBound, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::UniformRandomVec4BufferFillPolicy)
	RTTI_PROPERTY("Lowerbound", &nap::UniformRandomVec4BufferFillPolicy::mLowerBound, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("UpperBound", &nap::UniformRandomVec4BufferFillPolicy::mUpperBound, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::UniformRandomMat4BufferFillPolicy)
	RTTI_PROPERTY("Lowerbound", &nap::UniformRandomMat4BufferFillPolicy::mLowerBound, nap::rtti::EPropertyMetaData::Default)
	RTTI_PROPERTY("UpperBound", &nap::UniformRandomMat4BufferFillPolicy::mUpperBound, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS


//////////////////////////////////////////////////////////////////////////
// ConstantBufferFillPolicy
//////////////////////////////////////////////////////////////////////////

RTTI_BEGIN_CLASS(nap::ConstantIntBufferFillPolicy)
	RTTI_PROPERTY("Constant", &nap::ConstantIntBufferFillPolicy::mConstant, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::ConstantFloatBufferFillPolicy)
	RTTI_PROPERTY("Constant", &nap::ConstantFloatBufferFillPolicy::mConstant, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::ConstantIntBufferFillPolicy)
	RTTI_PROPERTY("Constant", &nap::ConstantIntBufferFillPolicy::mConstant, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::ConstantVec2BufferFillPolicy)
	RTTI_PROPERTY("Constant", &nap::ConstantVec2BufferFillPolicy::mConstant, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::ConstantVec3BufferFillPolicy)
	RTTI_PROPERTY("Constant", &nap::ConstantVec3BufferFillPolicy::mConstant, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::ConstantVec4BufferFillPolicy)
	RTTI_PROPERTY("Constant", &nap::ConstantVec4BufferFillPolicy::mConstant, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS

RTTI_BEGIN_CLASS(nap::ConstantMat4BufferFillPolicy)
	RTTI_PROPERTY("Constant", &nap::ConstantMat4BufferFillPolicy::mConstant, nap::rtti::EPropertyMetaData::Default)
RTTI_END_CLASS


//////////////////////////////////////////////////////////////////////////
// StructBufferFillPolicy
//////////////////////////////////////////////////////////////////////////

RTTI_BEGIN_CLASS(nap::UniformRandomStructBufferFillPolicy)
RTTI_END_CLASS


namespace nap
{
	/**
	 * Randomly initializes a data element from a ShaderVariableStructResource
	 * This function is unfinished but works with vec4 for now
	 */
	static int uniformRandomInitShaderVariableStructBufferRecursive(const ShaderVariableStructResource& uniformStruct, uint8* data, size_t offset = 0)
	{
		size_t size = 0;
		for (const auto& uniform : uniformStruct.mUniforms)
		{
			rtti::TypeInfo uniform_type = uniform->get_type();

			if (uniform_type.is_derived_from(RTTI_OF(UniformStructArray)))
			{
				UniformStructArray* uniform_resolved = rtti_cast<UniformStructArray>(uniform.get());
				if (!uniform_resolved->mStructs.empty())
				{
					int struct_element_size = uniformRandomInitShaderVariableStructBufferRecursive(*uniform_resolved->mStructs[0].get(), data, offset + size);
					size += struct_element_size * uniform_resolved->mStructs.size();
				}
			}
			else if (uniform_type.is_derived_from(RTTI_OF(UniformValueArray)))
			{
				UniformValueArray* uniform_resolved = rtti_cast<UniformValueArray>(uniform.get());

				if (uniform_type == RTTI_OF(TypedUniformValueArray<int>))
				{
					for (size_t idx = 0; idx < uniform_resolved->getCount(); idx++)
					{
						int rand = math::random(-1, 1);
						std::memcpy((uint8*)(data + offset + size), &rand, sizeof(int));
						size += sizeof(int);
					}
				}
				else if (uniform_type == RTTI_OF(TypedUniformValueArray<float>))
				{
					assert(false);
				}
				else if (uniform_type == RTTI_OF(TypedUniformValueArray<glm::vec2>))
				{
					assert(false);
				}
				else if (uniform_type == RTTI_OF(TypedUniformValueArray<glm::vec3>))
				{
					assert(false);
				}
				else if (uniform_type == RTTI_OF(TypedUniformValueArray<glm::vec4>))
				{
					for (size_t idx = 0; idx < uniform_resolved->getCount(); idx++)
					{
						glm::vec4 rand = math::random<glm::vec4>({ -1.0f, -1.0f, -1.0f, -1.0f }, { 1.0f, 1.0f, 1.0f, 1.0f });
						std::memcpy((uint8*)(data + offset + size), &rand, sizeof(glm::vec4));
						size += sizeof(glm::vec4);
					}
				}
				else if (uniform_type == RTTI_OF(TypedUniformValueArray<glm::mat4>))
				{
					assert(false);
				}
			}
			else if (uniform_type.is_derived_from(RTTI_OF(UniformStruct)))
			{
				UniformStruct* uniform_resolved = rtti_cast<UniformStruct>(uniform.get());
				size += uniformRandomInitShaderVariableStructBufferRecursive(*uniform_resolved, data, offset + size);
			}
			else if (uniform_type.is_derived_from(RTTI_OF(UniformValue)))
			{
				if (uniform_type == RTTI_OF(TypedUniformValue<int>))
				{
					int rand = math::random(-1, 1);
					std::memcpy((uint8*)(data + offset + size), &rand, sizeof(int));
					size += sizeof(int);
				}
				else if (uniform_type == RTTI_OF(TypedUniformValue<float>))
				{
					float rand = math::random(-1.0f, 1.0f);
					std::memcpy((uint8*)(data + offset + size), &rand, sizeof(float));
					size += sizeof(float);
				}
				else if (uniform_type == RTTI_OF(TypedUniformValue<glm::vec2>))
				{
					glm::vec2 rand = math::random<glm::vec2>({ -1.0f, -1.0f }, { 1.0f, 1.0f });
					std::memcpy((uint8*)(data + offset + size), &rand, sizeof(glm::vec2));
					size += sizeof(glm::vec2);
				}
				else if (uniform_type == RTTI_OF(TypedUniformValue<glm::vec3>))
				{
					glm::vec3 rand = math::random<glm::vec3>({-1.0f, -1.0f, -1.0f}, {1.0f, 1.0f, 1.0f});
					std::memcpy((uint8*)(data + offset + size), &rand, sizeof(glm::vec3));
					size += sizeof(glm::vec3);
				}
				else if (uniform_type == RTTI_OF(TypedUniformValue<glm::vec4>))
				{
					glm::vec4 rand = math::random<glm::vec4>({ -1.0f, -1.0f, -1.0f, -1.0f }, { 1.0f, 1.0f, 1.0f, 1.0f });
					std::memcpy((uint8*)(data + offset + size), &rand, sizeof(glm::vec4));
					size += sizeof(glm::vec4);
				}
				else if (uniform_type == RTTI_OF(TypedUniformValue<glm::mat4>))
				{
					// Uh oh
					glm::mat4 rand = math::random<glm::mat4>(
						{ -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f },
						{ 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f }
					);
					std::memcpy((uint8*)(data + offset + size), &rand, sizeof(glm::mat4));
					size += sizeof(glm::mat4);
				}
			}
		}
		return size;
	}


	bool UniformRandomStructBufferFillPolicy::fill(const StructBufferDescriptor& descriptor, uint8* data, utility::ErrorState& errorState)
	{
		// Still poor performance on large buffers
		size_t element_size = 0;
		for (size_t i = 0; i < descriptor.mCount; i++)
			element_size = uniformRandomInitShaderVariableStructBufferRecursive(*descriptor.mElement, data + i*element_size);

		return true;
	}
}
