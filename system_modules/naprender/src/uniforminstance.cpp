/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "uniforminstance.h"

RTTI_DEFINE_BASE(nap::UniformInstance)
RTTI_DEFINE_BASE(nap::UniformLeafInstance)
RTTI_DEFINE_BASE(nap::UniformValueInstance)
RTTI_DEFINE_BASE(nap::UniformValueArrayInstance)

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::UniformStructInstance)
	RTTI_CONSTRUCTOR(const nap::ShaderVariableStructDeclaration&, const nap::UniformCreatedCallback&)
	RTTI_FUNCTION("findUniform", (nap::UniformInstance* (nap::UniformStructInstance::*)(const std::string&)) &nap::UniformStructInstance::findUniform)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::UniformStructArrayInstance)
	RTTI_CONSTRUCTOR(const nap::ShaderVariableStructArrayDeclaration&)
	RTTI_FUNCTION("findElement", (nap::UniformStructInstance* (nap::UniformStructArrayInstance::*)(int)) &nap::UniformStructArrayInstance::findElement)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::UniformUIntInstance)
	RTTI_CONSTRUCTOR(const nap::ShaderVariableValueDeclaration&)
	RTTI_FUNCTION("setValue", &nap::UniformUIntInstance::setValue)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::UniformIntInstance)
	RTTI_CONSTRUCTOR(const nap::ShaderVariableValueDeclaration&)
	RTTI_FUNCTION("setValue", &nap::UniformIntInstance::setValue)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::UniformFloatInstance)
	RTTI_CONSTRUCTOR(const nap::ShaderVariableValueDeclaration&)
	RTTI_FUNCTION("setValue", &nap::UniformFloatInstance::setValue)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::UniformVec2Instance)
	RTTI_CONSTRUCTOR(const nap::ShaderVariableValueDeclaration&)
	RTTI_FUNCTION("setValue", &nap::UniformVec2Instance::setValue)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::UniformVec3Instance)
	RTTI_CONSTRUCTOR(const nap::ShaderVariableValueDeclaration&)
	RTTI_FUNCTION("setValue", &nap::UniformVec3Instance::setValue)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::UniformVec4Instance)
	RTTI_CONSTRUCTOR(const nap::ShaderVariableValueDeclaration&)
	RTTI_FUNCTION("setValue", &nap::UniformVec4Instance::setValue)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::UniformIVec4Instance)
	RTTI_CONSTRUCTOR(const nap::ShaderVariableValueDeclaration&)
	RTTI_FUNCTION("setValue", &nap::UniformIVec4Instance::setValue)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::UniformUVec4Instance)
	RTTI_CONSTRUCTOR(const nap::ShaderVariableValueDeclaration&)
	RTTI_FUNCTION("setValue", &nap::UniformUVec4Instance::setValue)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::UniformMat4Instance)
	RTTI_CONSTRUCTOR(const nap::ShaderVariableValueDeclaration&)
	RTTI_FUNCTION("setValue", &nap::UniformMat4Instance::setValue)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::UniformUIntArrayInstance)
	RTTI_CONSTRUCTOR(const nap::ShaderVariableValueArrayDeclaration&)
	RTTI_FUNCTION("setValue", &nap::UniformUIntArrayInstance::setValue)
	RTTI_FUNCTION("getNumElements", &nap::UniformUIntArrayInstance::getNumElements)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::UniformIntArrayInstance)
	RTTI_CONSTRUCTOR(const nap::ShaderVariableValueArrayDeclaration&)
	RTTI_FUNCTION("setValue", &nap::UniformIntArrayInstance::setValue)
	RTTI_FUNCTION("getNumElements", &nap::UniformIntArrayInstance::getNumElements)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::UniformFloatArrayInstance)
	RTTI_CONSTRUCTOR(const nap::ShaderVariableValueArrayDeclaration&)
	RTTI_FUNCTION("setValue", &nap::UniformFloatArrayInstance::setValue)
	RTTI_FUNCTION("getNumElements", &nap::UniformFloatArrayInstance::getNumElements)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::UniformVec2ArrayInstance)
	RTTI_CONSTRUCTOR(const nap::ShaderVariableValueArrayDeclaration&)
	RTTI_FUNCTION("setValue", &nap::UniformVec2ArrayInstance::setValue)
	RTTI_FUNCTION("getNumElements", &nap::UniformVec2ArrayInstance::getNumElements)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::UniformVec3ArrayInstance)
	RTTI_CONSTRUCTOR(const nap::ShaderVariableValueArrayDeclaration&)
	RTTI_FUNCTION("setValue", &nap::UniformVec3ArrayInstance::setValue)
	RTTI_FUNCTION("getNumElements", &nap::UniformVec3ArrayInstance::getNumElements)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::UniformVec4ArrayInstance)
	RTTI_CONSTRUCTOR(const nap::ShaderVariableValueArrayDeclaration&)
	RTTI_FUNCTION("setValue", &nap::UniformVec4ArrayInstance::setValue)
	RTTI_FUNCTION("getNumElements", &nap::UniformVec4ArrayInstance::getNumElements)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::UniformIVec4ArrayInstance)
	RTTI_CONSTRUCTOR(const nap::ShaderVariableValueArrayDeclaration&)
	RTTI_FUNCTION("setValue", &nap::UniformIVec4ArrayInstance::setValue)
	RTTI_FUNCTION("getNumElements", &nap::UniformIVec4ArrayInstance::getNumElements)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::UniformUVec4ArrayInstance)
	RTTI_CONSTRUCTOR(const nap::ShaderVariableValueArrayDeclaration&)
	RTTI_FUNCTION("setValue", &nap::UniformUVec4ArrayInstance::setValue)
	RTTI_FUNCTION("getNumElements", &nap::UniformUVec4ArrayInstance::getNumElements)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::UniformMat4ArrayInstance)
	RTTI_CONSTRUCTOR(const nap::ShaderVariableValueArrayDeclaration&)
	RTTI_FUNCTION("setValue", &nap::UniformMat4ArrayInstance::setValue)
	RTTI_FUNCTION("getNumElements", &nap::UniformMat4ArrayInstance::getNumElements)
