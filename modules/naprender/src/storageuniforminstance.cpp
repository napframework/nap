/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "storageuniforminstance.h"

RTTI_DEFINE_BASE(nap::StorageUniformInstance)
RTTI_DEFINE_BASE(nap::StorageUniformBufferInstance)
RTTI_DEFINE_BASE(nap::StorageUniformValueBufferInstance)

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::StorageUniformStructInstance)
	RTTI_CONSTRUCTOR(const nap::ShaderVariableStructDeclaration&, const nap::StorageUniformChangedCallback&)
	RTTI_FUNCTION("findStorageUniform", (nap::StorageUniformInstance* (nap::StorageUniformStructInstance::*)(const std::string&)) &nap::StorageUniformStructInstance::findStorageUniform)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::StorageUniformStructBufferInstance)
	RTTI_CONSTRUCTOR(const nap::ShaderVariableStructArrayDeclaration&)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::StorageUniformIntBufferInstance)
	RTTI_CONSTRUCTOR(const nap::UniformValueArrayDeclaration&)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::StorageUniformFloatBufferInstance)
	RTTI_CONSTRUCTOR(const nap::UniformValueArrayDeclaration&)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::StorageUniformVec2BufferInstance)
	RTTI_CONSTRUCTOR(const nap::UniformValueArrayDeclaration&)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::StorageUniformVec3BufferInstance)
	RTTI_CONSTRUCTOR(const nap::UniformValueArrayDeclaration&)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::StorageUniformVec4BufferInstance)
	RTTI_CONSTRUCTOR(const nap::UniformValueArrayDeclaration&)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::StorageUniformMat4BufferInstance)
	RTTI_CONSTRUCTOR(const nap::UniformValueArrayDeclaration&)
RTTI_END_CLASS


namespace nap
{
	template<typename INSTANCE_TYPE, typename RESOURCE_TYPE, typename DECLARATION_TYPE>
	static std::unique_ptr<INSTANCE_TYPE> createUniformValueInstance(const StorageUniform* value, const DECLARATION_TYPE& declaration, utility::ErrorState& errorState)
	{
		std::unique_ptr<INSTANCE_TYPE> result = std::make_unique<INSTANCE_TYPE>(declaration);
		if (value != nullptr)
		{
			const RESOURCE_TYPE* typed_resource = rtti_cast<const RESOURCE_TYPE>(value);
			if (!errorState.check(typed_resource != nullptr, "Encountered type mismatch between uniform in material and uniform in shader"))
				return nullptr;

			result->set(*typed_resource);
		}
		return result;
	}


	//////////////////////////////////////////////////////////////////////////
	// UniformStructInstance
	//////////////////////////////////////////////////////////////////////////

	std::unique_ptr<StorageUniformInstance> StorageUniformStructInstance::createStorageUniformFromDeclaration(const ShaderVariableDeclaration& declaration, const StorageUniformCreatedCallback& uniformCreatedCallback)
	{
		rtti::TypeInfo declaration_type = declaration.get_type();

		if (declaration_type == RTTI_OF(ShaderVariableStructArrayDeclaration))
		{
			const ShaderVariableStructArrayDeclaration* struct_array_declaration = rtti_cast<const ShaderVariableStructArrayDeclaration>(&declaration);
			std::unique_ptr<StorageUniformStructBufferInstance> buffer_instance = std::make_unique<StorageUniformStructBufferInstance>(*struct_array_declaration);
			for (auto& struct_declaration : struct_array_declaration->mElements)
			{
				//std::unique_ptr<UniformStructInstance> struct_instance = std::make_unique<UniformStructInstance>(*struct_declaration, uniformCreatedCallback);
				//struct_array_instance->addElement(std::move(struct_instance));
			}
			return std::move(buffer_instance);
		}
		else if (declaration_type == RTTI_OF(ShaderVariableValueArrayDeclaration))
		{
			const ShaderVariableValueArrayDeclaration* value_array_declaration = rtti_cast<const ShaderVariableValueArrayDeclaration>(&declaration);

			if (value_array_declaration->mElementType == EShaderVariableValueType::Int)
			{
				std::unique_ptr<StorageUniformIntBufferInstance> buffer_instance = std::make_unique<StorageUniformIntBufferInstance>(*value_array_declaration);
				return std::move(buffer_instance);
			}
			else if (value_array_declaration->mElementType == EShaderVariableValueType::Float)
			{
				std::unique_ptr<StorageUniformFloatBufferInstance> buffer_instance = std::make_unique<StorageUniformFloatBufferInstance>(*value_array_declaration);
				return std::move(buffer_instance);
			}
			else if (value_array_declaration->mElementType == EShaderVariableValueType::Vec2)
			{
				std::unique_ptr<StorageUniformVec2BufferInstance> buffer_instance = std::make_unique<StorageUniformVec2BufferInstance>(*value_array_declaration);
				return std::move(buffer_instance);
			}
			else if (value_array_declaration->mElementType == EShaderVariableValueType::Vec3)
			{
				std::unique_ptr<StorageUniformVec3BufferInstance> buffer_instance = std::make_unique<StorageUniformVec3BufferInstance>(*value_array_declaration);
				return std::move(buffer_instance);
			}
			else if (value_array_declaration->mElementType == EShaderVariableValueType::Vec4)
			{
				std::unique_ptr<StorageUniformVec4BufferInstance> buffer_instance = std::make_unique<StorageUniformVec4BufferInstance>(*value_array_declaration);
				return std::move(buffer_instance);
			}
			else if (value_array_declaration->mElementType == EShaderVariableValueType::Mat4)
			{
				std::unique_ptr<StorageUniformMat4BufferInstance> buffer_instance = std::make_unique<StorageUniformMat4BufferInstance>(*value_array_declaration);
				return std::move(buffer_instance);
			}
		}
		else if (declaration_type == RTTI_OF(ShaderVariableStructDeclaration))
		{
			const ShaderVariableStructDeclaration* struct_declaration = rtti_cast<const ShaderVariableStructDeclaration>(&declaration);

			// Structs not supported
			assert(false);
		}
		else
		{
			const ShaderVariableValueDeclaration* value_declaration = rtti_cast<const ShaderVariableValueDeclaration>(&declaration);

			// Values not supported
			assert(false);
		}

		return nullptr;
	}


