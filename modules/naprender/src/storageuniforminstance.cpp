/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "storageuniforminstance.h"

RTTI_DEFINE_BASE(nap::BufferBindingInstance)
RTTI_DEFINE_BASE(nap::BufferBindingNumericInstance)

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::BufferBindingStructInstance)
	RTTI_CONSTRUCTOR(const nap::ShaderVariableStructBufferDeclaration&, const nap::BufferBindingChangedCallback&)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::BufferBindingUIntInstance)
	RTTI_CONSTRUCTOR(const nap::ShaderVariableValueArrayDeclaration&, const nap::BufferBindingChangedCallback&)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::BufferBindingIntInstance)
	RTTI_CONSTRUCTOR(const nap::ShaderVariableValueArrayDeclaration&, const nap::BufferBindingChangedCallback&)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::BufferBindingFloatInstance)
	RTTI_CONSTRUCTOR(const nap::ShaderVariableValueArrayDeclaration&, const nap::BufferBindingChangedCallback&)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::BufferBindingVec2Instance)
	RTTI_CONSTRUCTOR(const nap::ShaderVariableValueArrayDeclaration&, const nap::BufferBindingChangedCallback&)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::BufferBindingVec3Instance)
	RTTI_CONSTRUCTOR(const nap::ShaderVariableValueArrayDeclaration&, const nap::BufferBindingChangedCallback&)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::BufferBindingVec4Instance)
	RTTI_CONSTRUCTOR(const nap::ShaderVariableValueArrayDeclaration&, const nap::BufferBindingChangedCallback&)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::BufferBindingMat4Instance)
	RTTI_CONSTRUCTOR(const nap::ShaderVariableValueArrayDeclaration&, const nap::BufferBindingChangedCallback&)
RTTI_END_CLASS


namespace nap
{
	template<typename INSTANCE_TYPE, typename RESOURCE_TYPE, typename DECLARATION_TYPE>
	static std::unique_ptr<INSTANCE_TYPE> createBufferBindingInstance(const BufferBinding* value, const DECLARATION_TYPE& declaration, BufferBindingChangedCallback bufferChangedCallback, utility::ErrorState& errorState)
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


	/**
	 * Returns the declaration of the buffer, if the declaration is a BufferObjectDeclaration.
	 */
	static const ShaderVariableDeclaration& getBufferDeclaration(const ShaderVariableDeclaration& declaration)
	{
		// If a buffer object declaration is passed, we can safely acquire the actual buffer declaration from it
		if (declaration.get_type() == RTTI_OF(BufferObjectDeclaration))
		{
			const BufferObjectDeclaration* buffer_object_declaration = rtti_cast<const BufferObjectDeclaration>(&declaration);
			return buffer_object_declaration->getBufferDeclaration();
		}
		return declaration;
	}

	//////////////////////////////////////////////////////////////////////////
	// UniformStructInstance
	//////////////////////////////////////////////////////////////////////////