RTTI_END_CLASS


namespace nap
{
	template<typename INSTANCE_TYPE, typename RESOURCE_TYPE, typename DECLARATION_TYPE>
	static std::unique_ptr<INSTANCE_TYPE> createUniformValueInstance(const Uniform* value, const DECLARATION_TYPE& declaration, utility::ErrorState& errorState)
	{
		auto result = std::make_unique<INSTANCE_TYPE>(declaration);
		if (value != nullptr)
		{
			const auto* typed_resource = rtti_cast<const RESOURCE_TYPE>(value);
			if (!errorState.check(typed_resource != nullptr, "Encountered type mismatch between uniform in material and uniform in shader (%s)", declaration.mName.c_str()))
				return nullptr;

			result->set(*typed_resource);
		}
		return result;
	}


	//////////////////////////////////////////////////////////////////////////
	// UniformStructInstance
	//////////////////////////////////////////////////////////////////////////

	std::unique_ptr<UniformInstance> UniformStructInstance::createUniformFromDeclaration(const ShaderVariableDeclaration& declaration, const UniformCreatedCallback& uniformCreatedCallback)
	{
		rtti::TypeInfo declaration_type = declaration.get_type();

		if (declaration_type == RTTI_OF(ShaderVariableStructArrayDeclaration))
		{
			const ShaderVariableStructArrayDeclaration* struct_array_declaration = rtti_cast<const ShaderVariableStructArrayDeclaration>(&declaration);
			std::unique_ptr<UniformStructArrayInstance> struct_array_instance = std::make_unique<UniformStructArrayInstance>(*struct_array_declaration);
			for (auto& struct_declaration : struct_array_declaration->mElements)
			{
				std::unique_ptr<UniformStructInstance> struct_instance = std::make_unique<UniformStructInstance>(*struct_declaration, uniformCreatedCallback);
				struct_array_instance->addElement(std::move(struct_instance));
			}
			return std::move(struct_array_instance);
		}
		else if (declaration_type == RTTI_OF(ShaderVariableValueArrayDeclaration))
		{
			const ShaderVariableValueArrayDeclaration* value_array_declaration = rtti_cast<const ShaderVariableValueArrayDeclaration>(&declaration);

			if (value_array_declaration->mElementType == EShaderVariableValueType::UInt)
			{
				auto array_instance = std::make_unique<UniformUIntArrayInstance>(*value_array_declaration);
				array_instance->getValues().resize(value_array_declaration->mNumElements);
				return std::move(array_instance);
			}
			else if (value_array_declaration->mElementType == EShaderVariableValueType::Int)
			{
				auto array_instance = std::make_unique<UniformIntArrayInstance>(*value_array_declaration);
				array_instance->getValues().resize(value_array_declaration->mNumElements);
				return std::move(array_instance);
			}
			else if (value_array_declaration->mElementType == EShaderVariableValueType::Float)
			{
				auto array_instance = std::make_unique<UniformFloatArrayInstance>(*value_array_declaration);
				array_instance->getValues().resize(value_array_declaration->mNumElements);
				return std::move(array_instance);
			}
			else if (value_array_declaration->mElementType == EShaderVariableValueType::Vec2)
			{
				auto array_instance = std::make_unique<UniformVec2ArrayInstance>(*value_array_declaration);
				array_instance->getValues().resize(value_array_declaration->mNumElements);
				return std::move(array_instance);
			}
			else if (value_array_declaration->mElementType == EShaderVariableValueType::Vec3)
			{
				auto array_instance = std::make_unique<UniformVec3ArrayInstance>(*value_array_declaration);
				array_instance->getValues().resize(value_array_declaration->mNumElements);
				return std::move(array_instance);
			}
			else if (value_array_declaration->mElementType == EShaderVariableValueType::Vec4)
			{
				auto array_instance = std::make_unique<UniformVec4ArrayInstance>(*value_array_declaration);
				array_instance->getValues().resize(value_array_declaration->mNumElements);
				return std::move(array_instance);
			}
			else if (value_array_declaration->mElementType == EShaderVariableValueType::IVec4)
			{
				auto array_instance = std::make_unique<UniformIVec4ArrayInstance>(*value_array_declaration);
				array_instance->getValues().resize(value_array_declaration->mNumElements);
				return std::move(array_instance);
			}
			else if (value_array_declaration->mElementType == EShaderVariableValueType::UVec4)
			{
				auto array_instance = std::make_unique<UniformUVec4ArrayInstance>(*value_array_declaration);
				array_instance->getValues().resize(value_array_declaration->mNumElements);
				return std::move(array_instance);
			}
			else if (value_array_declaration->mElementType == EShaderVariableValueType::Mat4)
			{
				auto array_instance = std::make_unique<UniformMat4ArrayInstance>(*value_array_declaration);
				array_instance->getValues().resize(value_array_declaration->mNumElements);
				return std::move(array_instance);
			}
		}
		else if (declaration_type == RTTI_OF(ShaderVariableStructDeclaration))
		{
			const ShaderVariableStructDeclaration* struct_declaration = rtti_cast<const ShaderVariableStructDeclaration>(&declaration);
			return std::make_unique<UniformStructInstance>(*struct_declaration, uniformCreatedCallback);
		}
		else if (declaration_type == RTTI_OF(ShaderVariableValueDeclaration))
		{
			const ShaderVariableValueDeclaration* value_declaration = rtti_cast<const ShaderVariableValueDeclaration>(&declaration);

			if (value_declaration->mType == EShaderVariableValueType::UInt)
			{
				return std::make_unique<UniformUIntInstance>(*value_declaration);
			}
			else if (value_declaration->mType == EShaderVariableValueType::Int)
			{
				return std::make_unique<UniformIntInstance>(*value_declaration);
			}
			else if (value_declaration->mType == EShaderVariableValueType::Float)
			{
				return std::make_unique<UniformFloatInstance>(*value_declaration);
			}
			else if (value_declaration->mType == EShaderVariableValueType::Vec2)
			{
				return std::make_unique<UniformVec2Instance>(*value_declaration);
			}
			else if (value_declaration->mType == EShaderVariableValueType::Vec3)
			{
				return std::make_unique<UniformVec3Instance>(*value_declaration);
			}
			else if (value_declaration->mType == EShaderVariableValueType::Vec4)
			{
				return std::make_unique<UniformVec4Instance>(*value_declaration);
			}
			else if (value_declaration->mType == EShaderVariableValueType::IVec4)
			{
				return std::make_unique<UniformIVec4Instance>(*value_declaration);
			}
			else if (value_declaration->mType == EShaderVariableValueType::UVec4)
			{
				return std::make_unique<UniformUVec4Instance>(*value_declaration);
			}
			else if (value_declaration->mType == EShaderVariableValueType::Mat4)
			{
				return std::make_unique<UniformMat4Instance>(*value_declaration);
			}
		}
		else
		{
			// Possibly UniformStructBufferDeclaration - which is supported for BufferBindings only
			assert(false);
		}

		return nullptr;
	}