	nap::StorageUniformInstance* StorageUniformStructInstance::findStorageUniform(const std::string& name)
	{
		for (auto& uniform_instance : mStorageUniforms)
		{
			if (uniform_instance->getDeclaration().mName == name)
				return uniform_instance.get();
		}
		return nullptr;
	}


	bool StorageUniformStructInstance::addStorageUniform(const ShaderVariableStructDeclaration& structDeclaration, const StorageUniformStruct* structResource, const StorageUniformChangedCallback& uniformChangedCallback, utility::ErrorState& errorState)
	{
		for (auto& uniform_declaration : structDeclaration.mMembers)
		{
			rtti::TypeInfo declaration_type = uniform_declaration->get_type();

			const StorageUniform* resource = nullptr;
			if (structResource != nullptr && structResource->mStorageUniformBuffer != nullptr && structResource->mStorageUniformBuffer->mName == uniform_declaration->mName)
				resource = structResource->mStorageUniformBuffer.get();

			if (declaration_type == RTTI_OF(ShaderVariableStructArrayDeclaration))
			{
				ShaderVariableStructArrayDeclaration* struct_array_declaration = rtti_cast<ShaderVariableStructArrayDeclaration>(uniform_declaration.get());
				const StorageUniformStructBuffer* struct_buffer_resource = rtti_cast<const StorageUniformStructBuffer>(resource);

				// Ensure buffer size in uniform and shader declaration are the same
				if (!errorState.check(struct_buffer_resource == nullptr || struct_buffer_resource->getSize() == struct_array_declaration->mSize, "Mismatch between buffer size in shader and json"))
					return false;

				std::unique_ptr<StorageUniformStructBufferInstance> struct_buffer_instance = createUniformValueInstance<StorageUniformStructBufferInstance, StorageUniformStructBuffer>(resource, *struct_array_declaration, errorState);

				mStorageUniforms.emplace_back(std::move(struct_buffer_instance));
			}
			else if (declaration_type == RTTI_OF(ShaderVariableValueArrayDeclaration))
			{
				ShaderVariableValueArrayDeclaration* value_declaration = rtti_cast<ShaderVariableValueArrayDeclaration>(uniform_declaration.get());
				std::unique_ptr<StorageUniformValueBufferInstance> instance_value_buffer;

				const StorageUniformValueBuffer* value_buffer_resource = rtti_cast<const StorageUniformValueBuffer>(resource);
				if (!errorState.check(resource == nullptr || value_buffer_resource != nullptr, "Type mismatch between shader type and json type"))
					return false;

				if (value_declaration->mElementType == EShaderVariableValueType::Int)
				{
					instance_value_buffer = createUniformValueInstance<StorageUniformIntBufferInstance, StorageUniformIntBuffer>(resource, *value_declaration, errorState);
				}
				else if (value_declaration->mElementType == EShaderVariableValueType::Float)
				{
					instance_value_buffer = createUniformValueInstance<StorageUniformFloatBufferInstance, StorageUniformFloatBuffer>(resource, *value_declaration, errorState);
				}
				else if (value_declaration->mElementType == EShaderVariableValueType::Vec2)
				{
					instance_value_buffer = createUniformValueInstance<StorageUniformVec2BufferInstance, StorageUniformVec2Buffer>(resource, *value_declaration, errorState);
				}
				else if (value_declaration->mElementType == EShaderVariableValueType::Vec3)
				{
					instance_value_buffer = createUniformValueInstance<StorageUniformVec3BufferInstance, StorageUniformVec3Buffer>(resource, *value_declaration, errorState);
				}
				else if (value_declaration->mElementType == EShaderVariableValueType::Vec4)
				{
					instance_value_buffer = createUniformValueInstance<StorageUniformVec4BufferInstance, StorageUniformVec4Buffer>(resource, *value_declaration, errorState);
				}
				else if (value_declaration->mElementType == EShaderVariableValueType::Mat4)
				{
					instance_value_buffer = createUniformValueInstance<StorageUniformMat4BufferInstance, StorageUniformMat4Buffer>(resource, *value_declaration, errorState);
				}
				else
				{
					assert(false);
				}

				if (instance_value_buffer == nullptr)
					return false;

				if (!errorState.check(resource == nullptr || value_buffer_resource->getCount() == value_declaration->mNumElements, "Encountered mismatch in array elements between array in material and array in shader"))
					return false;

				mStorageUniforms.emplace_back(std::move(instance_value_buffer));
			}

			// Unsupported shader declarations
			else if (declaration_type == RTTI_OF(ShaderVariableStructDeclaration))
			{
				errorState.fail("Nested storage uniform structs are not yet supported for storage buffer shader variables");
				return false;
			}
			else
			{
				errorState.fail("Storage uniform values are not yet supported for storage buffer shader variables");
				return false;
			}
		}
		return true;
	}
}
