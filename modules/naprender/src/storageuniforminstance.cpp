/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "storageuniforminstance.h"

RTTI_DEFINE_BASE(nap::StorageUniformInstance)
RTTI_DEFINE_BASE(nap::StorageUniformBufferInstance)
RTTI_DEFINE_BASE(nap::StorageUniformValueBufferInstance)

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::StorageUniformStructInstance)
	RTTI_CONSTRUCTOR(const nap::ShaderVariableStructDeclaration&, const nap::StorageUniformCreatedCallback&)
	RTTI_FUNCTION("findStorageUniformBuffer", (nap::StorageUniformInstance* (nap::StorageUniformStructInstance::*)(const std::string&)) &nap::StorageUniformStructInstance::findStorageUniformBuffer)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::StorageUniformStructBufferInstance)
	RTTI_CONSTRUCTOR(const nap::ShaderVariableStructBufferDeclaration&)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::StorageUniformUIntBufferInstance)
	RTTI_CONSTRUCTOR(const nap::ShaderVariableValueArrayDeclaration&)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::StorageUniformIntBufferInstance)
	RTTI_CONSTRUCTOR(const nap::ShaderVariableValueArrayDeclaration&)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::StorageUniformFloatBufferInstance)
	RTTI_CONSTRUCTOR(const nap::ShaderVariableValueArrayDeclaration&)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::StorageUniformVec2BufferInstance)
	RTTI_CONSTRUCTOR(const nap::ShaderVariableValueArrayDeclaration&)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::StorageUniformVec3BufferInstance)
	RTTI_CONSTRUCTOR(const nap::ShaderVariableValueArrayDeclaration&)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::StorageUniformVec4BufferInstance)
	RTTI_CONSTRUCTOR(const nap::ShaderVariableValueArrayDeclaration&)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::StorageUniformMat4BufferInstance)
	RTTI_CONSTRUCTOR(const nap::ShaderVariableValueArrayDeclaration&)
RTTI_END_CLASS


namespace nap
{
	template<typename INSTANCE_TYPE, typename RESOURCE_TYPE, typename DECLARATION_TYPE>
	static std::unique_ptr<INSTANCE_TYPE> createStorageUniformBufferInstance(const StorageUniform* value, const DECLARATION_TYPE& declaration, StorageUniformBufferChangedCallback bufferChangedCallback, utility::ErrorState& errorState)
	{
		std::unique_ptr<INSTANCE_TYPE> result = std::make_unique<INSTANCE_TYPE>(declaration, bufferChangedCallback);
		if (value != nullptr)
		{
			const RESOURCE_TYPE* typed_resource = rtti_cast<const RESOURCE_TYPE>(value);
			if (!errorState.check(typed_resource != nullptr, "Encountered type mismatch between uniform in material and uniform in shader"))
				return nullptr;

			result->setBuffer(*typed_resource);
		}
		return result;
	}


	//////////////////////////////////////////////////////////////////////////
	// UniformStructInstance
	//////////////////////////////////////////////////////////////////////////

