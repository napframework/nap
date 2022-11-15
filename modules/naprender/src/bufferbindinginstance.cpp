/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "bufferbindinginstance.h"

RTTI_DEFINE_BASE(nap::BufferBindingInstance)
RTTI_DEFINE_BASE(nap::BufferBindingNumericInstance)

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::BufferBindingStructInstance)
	RTTI_CONSTRUCTOR(const std::string&, const nap::ShaderVariableStructBufferDeclaration&, const nap::BufferBindingChangedCallback&)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::BufferBindingUIntInstance)
	RTTI_CONSTRUCTOR(const std::string&, const nap::ShaderVariableValueArrayDeclaration&, const nap::BufferBindingChangedCallback&)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::BufferBindingIntInstance)
	RTTI_CONSTRUCTOR(const std::string&, const nap::ShaderVariableValueArrayDeclaration&, const nap::BufferBindingChangedCallback&)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::BufferBindingFloatInstance)
	RTTI_CONSTRUCTOR(const std::string&, const nap::ShaderVariableValueArrayDeclaration&, const nap::BufferBindingChangedCallback&)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::BufferBindingVec2Instance)
	RTTI_CONSTRUCTOR(const std::string&, const nap::ShaderVariableValueArrayDeclaration&, const nap::BufferBindingChangedCallback&)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::BufferBindingVec3Instance)
	RTTI_CONSTRUCTOR(const std::string&, const nap::ShaderVariableValueArrayDeclaration&, const nap::BufferBindingChangedCallback&)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::BufferBindingVec4Instance)
	RTTI_CONSTRUCTOR(const std::string&, const nap::ShaderVariableValueArrayDeclaration&, const nap::BufferBindingChangedCallback&)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::BufferBindingIVec4Instance)
	RTTI_CONSTRUCTOR(const std::string&, const nap::ShaderVariableValueArrayDeclaration&, const nap::BufferBindingChangedCallback&)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::BufferBindingUVec4Instance)
	RTTI_CONSTRUCTOR(const std::string&, const nap::ShaderVariableValueArrayDeclaration&, const nap::BufferBindingChangedCallback&)
RTTI_END_CLASS

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::BufferBindingMat4Instance)
	RTTI_CONSTRUCTOR(const std::string&, const nap::ShaderVariableValueArrayDeclaration&, const nap::BufferBindingChangedCallback&)
RTTI_END_CLASS


namespace nap
{
	template<typename INSTANCE_TYPE, typename RESOURCE_TYPE, typename DECLARATION_TYPE>
	std::unique_ptr<INSTANCE_TYPE> BufferBindingInstance::createBufferBindingInstance(const DECLARATION_TYPE& declaration, const BufferBinding* binding, BufferBindingChangedCallback bufferChangedCallback, utility::ErrorState& errorState)
	{
		std::unique_ptr<INSTANCE_TYPE> result = std::make_unique<INSTANCE_TYPE>(binding->mName, declaration, bufferChangedCallback);
		if (binding != nullptr)
		{
			const RESOURCE_TYPE* typed_resource = rtti_cast<const RESOURCE_TYPE>(binding);
			if (!errorState.check(typed_resource != nullptr, "Encountered type mismatch between uniform in material and uniform in shader"))
				return nullptr;

			result->mBuffer = typed_resource->mBuffer.get();
		}
		return result;
	}


	/**
	 * Returns the declaration of the buffer, if the declaration is a BufferObjectDeclaration.
	 */
	static const ShaderVariableDeclaration& getBufferDeclaration(const BufferObjectDeclaration& declaration)
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

