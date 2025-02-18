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
	std::unique_ptr<INSTANCE_TYPE> BufferBindingInstance::createBufferBindingInstance(const std::string& bindingName, const DECLARATION_TYPE& declaration, const BufferBinding* binding, BufferBindingChangedCallback bufferChangedCallback, utility::ErrorState& errorState)
	{
		auto result = std::make_unique<INSTANCE_TYPE>(bindingName, declaration, bufferChangedCallback);
		if (binding != nullptr)
		{
			const auto* typed_resource = rtti_cast<const RESOURCE_TYPE>(binding);
			if (!errorState.check(typed_resource != nullptr, "Encountered type mismatch between uniform in material and uniform in shader (%s)", declaration.mName.c_str()))
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
		return declaration.getBufferDeclaration();
	}


	/**
	 * Checks whether a buffer delcaration is compatible with a buffer binding.
	 * It does so by ensuring the shader variable stride is equal to the buffer binding element size.
	 */
	static bool isCompatible(const ShaderVariableDeclaration& declaration, const BufferBinding& binding)
	{
		assert(binding.getBuffer() != nullptr);

		// If the declaration is a struct buffer or value array, check its stride
		auto declaration_type = declaration.get_type();
		if (declaration_type == RTTI_OF(ShaderVariableStructBufferDeclaration))
		{
			const auto& struct_buffer_declaration = static_cast<const ShaderVariableStructBufferDeclaration&>(declaration);
			return struct_buffer_declaration.mStride == binding.getBuffer()->getElementSize();
		}
		else if (declaration_type == RTTI_OF(ShaderVariableValueArrayDeclaration))
		{
			const auto &struct_buffer_declaration = static_cast<const ShaderVariableValueArrayDeclaration &>(declaration);
			return struct_buffer_declaration.mStride == binding.getBuffer()->getElementSize();
		}
		// The declaration is for a primitive value, so we check its size directly
		return declaration.mSize == binding.getBuffer()->getElementSize();
	}


	//////////////////////////////////////////////////////////////////////////
	// UniformStructInstance
	//////////////////////////////////////////////////////////////////////////

	std::unique_ptr<BufferBindingInstance> BufferBindingInstance::createBufferBindingInstanceFromDeclaration(const BufferObjectDeclaration& declaration, const BufferBinding* binding, BufferBindingChangedCallback bufferChangedCallback, utility::ErrorState& errorState)
	{
		// Fetch the binding name
		const std::string& binding_name = declaration.mName;

		// Ensure we retrieve the actual buffer declaration
		const auto& buffer_declaration = getBufferDeclaration(declaration);

		// If the buffer binding was created from a resource, ensure its element count matches
		// the one in the shader declaration and ensure the descriptortype is storage
		if (binding != nullptr)
		{
			// Verify descriptor type
			if (!errorState.check((binding->getBuffer()->getBufferUsageFlags() & VK_BUFFER_USAGE_STORAGE_BUFFER_BIT) > 0,
				"DescriptorType mismatch. StructGPUBuffer 'DescriptorType' property must be 'Storage' to be used as a buffer binding"))
				return nullptr;

			// Verify bounds
			if (!errorState.check(isCompatible(buffer_declaration, *binding),
				"Mismatch between element stride and buffer element size for buffer binding '%s'. Please refer to the alignment requirements for shader resources in Section 15.8.4 of the Vulkan specification", buffer_declaration.mName.c_str()))
				return nullptr;
		}

		auto declaration_type = buffer_declaration.get_type();

		// Creates a BufferBindingStructInstance
		if (declaration_type == RTTI_OF(ShaderVariableStructBufferDeclaration))
		{
			const auto& struct_buffer_declaration = static_cast<const ShaderVariableStructBufferDeclaration&>(buffer_declaration);
			return createBufferBindingInstance<BufferBindingStructInstance, BufferBindingStruct, ShaderVariableStructBufferDeclaration>(
				binding_name, struct_buffer_declaration, binding, bufferChangedCallback, errorState);
		}

		// Creates a BufferBindingNumericInstance
		if (declaration_type == RTTI_OF(ShaderVariableValueArrayDeclaration))
		{
			const auto* value_array_declaration = rtti_cast<const ShaderVariableValueArrayDeclaration>(&buffer_declaration);
			if (value_array_declaration->mElementType == EShaderVariableValueType::UInt)
			{
				return createBufferBindingInstance<BufferBindingUIntInstance, BufferBindingUInt, ShaderVariableValueArrayDeclaration>(
					binding_name, *value_array_declaration, binding, bufferChangedCallback, errorState);
			}
			else if (value_array_declaration->mElementType == EShaderVariableValueType::Int)
			{
				return createBufferBindingInstance<BufferBindingIntInstance, BufferBindingInt, ShaderVariableValueArrayDeclaration>(
					binding_name, *value_array_declaration, binding, bufferChangedCallback, errorState);
			}
			else if (value_array_declaration->mElementType == EShaderVariableValueType::Float)
			{
				return createBufferBindingInstance<BufferBindingFloatInstance, BufferBindingFloat, ShaderVariableValueArrayDeclaration>(
					binding_name, *value_array_declaration, binding, bufferChangedCallback, errorState);
			}
			else if (value_array_declaration->mElementType == EShaderVariableValueType::Vec2)
			{
				return createBufferBindingInstance<BufferBindingVec2Instance, BufferBindingVec2, ShaderVariableValueArrayDeclaration>(
					binding_name, *value_array_declaration, binding, bufferChangedCallback, errorState);
			}
			else if (value_array_declaration->mElementType == EShaderVariableValueType::Vec3)
			{
				return createBufferBindingInstance<BufferBindingVec3Instance, BufferBindingVec3, ShaderVariableValueArrayDeclaration>(
					binding_name, *value_array_declaration, binding, bufferChangedCallback, errorState);
			}
			else if (value_array_declaration->mElementType == EShaderVariableValueType::Vec4)
			{
				return createBufferBindingInstance<BufferBindingVec4Instance, BufferBindingVec4, ShaderVariableValueArrayDeclaration>(
					binding_name, *value_array_declaration, binding, bufferChangedCallback, errorState);
			}
			else if (value_array_declaration->mElementType == EShaderVariableValueType::IVec4)
			{
				return createBufferBindingInstance<BufferBindingIVec4Instance, BufferBindingIVec4, ShaderVariableValueArrayDeclaration>(
					binding_name, *value_array_declaration, binding, bufferChangedCallback, errorState);
			}
			else if (value_array_declaration->mElementType == EShaderVariableValueType::UVec4)
			{
				return createBufferBindingInstance<BufferBindingUVec4Instance, BufferBindingUVec4, ShaderVariableValueArrayDeclaration>(
					binding_name, *value_array_declaration, binding, bufferChangedCallback, errorState);
			}
			else if (value_array_declaration->mElementType == EShaderVariableValueType::Mat4)
			{
				return createBufferBindingInstance<BufferBindingMat4Instance, BufferBindingMat4, ShaderVariableValueArrayDeclaration>(
					binding_name, *value_array_declaration, binding, bufferChangedCallback, errorState);
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
		NAP_ASSERT_MSG(mDeclaration->mStride == buffer.getElementSize(), "Buffer declaration stride is not equal to buffer element size");
		BufferBindingInstance::mBuffer = &buffer;
		raiseChanged();
	}


	void BufferBindingStructInstance::setBuffer(const BufferBindingStruct& resource)
	{
		assert(resource.mBuffer != nullptr);
		NAP_ASSERT_MSG(mDeclaration->mStride == resource.mBuffer->getElementSize(), "Buffer declaration stride is not equal to buffer element size");
		BufferBindingInstance::mBuffer = resource.mBuffer.get();
		raiseChanged();
	}
}