	std::unique_ptr<StorageUniformBufferInstance> StorageUniformStructInstance::createStorageUniformBufferFromDeclaration(const ShaderVariableDeclaration& declaration, const StorageUniformCreatedCallback& storageUniformCreatedCallback)
	{
		rtti::TypeInfo declaration_type = declaration.get_type();

		// Creates a StorageUniformStructBufferInstance
		if (declaration_type == RTTI_OF(ShaderVariableStructBufferDeclaration))
		{
			const ShaderVariableStructBufferDeclaration* struct_buffer_declaration = rtti_cast<const ShaderVariableStructBufferDeclaration>(&declaration);
			std::unique_ptr<StorageUniformStructBufferInstance> buffer_instance = std::make_unique<StorageUniformStructBufferInstance>(*struct_buffer_declaration);
			return std::move(buffer_instance);
		}

		// Creates a StorageUniformValueBufferInstance
		else if (declaration_type == RTTI_OF(ShaderVariableValueArrayDeclaration))
		{
			const ShaderVariableValueArrayDeclaration* value_array_declaration = rtti_cast<const ShaderVariableValueArrayDeclaration>(&declaration);

			if (value_array_declaration->mElementType == EShaderVariableValueType::UInt)
			{
				std::unique_ptr<StorageUniformUIntBufferInstance> buffer_instance = std::make_unique<StorageUniformUIntBufferInstance>(*value_array_declaration);
				return std::move(buffer_instance);
			}
			else if (value_array_declaration->mElementType == EShaderVariableValueType::Int)
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

		// Unsupported shader declarations for storage uniforms.
		// As shader variable declarations are descriptor type agnostic, this point can be reached if individual values or structs are declared inside a storage buffer block.
		// Notify the user and return false.
		else if (declaration_type == RTTI_OF(ShaderVariableStructDeclaration))
		{
			NAP_ASSERT_MSG(false, "Individual structs not supported for storage uniforms");
		}
		else if (declaration_type == RTTI_OF(ShaderVariableValueDeclaration))
		{
			NAP_ASSERT_MSG(false, "Individual values not supported for storage uniforms");
		}
		else
		{
			// Possibly ShaderVariableStructArrayDeclaration - which is not supported for StorageUniforms
			NAP_ASSERT_MSG(false, "Unsupported shader variable declaration");
			assert(false);
		}
		return nullptr;
	}


	StorageUniformBufferInstance* StorageUniformStructInstance::findStorageUniformBuffer(const std::string& name)
	{
		if (mStorageUniformBuffer != nullptr)
		{
			if (mStorageUniformBuffer->getDeclaration().mName == name)
				return mStorageUniformBuffer.get();
		}
		return nullptr;
	}


	bool StorageUniformStructInstance::addStorageUniformBuffer(const ShaderVariableStructDeclaration& structDeclaration, const StorageUniformStruct* structResource, const StorageUniformBufferChangedCallback& bufferChangedCallback, utility::ErrorState& errorState)
	{
		for (auto& uniform_declaration : structDeclaration.mMembers)
		{
			rtti::TypeInfo declaration_type = uniform_declaration->get_type();

			const StorageUniformBuffer* resource = nullptr;
			if (structResource != nullptr)
				resource = structResource->findStorageUniformBuffer(uniform_declaration->mName);

			// Creates a StorageUniformStructBufferInstance
			if (declaration_type == RTTI_OF(ShaderVariableStructBufferDeclaration))
			{
				ShaderVariableStructBufferDeclaration* struct_buffer_declaration = rtti_cast<ShaderVariableStructBufferDeclaration>(uniform_declaration.get());

				const StorageUniformStructBuffer* struct_buffer_resource = rtti_cast<const StorageUniformStructBuffer>(resource);
				if (!errorState.check(resource == nullptr || struct_buffer_resource->getSize() == struct_buffer_declaration->mSize,
					utility::stringFormat("Mismatch between total buffer size in shader and json for storage uniform '%s'. Please refer to the alignment requirements for shader resources in Section 15.6.4 of the Vulkan specification", uniform_declaration->mName.c_str())))
					return false;

				std::unique_ptr<StorageUniformStructBufferInstance> struct_buffer_instance;
				struct_buffer_instance = createStorageUniformBufferInstance<StorageUniformStructBufferInstance, StorageUniformStructBuffer>(resource, *struct_buffer_declaration, bufferChangedCallback, errorState);

				if (struct_buffer_instance == nullptr)
					return false;

				// If the storage uniform was created from a resource, ensure its element count matches the one in the shader declaration and ensure the descriptortype is storage
				if (struct_buffer_instance != nullptr && struct_buffer_instance->hasBuffer())
				{
					if (!errorState.check(struct_buffer_instance->getBuffer().getCount() == struct_buffer_declaration->mNumElements, "Encountered mismatch in array elements between array in material and array in shader"))
						return false;

					if (!errorState.check(struct_buffer_instance->getBuffer().mDescriptorType == EDescriptorType::Storage, "DescriptorType mismatch. StructGPUBuffer 'DescriptorType' property must be 'Storage' to be used as a storage uniform"))
						return false;
				}

				mStorageUniformBuffer = std::move(struct_buffer_instance);
			}

			// Creates a StorageUniformValueBufferInstance
			else if (declaration_type == RTTI_OF(ShaderVariableValueArrayDeclaration))
			{
				ShaderVariableValueArrayDeclaration* value_declaration = rtti_cast<ShaderVariableValueArrayDeclaration>(uniform_declaration.get());
				std::unique_ptr<StorageUniformValueBufferInstance> instance_value_buffer;

				const StorageUniformValueBuffer* value_buffer_resource = rtti_cast<const StorageUniformValueBuffer>(resource);
				if (!errorState.check(resource == nullptr || value_buffer_resource != nullptr, "Type mismatch between shader type and json type"))
					return false;

				if (value_declaration->mElementType == EShaderVariableValueType::UInt)
				{
					instance_value_buffer = createStorageUniformBufferInstance<StorageUniformUIntBufferInstance, StorageUniformUIntBuffer>(resource, *value_declaration, bufferChangedCallback, errorState);
				}
				else if (value_declaration->mElementType == EShaderVariableValueType::Int)
				{
					instance_value_buffer = createStorageUniformBufferInstance<StorageUniformIntBufferInstance, StorageUniformIntBuffer>(resource, *value_declaration, bufferChangedCallback, errorState);
				}
				else if (value_declaration->mElementType == EShaderVariableValueType::Float)
				{
					instance_value_buffer = createStorageUniformBufferInstance<StorageUniformFloatBufferInstance, StorageUniformFloatBuffer>(resource, *value_declaration, bufferChangedCallback, errorState);
				}
				else if (value_declaration->mElementType == EShaderVariableValueType::Vec2)
				{
					instance_value_buffer = createStorageUniformBufferInstance<StorageUniformVec2BufferInstance, StorageUniformVec2Buffer>(resource, *value_declaration, bufferChangedCallback, errorState);
				}
				else if (value_declaration->mElementType == EShaderVariableValueType::Vec3)
				{
					instance_value_buffer = createStorageUniformBufferInstance<StorageUniformVec3BufferInstance, StorageUniformVec3Buffer>(resource, *value_declaration, bufferChangedCallback, errorState);
				}
				else if (value_declaration->mElementType == EShaderVariableValueType::Vec4)
				{
					instance_value_buffer = createStorageUniformBufferInstance<StorageUniformVec4BufferInstance, StorageUniformVec4Buffer>(resource, *value_declaration, bufferChangedCallback, errorState);
				}
				else if (value_declaration->mElementType == EShaderVariableValueType::Mat4)
				{
					instance_value_buffer = createStorageUniformBufferInstance<StorageUniformMat4BufferInstance, StorageUniformMat4Buffer>(resource, *value_declaration, bufferChangedCallback, errorState);
				}
				else
				{
					errorState.fail("Data type of shader variable %s is not supported", uniform_declaration->mName.c_str());
					return false;
				}

				if (instance_value_buffer == nullptr)
					return false;

				// If the storage uniform was created from a resource, ensure its element count matches the one in the shader declaration and ensure the descriptortype is storage
				if (value_buffer_resource != nullptr && value_buffer_resource->hasBuffer())
				{
					if (!errorState.check(value_buffer_resource->getCount() == value_declaration->mNumElements, "Encountered mismatch in array elements between array in material and array in shader"))
						return false;

					if (!errorState.check(value_buffer_resource->getBuffer()->mDescriptorType == EDescriptorType::Storage, "DescriptorType mismatch. ValueGPUBuffer 'DescriptorType' property must be 'Storage' to be used as a storage uniform."))
						return false;
				}

				mStorageUniformBuffer = std::move(instance_value_buffer);
			}

			// Unsupported shader declarations for storage uniforms.
			// As shader variable declarations are descriptor type agnostic, this point can be reached if individual values or structs are declared inside a storage buffer block.
			// Notify the user and return false.
			else if (declaration_type == RTTI_OF(ShaderVariableStructDeclaration))
			{
				errorState.fail("Nested storage uniform structs are not yet supported for storage buffer shader variables");
				return false;
			}
			else if (declaration_type == RTTI_OF(ShaderVariableValueDeclaration))
			{
				errorState.fail("Storage uniform values are not yet supported for storage buffer shader variables");
				return false;
			}
			else
			{
				NAP_ASSERT_MSG(false, "Unsupported shader variable declaration");
				return false;
			}
		}
		return true;
	}
}