	std::unique_ptr<BufferBindingInstance> BufferBindingInstance::createBufferBindingInstanceFromDeclaration(const BufferObjectDeclaration& declaration, const BufferBinding* binding, BufferBindingChangedCallback bufferChangedCallback, utility::ErrorState& errorState)
	{
		// Ensure we retrieve the actual buffer declaration
		const auto& buffer_declaration = getBufferDeclaration(declaration);
		rtti::TypeInfo declaration_type = buffer_declaration.get_type();

		// If the buffer binding was created from a resource, ensure its element count matches
		// the one in the shader declaration and ensure the descriptortype is storage
		if (binding != nullptr)
		{
			// Verify descriptortype
			if (!errorState.check((binding->getBuffer()->getBufferUsageFlags() & VK_BUFFER_USAGE_STORAGE_BUFFER_BIT) > 0,
				"DescriptorType mismatch. StructGPUBuffer 'DescriptorType' property must be 'Storage' to be used as a buffer binding"))
				return nullptr;

			// Verify bounds
			if (!errorState.check(binding->getSize() == buffer_declaration.mSize,
				utility::stringFormat("Mismatch between total buffer size in shader and json for buffer binding '%s'. Please refer to the alignment requirements for shader resources in Section 15.6.4 of the Vulkan specification", buffer_declaration.mName.c_str())))
				return nullptr;
		}

		// Creates a BufferBindingStructInstance
		if (declaration_type == RTTI_OF(ShaderVariableStructBufferDeclaration))
		{
			const ShaderVariableStructBufferDeclaration* struct_buffer_declaration = rtti_cast<const ShaderVariableStructBufferDeclaration>(&buffer_declaration);
			if (binding != nullptr)
			{
				// Verify count
				if (!errorState.check(binding->getCount() == struct_buffer_declaration->mNumElements,
					"Encountered mismatch in array elements between array in material and array in shader"))
					return nullptr;
			}

			return createBufferBindingInstance<BufferBindingStructInstance, BufferBindingStruct, ShaderVariableStructBufferDeclaration>(
				*struct_buffer_declaration, binding, bufferChangedCallback, errorState);
		}

		// Creates a BufferBindingNumericInstance
		if (declaration_type == RTTI_OF(ShaderVariableValueArrayDeclaration))
		{
			const ShaderVariableValueArrayDeclaration* value_array_declaration = rtti_cast<const ShaderVariableValueArrayDeclaration>(&buffer_declaration);
			if (binding != nullptr)
			{
				// Verify count
				if (!errorState.check(binding->getCount() == value_array_declaration->mNumElements,
					"Encountered mismatch in array elements between array in material and array in shader"))
					return nullptr;
			}

			if (value_array_declaration->mElementType == EShaderVariableValueType::UInt)
			{
				return createBufferBindingInstance<BufferBindingUIntInstance, BufferBindingUInt, ShaderVariableValueArrayDeclaration>(
					*value_array_declaration, binding, bufferChangedCallback, errorState);
			}
			else if (value_array_declaration->mElementType == EShaderVariableValueType::Int)
			{
				return createBufferBindingInstance<BufferBindingIntInstance, BufferBindingInt, ShaderVariableValueArrayDeclaration>(
					*value_array_declaration, binding, bufferChangedCallback, errorState);
			}
			else if (value_array_declaration->mElementType == EShaderVariableValueType::Float)
			{
				return createBufferBindingInstance<BufferBindingFloatInstance, BufferBindingFloat, ShaderVariableValueArrayDeclaration>(
					*value_array_declaration, binding, bufferChangedCallback, errorState);
			}
			else if (value_array_declaration->mElementType == EShaderVariableValueType::Vec2)
			{
				return createBufferBindingInstance<BufferBindingVec2Instance, BufferBindingVec2, ShaderVariableValueArrayDeclaration>(
					*value_array_declaration, binding, bufferChangedCallback, errorState);
			}
			else if (value_array_declaration->mElementType == EShaderVariableValueType::Vec3)
			{
				return createBufferBindingInstance<BufferBindingVec3Instance, BufferBindingVec3, ShaderVariableValueArrayDeclaration>(
					*value_array_declaration, binding, bufferChangedCallback, errorState);
			}
			else if (value_array_declaration->mElementType == EShaderVariableValueType::Vec4)
			{
				return createBufferBindingInstance<BufferBindingVec4Instance, BufferBindingVec4, ShaderVariableValueArrayDeclaration>(
					*value_array_declaration, binding, bufferChangedCallback, errorState);
			}
			else if (value_array_declaration->mElementType == EShaderVariableValueType::IVec4)
			{
				return createBufferBindingInstance<BufferBindingIVec4Instance, BufferBindingIVec4, ShaderVariableValueArrayDeclaration>(
					*value_array_declaration, binding, bufferChangedCallback, errorState);
			}
			else if (value_array_declaration->mElementType == EShaderVariableValueType::UVec4)
			{
				return createBufferBindingInstance<BufferBindingUVec4Instance, BufferBindingUVec4, ShaderVariableValueArrayDeclaration>(
					*value_array_declaration, binding, bufferChangedCallback, errorState);
			}
			else if (value_array_declaration->mElementType == EShaderVariableValueType::Mat4)
			{
				return createBufferBindingInstance<BufferBindingMat4Instance, BufferBindingMat4, ShaderVariableValueArrayDeclaration>(
					*value_array_declaration, binding, bufferChangedCallback, errorState);
			}
		}

		// Unsupported shader declarations for buffer bindings.
		// As shader variable declarations are descriptor type agnostic, this point can be reached if e.g. individual
		// values or structs are declared inside a storage buffer block. Notify the user and return false.
		else if (declaration_type == RTTI_OF(ShaderVariableStructDeclaration) || declaration_type == RTTI_OF(ShaderVariableValueDeclaration))
		{
			NAP_ASSERT_MSG(false, "Individual values (or structs) not supported for buffer bindings.");
		}
		else if (declaration_type == RTTI_OF(ShaderVariableStructArrayDeclaration))
		{
			NAP_ASSERT_MSG(false, "Declaration type 'ShaderVariableStructArrayDeclaration' is not supported for buffer bindings. Use 'ShaderVariableStructBufferDeclaration' instead.");
		}
		
		NAP_ASSERT_MSG(false, "Unsupported shader variable declaration");
		return nullptr;
	}


	void BufferBindingStructInstance::setBuffer(StructBuffer& buffer)
	{
		assert(buffer.getSize() == mDeclaration->mSize);
		BufferBindingInstance::mBuffer = &buffer;
		raiseChanged();
	}


	void BufferBindingStructInstance::setBuffer(const BufferBindingStruct& resource)
	{
		assert(resource.mBuffer != nullptr);
		assert(resource.mBuffer->getSize() == mDeclaration->mSize);
		BufferBindingInstance::mBuffer = resource.mBuffer.get();
		raiseChanged();
	}
}