	nap::UniformInstance* UniformStructInstance::findUniform(const std::string& name)
	{
		for (auto& uniform_instance : mUniforms)
		{
			if (uniform_instance->getDeclaration().mName == name)
				return uniform_instance.get();
		}
		return nullptr;
	}


	bool UniformStructInstance::addUniformRecursive(const ShaderVariableStructDeclaration& structDeclaration, const UniformStruct* structResource, const UniformCreatedCallback& uniformCreatedCallback, bool createDefaults, utility::ErrorState& errorState)
	{
		for (auto& uniform_declaration : structDeclaration.mMembers)
		{
			rtti::TypeInfo declaration_type = uniform_declaration->get_type();

			const Uniform* resource = nullptr;
			if (structResource != nullptr)
				resource = findUniformStructMember(structResource->mUniforms, *uniform_declaration);

			if (!createDefaults && resource == nullptr)
				continue;

			if (declaration_type == RTTI_OF(ShaderVariableStructArrayDeclaration))
			{
				ShaderVariableStructArrayDeclaration* struct_array_declaration = rtti_cast<ShaderVariableStructArrayDeclaration>(uniform_declaration.get());

				std::unique_ptr<UniformStructArrayInstance> struct_array_instance = std::make_unique<UniformStructArrayInstance>(*struct_array_declaration);
				const UniformStructArray* struct_array_resource = rtti_cast<const UniformStructArray>(resource);
				if (!errorState.check(resource == nullptr || struct_array_resource->mStructs.size() == struct_array_declaration->mElements.size(), "Mismatch between number of array elements in shader and json."))
					return false;

				int resource_index = 0;
				for (auto& declaration_element : struct_array_declaration->mElements)
				{
					UniformStruct* struct_resource = nullptr;
					if (struct_array_resource != nullptr && resource_index < struct_array_resource->mStructs.size())
						struct_resource = struct_array_resource->mStructs[resource_index++].get();
					std::unique_ptr<UniformStructInstance> instance_element = std::make_unique<UniformStructInstance>(*declaration_element, uniformCreatedCallback);
					if (!instance_element->addUniformRecursive(*declaration_element, struct_resource, uniformCreatedCallback, createDefaults, errorState))
						return false;
					struct_array_instance->addElement(std::move(instance_element));
				}

				mUniforms.emplace_back(std::move(struct_array_instance));
			}
			else if (declaration_type == RTTI_OF(ShaderVariableValueArrayDeclaration))
			{
				const UniformValueArray* value_array_resource = rtti_cast<const UniformValueArray>(resource);
				if (!errorState.check(resource == nullptr || value_array_resource != nullptr, "Type mismatch between shader type and json type"))
					return false;

				ShaderVariableValueArrayDeclaration* value_declaration = rtti_cast<ShaderVariableValueArrayDeclaration>(uniform_declaration.get());
				std::unique_ptr<UniformValueArrayInstance> instance_value_array;

				if (value_declaration->mElementType == EShaderVariableValueType::UInt)
				{
					instance_value_array = createUniformValueInstance<UniformUIntArrayInstance, UniformUIntArray>(resource, *value_declaration, errorState);
				}
				else if (value_declaration->mElementType == EShaderVariableValueType::Int)
				{
					instance_value_array = createUniformValueInstance<UniformIntArrayInstance, UniformIntArray>(resource, *value_declaration, errorState);
				}
				else if (value_declaration->mElementType == EShaderVariableValueType::Float)
				{
					instance_value_array = createUniformValueInstance<UniformFloatArrayInstance, UniformFloatArray>(resource, *value_declaration, errorState);
				}
				else if (value_declaration->mElementType == EShaderVariableValueType::Vec2)
				{
					instance_value_array = createUniformValueInstance<UniformVec2ArrayInstance, UniformVec2Array>(resource, *value_declaration, errorState);
				}
				else if (value_declaration->mElementType == EShaderVariableValueType::Vec3)
				{
					instance_value_array = createUniformValueInstance<UniformVec3ArrayInstance, UniformVec3Array>(resource, *value_declaration, errorState);
				}
				else if (value_declaration->mElementType == EShaderVariableValueType::Vec4)
				{
					instance_value_array = createUniformValueInstance<UniformVec4ArrayInstance, UniformVec4Array>(resource, *value_declaration, errorState);
				}
				else if (value_declaration->mElementType == EShaderVariableValueType::IVec4)
				{
					instance_value_array = createUniformValueInstance<UniformIVec4ArrayInstance, UniformIVec4Array>(resource, *value_declaration, errorState);
				}
				else if (value_declaration->mElementType == EShaderVariableValueType::UVec4)
				{
					instance_value_array = createUniformValueInstance<UniformUVec4ArrayInstance, UniformUVec4Array>(resource, *value_declaration, errorState);
				}
				else if (value_declaration->mElementType == EShaderVariableValueType::Mat4)
				{
					instance_value_array = createUniformValueInstance<UniformMat4ArrayInstance, UniformMat4Array>(resource, *value_declaration, errorState);
				}
				else
				{
					errorState.fail("Data type of shader variable %s is not supported", uniform_declaration->mName.c_str());
					return false;
				}

				if (instance_value_array == nullptr)
					return false;

				// If the array was not set in json, we need to ensure the array has the correct size & is filled with default values
				if (resource == nullptr)
					instance_value_array->setDefault();

				if (!errorState.check(resource == nullptr || value_array_resource->getCount() == value_declaration->mNumElements, "Encountered mismatch in array elements between array in material and array in shader"))
					return false;

				mUniforms.emplace_back(std::move(instance_value_array));
			}
			else if (declaration_type == RTTI_OF(ShaderVariableStructDeclaration))
			{
				const UniformStruct* struct_resource = rtti_cast<const UniformStruct>(resource);

				ShaderVariableStructDeclaration* struct_declaration = rtti_cast<ShaderVariableStructDeclaration>(uniform_declaration.get());
				std::unique_ptr<UniformStructInstance> struct_instance = std::make_unique<UniformStructInstance>(*struct_declaration, uniformCreatedCallback);
				if (!struct_instance->addUniformRecursive(*struct_declaration, struct_resource, uniformCreatedCallback, createDefaults, errorState))
					return false;

				mUniforms.emplace_back(std::move(struct_instance));
			}
			else if(declaration_type == RTTI_OF(ShaderVariableValueDeclaration))
			{
				ShaderVariableValueDeclaration* value_declaration = rtti_cast<ShaderVariableValueDeclaration>(uniform_declaration.get());
				std::unique_ptr<UniformValueInstance> value_instance;

				if (value_declaration->mType == EShaderVariableValueType::UInt)
				{
					value_instance = createUniformValueInstance<UniformUIntInstance, UniformUInt>(resource, *value_declaration, errorState);
				}
				else if (value_declaration->mType == EShaderVariableValueType::Int)
				{
					value_instance = createUniformValueInstance<UniformIntInstance, UniformInt>(resource, *value_declaration, errorState);
				}
				else if (value_declaration->mType == EShaderVariableValueType::Float)
				{
					value_instance = createUniformValueInstance<UniformFloatInstance, UniformFloat>(resource, *value_declaration, errorState);
				}
				else if (value_declaration->mType == EShaderVariableValueType::Vec2)
				{
					value_instance = createUniformValueInstance<UniformVec2Instance, UniformVec2>(resource, *value_declaration, errorState);
				}
				else if (value_declaration->mType == EShaderVariableValueType::Vec3)
				{
					value_instance = createUniformValueInstance<UniformVec3Instance, UniformVec3>(resource, *value_declaration, errorState);
				}
				else if (value_declaration->mType == EShaderVariableValueType::Vec4)
				{
					value_instance = createUniformValueInstance<UniformVec4Instance, UniformVec4>(resource, *value_declaration, errorState);
				}
				else if (value_declaration->mType == EShaderVariableValueType::IVec4)
				{
					value_instance = createUniformValueInstance<UniformIVec4Instance, UniformIVec4>(resource, *value_declaration, errorState);
				}
				else if (value_declaration->mType == EShaderVariableValueType::UVec4)
				{
					value_instance = createUniformValueInstance<UniformUVec4Instance, UniformUVec4>(resource, *value_declaration, errorState);
				}
				else if (value_declaration->mType == EShaderVariableValueType::Mat4)
				{
					value_instance = createUniformValueInstance<UniformMat4Instance, UniformMat4>(resource, *value_declaration, errorState);
				}
				else
				{
					errorState.fail("Data type of shader variable %s is not supported", uniform_declaration->mName.c_str());
					return false;
				}

				if (value_instance == nullptr)
					return false;

				mUniforms.emplace_back(std::move(value_instance));
			}
			else
			{
				// Possibly UniformStructBufferDeclaration - which is supported for BufferBindings only
				assert(false);
			}
		}
		return true;
	}


	nap::UniformStructInstance* UniformStructArrayInstance::findElement(int index)
	{
		return index >= mElements.size() ? nullptr : mElements[index].get();
	}


	//////////////////////////////////////////////////////////////////////////
	// UniformStructArrayInstance
	//////////////////////////////////////////////////////////////////////////

	void UniformStructArrayInstance::addElement(std::unique_ptr<UniformStructInstance> element)
	{
		mElements.emplace_back(std::move(element));
	}
}