	std::unique_ptr<BufferBindingInstance> BufferBindingInstance::createBufferBindingInstanceFromDeclaration(const ShaderVariableDeclaration& declaration, const BufferBinding* binding, BufferBindingChangedCallback bufferChangedCallback, utility::ErrorState& errorState)
	{
		// Ensure we retrieve the actual buffer declaration
		const auto& buffer_declaration = getBufferDeclaration(declaration);
		rtti::TypeInfo declaration_type = buffer_declaration.get_type();

		// Creates a BufferBindingStructInstance
		if (declaration_type == RTTI_OF(ShaderVariableStructBufferDeclaration))
		{
			const ShaderVariableStructBufferDeclaration* struct_buffer_declaration = rtti_cast<const ShaderVariableStructBufferDeclaration>(&buffer_declaration);
			return createBufferBindingInstance<BufferBindingStructInstance, BufferBindingStruct, ShaderVariableStructBufferDeclaration>(binding, *struct_buffer_declaration, bufferChangedCallback, errorState);
		}

		// Creates a BufferBindingNumericInstance
		if (declaration_type == RTTI_OF(ShaderVariableValueArrayDeclaration))
		{
			const ShaderVariableValueArrayDeclaration* value_array_declaration = rtti_cast<const ShaderVariableValueArrayDeclaration>(&buffer_declaration);

			if (value_array_declaration->mElementType == EShaderVariableValueType::UInt)
			{
				return createBufferBindingInstance<BufferBindingUIntInstance, BufferBindingUInt, ShaderVariableValueArrayDeclaration>(binding, *value_array_declaration, bufferChangedCallback, errorState);
			}
			else if (value_array_declaration->mElementType == EShaderVariableValueType::Int)
			{
				return createBufferBindingInstance<BufferBindingIntInstance, BufferBindingInt, ShaderVariableValueArrayDeclaration>(binding, *value_array_declaration, bufferChangedCallback, errorState);
			}
			else if (value_array_declaration->mElementType == EShaderVariableValueType::Float)
			{
				return createBufferBindingInstance<BufferBindingFloatInstance, BufferBindingFloat, ShaderVariableValueArrayDeclaration>(binding, *value_array_declaration, bufferChangedCallback, errorState);
			}
			else if (value_array_declaration->mElementType == EShaderVariableValueType::Vec2)
			{
				return createBufferBindingInstance<BufferBindingVec2Instance, BufferBindingVec2, ShaderVariableValueArrayDeclaration>(binding, *value_array_declaration, bufferChangedCallback, errorState);
			}
			else if (value_array_declaration->mElementType == EShaderVariableValueType::Vec3)
			{
				return createBufferBindingInstance<BufferBindingVec3Instance, BufferBindingVec3, ShaderVariableValueArrayDeclaration>(binding, *value_array_declaration, bufferChangedCallback, errorState);
			}
			else if (value_array_declaration->mElementType == EShaderVariableValueType::Vec4)
			{
				return createBufferBindingInstance<BufferBindingVec4Instance, BufferBindingVec4, ShaderVariableValueArrayDeclaration>(binding, *value_array_declaration, bufferChangedCallback, errorState);
			}
			else if (value_array_declaration->mElementType == EShaderVariableValueType::Mat4)
			{
				return createBufferBindingInstance<BufferBindingMat4Instance, BufferBindingMat4, ShaderVariableValueArrayDeclaration>(binding, *value_array_declaration, bufferChangedCallback, errorState);
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
			// Possibly ShaderVariableStructArrayDeclaration - which is not supported for BufferBindings
			NAP_ASSERT_MSG(false, "Unsupported shader variable declaration");
			assert(false);
		}
		return nullptr;
	}

	
	//bool BufferBindingStructInstance::addBufferBinding(const ShaderVariableStructDeclaration& structDeclaration, const BufferBindingStruct* structResource, const BufferBindingChangedCallback& bufferChangedCallback, utility::ErrorState& errorState)
	//{
	//	for (auto& uniform_declaration : structDeclaration.mMembers)
	//	{
	//		rtti::TypeInfo declaration_type = uniform_declaration->get_type();

	//		const BufferBinding* resource = nullptr;
	//		if (structResource != nullptr)
	//			resource = structResource->findBufferBinding(uniform_declaration->mName);

	//		// Creates a BufferBindingStructInstance
	//		if (declaration_type == RTTI_OF(ShaderVariableStructBufferDeclaration))
	//		{
	//			ShaderVariableStructBufferDeclaration* struct_buffer_declaration = rtti_cast<ShaderVariableStructBufferDeclaration>(uniform_declaration.get());

	//			const BufferBindingStruct* struct_buffer_resource = rtti_cast<const BufferBindingStruct>(resource);
	//			if (!errorState.check(resource == nullptr || struct_buffer_resource->getSize() == struct_buffer_declaration->mSize,
	//				utility::stringFormat("Mismatch between total buffer size in shader and json for storage uniform '%s'. Please refer to the alignment requirements for shader resources in Section 15.6.4 of the Vulkan specification", uniform_declaration->mName.c_str())))
	//				return false;

	//			std::unique_ptr<BufferBindingStructInstance> struct_buffer_instance;
	//			struct_buffer_instance = createBufferBindingInstance<BufferBindingStructInstance, BufferBindingStruct>(resource, *struct_buffer_declaration, bufferChangedCallback, errorState);

	//			if (struct_buffer_instance == nullptr)
	//				return false;

	//			// If the storage uniform was created from a resource, ensure its element count matches the one in the shader declaration and ensure the descriptortype is storage
	//			if (struct_buffer_instance != nullptr && struct_buffer_instance->hasBuffer())
	//			{
	//				if (!errorState.check(struct_buffer_instance->getBuffer().getCount() == struct_buffer_declaration->mNumElements, "Encountered mismatch in array elements between array in material and array in shader"))
	//					return false;

	//				if (!errorState.check(struct_buffer_instance->getBuffer().mDescriptorType == EDescriptorType::Storage, "DescriptorType mismatch. StructGPUBuffer 'DescriptorType' property must be 'Storage' to be used as a storage uniform"))
	//					return false;
	//			}

	//			mBufferBinding = std::move(struct_buffer_instance);
	//		}

	//		// Creates a BufferBindingNumericInstance
	//		else if (declaration_type == RTTI_OF(ShaderVariableValueArrayDeclaration))
	//		{
	//			ShaderVariableValueArrayDeclaration* value_declaration = rtti_cast<ShaderVariableValueArrayDeclaration>(uniform_declaration.get());
	//			std::unique_ptr<BufferBindingNumericInstance> instance_value_buffer;

	//			const BufferBindingNumeric* value_buffer_resource = rtti_cast<const BufferBindingNumeric>(resource);
	//			if (!errorState.check(resource == nullptr || value_buffer_resource != nullptr, "Type mismatch between shader type and json type"))
	//				return false;

	//			if (value_declaration->mElementType == EShaderVariableValueType::UInt)
	//			{
	//				instance_value_buffer = createBufferBindingInstance<BufferBindingUIntInstance, BufferBindingUInt>(resource, *value_declaration, bufferChangedCallback, errorState);
	//			}
	//			else if (value_declaration->mElementType == EShaderVariableValueType::Int)
	//			{
	//				instance_value_buffer = createBufferBindingInstance<BufferBindingIntInstance, BufferBindingInt>(resource, *value_declaration, bufferChangedCallback, errorState);
	//			}
	//			else if (value_declaration->mElementType == EShaderVariableValueType::Float)
	//			{
	//				instance_value_buffer = createBufferBindingInstance<BufferBindingFloatInstance, BufferBindingFloat>(resource, *value_declaration, bufferChangedCallback, errorState);
	//			}
	//			else if (value_declaration->mElementType == EShaderVariableValueType::Vec2)
	//			{
	//				instance_value_buffer = createBufferBindingInstance<BufferBindingVec2Instance, BufferBindingVec2>(resource, *value_declaration, bufferChangedCallback, errorState);
	//			}
	//			else if (value_declaration->mElementType == EShaderVariableValueType::Vec3)
	//			{
	//				instance_value_buffer = createBufferBindingInstance<BufferBindingVec3Instance, BufferBindingVec3>(resource, *value_declaration, bufferChangedCallback, errorState);
	//			}
	//			else if (value_declaration->mElementType == EShaderVariableValueType::Vec4)
	//			{
	//				instance_value_buffer = createBufferBindingInstance<BufferBindingVec4Instance, BufferBindingVec4>(resource, *value_declaration, bufferChangedCallback, errorState);
	//			}
	//			else if (value_declaration->mElementType == EShaderVariableValueType::Mat4)
	//			{
	//				instance_value_buffer = createBufferBindingInstance<BufferBindingMat4Instance, BufferBindingMat4>(resource, *value_declaration, bufferChangedCallback, errorState);
	//			}
	//			else
	//			{
	//				errorState.fail("Data type of shader variable %s is not supported", uniform_declaration->mName.c_str());
	//				return false;
	//			}

	//			if (instance_value_buffer == nullptr)
	//				return false;

	//			// If the storage uniform was created from a resource, ensure its element count matches the one in the shader declaration and ensure the descriptortype is storage
	//			if (value_buffer_resource != nullptr && value_buffer_resource->hasBuffer())
	//			{
	//				if (!errorState.check(value_buffer_resource->getCount() == value_declaration->mNumElements, "Encountered mismatch in array elements between array in material and array in shader"))
	//					return false;

	//				if (!errorState.check(value_buffer_resource->getBuffer()->mDescriptorType == EDescriptorType::Storage, "DescriptorType mismatch. ValueGPUBuffer 'DescriptorType' property must be 'Storage' to be used as a storage uniform."))
	//					return false;
	//			}

	//			mBufferBinding = std::move(instance_value_buffer);
	//		}

	//		// Unsupported shader declarations for storage uniforms.
	//		// As shader variable declarations are descriptor type agnostic, this point can be reached if individual values or structs are declared inside a storage buffer block.
	//		// Notify the user and return false.
	//		else if (declaration_type == RTTI_OF(ShaderVariableStructDeclaration))
	//		{
	//			errorState.fail("Nested storage uniform structs are not yet supported for storage buffer shader variables");
	//			return false;
	//		}
	//		else if (declaration_type == RTTI_OF(ShaderVariableValueDeclaration))
	//		{
	//			errorState.fail("Storage uniform values are not yet supported for storage buffer shader variables");
	//			return false;
	//		}
	//		else
	//		{
	//			NAP_ASSERT_MSG(false, "Unsupported shader variable declaration");
	//			return false;
	//		}
	//	}
	//	return true;
	//}
}
