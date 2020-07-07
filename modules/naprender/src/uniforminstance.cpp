#include "uniforminstance.h"
#include "uniforms.h"

namespace nap
{
	template<typename INSTANCE_TYPE, typename RESOURCE_TYPE, typename DECLARATION_TYPE>
	static std::unique_ptr<INSTANCE_TYPE> createUniformValueInstance(const Uniform* value, const DECLARATION_TYPE& declaration, utility::ErrorState& errorState)
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

	std::unique_ptr<UniformInstance> UniformStructInstance::createUniformFromDeclaration(const UniformDeclaration& declaration, const UniformCreatedCallback& uniformCreatedCallback)
	{
		rtti::TypeInfo declaration_type = declaration.get_type();

		if (declaration_type == RTTI_OF(UniformStructArrayDeclaration))
		{
			const UniformStructArrayDeclaration* struct_array_declaration = rtti_cast<const UniformStructArrayDeclaration>(&declaration);
			std::unique_ptr<UniformStructArrayInstance> struct_array_instance = std::make_unique<UniformStructArrayInstance>(*struct_array_declaration);
			for (auto& struct_declaration : struct_array_declaration->mElements)
			{
				std::unique_ptr<UniformStructInstance> struct_instance = std::make_unique<UniformStructInstance>(*struct_declaration, uniformCreatedCallback);
				struct_array_instance->addElement(std::move(struct_instance));
			}
			return std::move(struct_array_instance);
		}
		else if (declaration_type == RTTI_OF(UniformValueArrayDeclaration))
		{
			const UniformValueArrayDeclaration* value_array_declaration = rtti_cast<const UniformValueArrayDeclaration>(&declaration);

			if (value_array_declaration->mElementType == EUniformValueType::Int)
			{
				std::unique_ptr<UniformIntArrayInstance> array_instance = std::make_unique<UniformIntArrayInstance>(*value_array_declaration);
				array_instance->getValues().resize(value_array_declaration->mNumElements);
				return std::move(array_instance);
			}
			else if (value_array_declaration->mElementType == EUniformValueType::Float)
			{
				std::unique_ptr<UniformFloatArrayInstance> array_instance = std::make_unique<UniformFloatArrayInstance>(*value_array_declaration);
				array_instance->getValues().resize(value_array_declaration->mNumElements);
				return std::move(array_instance);
			}
			else if (value_array_declaration->mElementType == EUniformValueType::Vec2)
			{
				std::unique_ptr<UniformVec2ArrayInstance> array_instance = std::make_unique<UniformVec2ArrayInstance>(*value_array_declaration);
				array_instance->getValues().resize(value_array_declaration->mNumElements);
				return std::move(array_instance);
			}
			else if (value_array_declaration->mElementType == EUniformValueType::Vec3)
			{
				std::unique_ptr<UniformVec3ArrayInstance> array_instance = std::make_unique<UniformVec3ArrayInstance>(*value_array_declaration);
				array_instance->getValues().resize(value_array_declaration->mNumElements);
				return std::move(array_instance);
			}
			else if (value_array_declaration->mElementType == EUniformValueType::Vec4)
			{
				std::unique_ptr<UniformVec4ArrayInstance> array_instance = std::make_unique<UniformVec4ArrayInstance>(*value_array_declaration);
				array_instance->getValues().resize(value_array_declaration->mNumElements);
				return std::move(array_instance);
			}
			else if (value_array_declaration->mElementType == EUniformValueType::Mat4)
			{
				std::unique_ptr<UniformMat4ArrayInstance> array_instance = std::make_unique<UniformMat4ArrayInstance>(*value_array_declaration);
				array_instance->getValues().resize(value_array_declaration->mNumElements);
				return std::move(array_instance);
			}
		}
		else if (declaration_type == RTTI_OF(UniformStructDeclaration))
		{
			const UniformStructDeclaration* struct_declaration = rtti_cast<const UniformStructDeclaration>(&declaration);
			return std::make_unique<UniformStructInstance>(*struct_declaration, uniformCreatedCallback);
		}
		else
		{
			const UniformValueDeclaration* value_declaration = rtti_cast<const UniformValueDeclaration>(&declaration);

			if (value_declaration->mType == EUniformValueType::Int)
			{
				return std::make_unique<UniformIntInstance>(*value_declaration);
			}
			else if (value_declaration->mType == EUniformValueType::Float)
			{
				return std::make_unique<UniformFloatInstance>(*value_declaration);
			}
			else if (value_declaration->mType == EUniformValueType::Vec2)
			{
				return std::make_unique<UniformVec2Instance>(*value_declaration);
			}
			else if (value_declaration->mType == EUniformValueType::Vec3)
			{
				return std::make_unique<UniformVec3Instance>(*value_declaration);
			}
			else if (value_declaration->mType == EUniformValueType::Vec4)
			{
				return std::make_unique<UniformVec4Instance>(*value_declaration);
			}
			else if (value_declaration->mType == EUniformValueType::Mat4)
			{
				return std::make_unique<UniformMat4Instance>(*value_declaration);
			}
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


	bool UniformStructInstance::addUniformRecursive(const UniformStructDeclaration& structDeclaration, const UniformStruct* structResource, const UniformCreatedCallback& uniformCreatedCallback, bool createDefaults, utility::ErrorState& errorState)
	{
		for (auto& uniform_declaration : structDeclaration.mMembers)
		{
			rtti::TypeInfo declaration_type = uniform_declaration->get_type();

			const Uniform* resource = nullptr;
			if (structResource != nullptr)
				resource = findUniformStructMember(structResource->mUniforms, *uniform_declaration);

			if (!createDefaults && resource == nullptr)
				continue;

			if (declaration_type == RTTI_OF(UniformStructArrayDeclaration))
			{
				UniformStructArrayDeclaration* struct_array_declaration = rtti_cast<UniformStructArrayDeclaration>(uniform_declaration.get());

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
			else if (declaration_type == RTTI_OF(UniformValueArrayDeclaration))
			{
				UniformValueArrayDeclaration* value_array_declaration = rtti_cast<UniformValueArrayDeclaration>(uniform_declaration.get());
				std::unique_ptr<UniformValueArrayInstance> instance_value_array;

				const UniformValueArray* value_array_resource = rtti_cast<const UniformValueArray>(resource);
				if (!errorState.check(resource == nullptr || value_array_resource != nullptr, "Type mismatch between shader type and json type"))
					return false;

				if (value_array_declaration->mElementType == EUniformValueType::Int)
				{
					instance_value_array = createUniformValueInstance<UniformIntArrayInstance, UniformIntArray>(resource, *value_array_declaration, errorState);
				}
				else if (value_array_declaration->mElementType == EUniformValueType::Float)
				{
					instance_value_array = createUniformValueInstance<UniformFloatArrayInstance, UniformFloatArray>(resource, *value_array_declaration, errorState);
				}
				else if (value_array_declaration->mElementType == EUniformValueType::Vec2)
				{
					instance_value_array = createUniformValueInstance<UniformVec2ArrayInstance, UniformVec2Array>(resource, *value_array_declaration, errorState);
				}
				else if (value_array_declaration->mElementType == EUniformValueType::Vec3)
				{
					instance_value_array = createUniformValueInstance<UniformVec3ArrayInstance, UniformVec3Array>(resource, *value_array_declaration, errorState);
				}
				else if (value_array_declaration->mElementType == EUniformValueType::Vec4)
				{
					instance_value_array = createUniformValueInstance<UniformVec4ArrayInstance, UniformVec4Array>(resource, *value_array_declaration, errorState);
				}
				else if (value_array_declaration->mElementType == EUniformValueType::Mat4)
				{
					instance_value_array = createUniformValueInstance<UniformMat4ArrayInstance, UniformMat4Array>(resource, *value_array_declaration, errorState);
				}
				else
				{
					assert(false);
				}

				if (instance_value_array == nullptr)
					return false;

				// If the array was not set in json, we need to ensure the array has the correct size & is filled with default values
				if (resource == nullptr)
					instance_value_array->setDefault();

				if (!errorState.check(resource == nullptr || value_array_resource->getCount() == value_array_declaration->mNumElements, "Encountered mismatch in array elements between array in material and array in shader"))
					return false;

				mUniforms.emplace_back(std::move(instance_value_array));
			}
			else if (declaration_type == RTTI_OF(UniformStructDeclaration))
			{
				const UniformStruct* struct_resource = rtti_cast<const UniformStruct>(resource);

				UniformStructDeclaration* struct_declaration = rtti_cast<UniformStructDeclaration>(uniform_declaration.get());
				std::unique_ptr<UniformStructInstance> struct_instance = std::make_unique<UniformStructInstance>(*struct_declaration, uniformCreatedCallback);
				if (!struct_instance->addUniformRecursive(*struct_declaration, struct_resource, uniformCreatedCallback, createDefaults, errorState))
					return false;

				mUniforms.emplace_back(std::move(struct_instance));
			}
			else
			{
				UniformValueDeclaration* value_declaration = rtti_cast<UniformValueDeclaration>(uniform_declaration.get());
				std::unique_ptr<UniformValueInstance> value_instance;

				if (value_declaration->mType == EUniformValueType::Int)
				{
					value_instance = createUniformValueInstance<UniformIntInstance, UniformInt>(resource, *value_declaration, errorState);
				}
				else if (value_declaration->mType == EUniformValueType::Float)
				{
					value_instance = createUniformValueInstance<UniformFloatInstance, UniformFloat>(resource, *value_declaration, errorState);
				}
				else if (value_declaration->mType == EUniformValueType::Vec2)
				{
					value_instance = createUniformValueInstance<UniformVec2Instance, UniformVec2>(resource, *value_declaration, errorState);
				}
				else if (value_declaration->mType == EUniformValueType::Vec3)
				{
					value_instance = createUniformValueInstance<UniformVec3Instance, UniformVec3>(resource, *value_declaration, errorState);
				}
				else if (value_declaration->mType == EUniformValueType::Vec4)
				{
					value_instance = createUniformValueInstance<UniformVec4Instance, UniformVec4>(resource, *value_declaration, errorState);
				}
				else if (value_declaration->mType == EUniformValueType::Mat4)
				{
					value_instance = createUniformValueInstance<UniformMat4Instance, UniformMat4>(resource, *value_declaration, errorState);
				}
				else
				{
					assert(false);
				}

				if (value_instance == nullptr)
					return false;

				mUniforms.emplace_back(std::move(value_instance));
			}
		}
		return true;
	}


	//////////////////////////////////////////////////////////////////////////
	// UniformStructArrayInstance
	//////////////////////////////////////////////////////////////////////////

	void UniformStructArrayInstance::addElement(std::unique_ptr<UniformStructInstance> element)
	{
		mElements.emplace_back(std::move(element));
	}

}
