#include "propertymapper.h"
#include "naputils.h"
#include "commands.h"

#include <QStringList>
#include <napqt/filterpopup.h>
#include <rtti/path.h>
#include <renderglobals.h>
#include <appcontext.h>

namespace napkin
{
	MaterialPropertyMapper::MaterialPropertyMapper(const PropertyPath& propertyPath) :
		mPath(propertyPath)
	{
		nap::BaseMaterial* material = nullptr;
		auto* doc = propertyPath.getDocument();
		assert(doc != nullptr);

		// Walk the tree and find the material
		nap::rtti::Object* current_object = propertyPath.getObject();
		while (current_object != nullptr)
		{
			// Check if the nested property belongs to a material
			if (current_object->get_type().is_derived_from(RTTI_OF(nap::BaseMaterial)))
			{
				auto& material = static_cast<nap::BaseMaterial&>(*current_object);
				resolveShader(material);
				return;
			}

			// Check if the object has a material instance resource
			auto properties = current_object->get_type().get_properties();
			for (const auto& property : properties)
			{
				if(!property.get_type().is_derived_from(RTTI_OF(nap::BaseMaterialInstanceResource)))
					continue;

				// Extract material instance resource
				auto variant = property.get_value(*current_object);
				assert(variant.is_valid());
				const auto& resource = variant.get_value<nap::BaseMaterialInstanceResource>();

				// Locate material
				auto material_property = resource.getMaterialProperty();
				assert(material_property.is_valid());
				auto material_variant = material_property.get_value(resource);
				assert(material_variant.is_valid());
				assert(material_variant.get_type().is_wrapper());
				material = material_variant.extract_wrapped_value().get_value<nap::BaseMaterial*>();
				if (material != nullptr)
					resolveShader(*material);
				return;
			}

			// Try parent
			current_object = doc->getEmbeddedObjectOwner(*current_object);
		}
	}


	void MaterialPropertyMapper::resolveShader(const nap::BaseMaterial& material)
	{
		// Fetch shader using RTTI
		auto property_path = nap::rtti::Path::fromString(nap::material::shader);
		nap::rtti::ResolvedPath resolved_path;
		property_path.resolve(&material, resolved_path);
		assert(resolved_path.isValid());
		nap::rtti::Variant prop_value = resolved_path.getValue();
		assert(prop_value.get_type().is_wrapper());
		mShader = prop_value.extract_wrapped_value().get_value<nap::BaseShader*>();
	}


	void MaterialPropertyMapper::map(QWidget* parent)
	{
		// If a shader is missing revert to regular behavior
		if (mShader == nullptr)
		{
			addUserBinding(parent);
			return;
		}

		// Make sure the shader is initialized
		if (mShader->getDescriptorSetLayout() == VK_NULL_HANDLE)
		{
			// Load shader
			nap::utility::ErrorState error;
			if (!loadShader(*mShader, AppContext::get().getCore(), error))
			{
				nap::Logger::error("Can't create binding for '%s' because '%s' is not initialized",
					mPath.toString().c_str(), mShader->mID.c_str());
				nap::Logger::error(error.toString());
				return;
			}
		}

		assert(mPath.isArray());
		auto binding_type = mPath.getArrayElementType();

		// Uniforms
		if (binding_type.is_derived_from(RTTI_OF(nap::Uniform)))
		{
			const auto* dec = selectVariableDeclaration(mShader->getUBODeclarations(), parent);
			if (dec != nullptr)
				addVariableBinding(*dec, mPath);
		}
		// Samplers
		else if (binding_type.is_derived_from(RTTI_OF(nap::Sampler)))
		{
			const auto* dec = selectSamplerDeclaration(parent);
			if (dec != nullptr)
				addSamplerBinding(*dec, mPath);
		}
		// Buffers
		else if (binding_type.is_derived_from(RTTI_OF(nap::BufferBinding)))
		{
			const auto* dec = selectBufferDeclaration(mShader->getSSBODeclarations(), parent);
			if (dec != nullptr)
				addBufferBinding(*dec, mPath);
		}
	}


	bool MaterialPropertyMapper::mappable() const
	{
		return mShader != nullptr;
	}


	const nap::ShaderVariableDeclaration* MaterialPropertyMapper::selectVariableDeclaration(const nap::BufferObjectDeclarationList& list, QWidget* parent)
	{
		QStringList names;
		std::unordered_map<std::string, const nap::ShaderVariableDeclaration*> dec_map;
		dec_map.reserve(list.size());
		for (const auto& dec : list)
		{
			if (dec.mName != nap::uniform::mvpStruct)
			{
				dec_map.emplace(dec.mName, &dec);
				names << QString::fromStdString(dec.mName);
			}
		}
		auto selection = nap::qt::FilterPopup::show(parent, names);
		return selection.isEmpty() ? nullptr : dec_map[selection.toStdString()];
	}


	const nap::SamplerDeclaration* MaterialPropertyMapper::selectSamplerDeclaration(QWidget* parent)
	{
		QStringList names;
		const auto& shader_decs = mShader->getSamplerDeclarations();
		std::unordered_map<std::string, const nap::SamplerDeclaration*> dec_map;
		dec_map.reserve(shader_decs.size());
		for (const auto& dec : shader_decs)
		{
			dec_map.emplace(dec.mName, &dec);
			names << QString::fromStdString(dec.mName);
		}
		auto selection = nap::qt::FilterPopup::show(parent, names);
		return selection.isEmpty() ? nullptr : dec_map[selection.toStdString()];
	}


	template<typename T>
	static T* createBinding(const std::string& name, const nap::rtti::TypeInfo& uniformType, const PropertyPath& propertyPath, Document& doc)
	{
		// Create uniform struct
		assert(propertyPath.isArray());
		int iidx = propertyPath.getArrayLength();
		int oidx = doc.arrayAddNewObject(propertyPath, uniformType, iidx);
		assert(iidx == oidx);

		// Fetch created uniform
		auto uni_path = propertyPath.getArrayElement(oidx);
		auto uni_value = uni_path.getValue();
		assert(uni_value.get_type().is_wrapper());
		auto* uni_obj = uni_value.extract_wrapped_value().get_value<T*>();
		assert(uni_obj != nullptr);

		// Assign name and ID
		uni_obj->mName = name;
		doc.setObjectName(*uni_obj, name, true);
		return uni_obj;
	}


	void MaterialPropertyMapper::addSamplerBinding(const nap::SamplerDeclaration& declaration, const PropertyPath& propPath)
	{
		// Get document
		auto* doc = AppContext::get().getDocument();
		assert(doc != nullptr);

		// Only 2D samplers are supports
		if (declaration.mType != nap::SamplerDeclaration::EType::Type_2D)
		{
			nap::Logger::warn("Data type of shader variable %s is not supported", declaration.mName.c_str());
			return;
		}

		// Create binding
		bool is_array = declaration.mNumArrayElements > 1;
		nap::rtti::TypeInfo sampler_type = is_array ? RTTI_OF(nap::Sampler2DArray) : RTTI_OF(nap::Sampler2D);
		createBinding<nap::Sampler>(declaration.mName, sampler_type, propPath, *doc);
	}

	
	void MaterialPropertyMapper::addVariableBinding(const nap::ShaderVariableDeclaration& declaration, const PropertyPath& path)
	{
		// Get document
		auto* doc = AppContext::get().getDocument();
		assert(doc != nullptr);

		// Handle struct declaration
		auto dec_type = declaration.get_type();
		if (dec_type.is_derived_from(RTTI_OF(nap::ShaderVariableStructDeclaration)))
		{
			// Create struct binding
			auto* new_uniform = createBinding<nap::UniformStruct>(declaration.mName, RTTI_OF(nap::UniformStruct), path, *doc);

			// Get path to members property
			PropertyPath members_path(*new_uniform,
				new_uniform->get_type().get_property(nap::uniform::uniforms), *doc);

			// Add variable binding for every member
			const auto& struct_dec = static_cast<const nap::ShaderVariableStructDeclaration&>(declaration);
			for (const auto& member_dec : struct_dec.mMembers)
				addVariableBinding(*member_dec, members_path);
		}

		// Handle value declaration
		if (dec_type.is_derived_from(RTTI_OF(nap::ShaderVariableValueDeclaration)))
		{
			const auto& value_dec = static_cast<const nap::ShaderVariableValueDeclaration&>(declaration);
			static const std::unordered_map<nap::EShaderVariableValueType, nap::rtti::TypeInfo> vuni_map =
			{
				{ nap::EShaderVariableValueType::Float,	RTTI_OF(nap::UniformFloat)	},
				{ nap::EShaderVariableValueType::Int,	RTTI_OF(nap::UniformInt)	},
				{ nap::EShaderVariableValueType::UInt,	RTTI_OF(nap::UniformUInt)	},
				{ nap::EShaderVariableValueType::Vec2,	RTTI_OF(nap::UniformVec2)	},
				{ nap::EShaderVariableValueType::Vec3,	RTTI_OF(nap::UniformVec3)	},
				{ nap::EShaderVariableValueType::Vec4,	RTTI_OF(nap::UniformVec4)	},
				{ nap::EShaderVariableValueType::IVec4,	RTTI_OF(nap::UniformIVec4)	},
				{ nap::EShaderVariableValueType::UVec4,	RTTI_OF(nap::UniformUVec4)	},
				{ nap::EShaderVariableValueType::Mat4,	RTTI_OF(nap::UniformMat4)	}
			};

			// Make sure the declared type is supported
			// TODO: Add support for Mat2 & Mat3
			auto found_it = vuni_map.find(value_dec.mType);
			if (found_it == vuni_map.end())
			{
				nap::Logger::warn("Data type of shader variable %s is not supported", value_dec.mName.c_str());
				return;
			}

			// Create and add value binding
			createBinding<nap::UniformValue>(declaration.mName, found_it->second, path, *doc);
		}

		// Handle value array declaration
		if (dec_type.is_derived_from(RTTI_OF(nap::ShaderVariableValueArrayDeclaration)))
		{
			const auto& array_dec = static_cast<const nap::ShaderVariableValueArrayDeclaration&>(declaration);
			static const std::unordered_map<nap::EShaderVariableValueType, nap::rtti::TypeInfo> vuni_map =
			{
				{ nap::EShaderVariableValueType::Float,	RTTI_OF(nap::UniformFloatArray)	},
				{ nap::EShaderVariableValueType::Int,	RTTI_OF(nap::UniformIntArray)	},
				{ nap::EShaderVariableValueType::UInt,	RTTI_OF(nap::UniformUIntArray)	},
				{ nap::EShaderVariableValueType::Vec2,	RTTI_OF(nap::UniformVec2Array)	},
				{ nap::EShaderVariableValueType::Vec3,	RTTI_OF(nap::UniformVec3Array)	},
				{ nap::EShaderVariableValueType::Vec4,	RTTI_OF(nap::UniformVec4Array)	},
				{ nap::EShaderVariableValueType::IVec4,	RTTI_OF(nap::UniformIVec4Array)	},
				{ nap::EShaderVariableValueType::UVec4,	RTTI_OF(nap::UniformUVec4Array)	},
				{ nap::EShaderVariableValueType::Mat4,	RTTI_OF(nap::UniformMat4Array)	}
			};

			// Make sure the declared type is supported
			// TODO: Add support for Mat2 & Mat3
			auto found_it = vuni_map.find(array_dec.mElementType);
			if (found_it == vuni_map.end())
			{
				nap::Logger::warn("Data type of shader variable %s is not supported", array_dec.mName.c_str());
				return;
			}

			// Create and add value binding
			auto* array_uniform = createBinding<nap::UniformValueArray>(declaration.mName, found_it->second, path, *doc);

			// Get path to values property
			PropertyPath values_path(*array_uniform,
				array_uniform->get_type().get_property(nap::uniform::values), *doc);

			// Add value entries
			for (int i = 0; i < array_dec.mNumElements; i++)
				doc->arrayAddValue(values_path);
		}

		// Handle struct array
		if (dec_type.is_derived_from(RTTI_OF(nap::ShaderVariableStructArrayDeclaration)))
		{
			const auto& array_dec = static_cast<const nap::ShaderVariableStructArrayDeclaration&>(declaration);

			// Create and add value binding
			auto* struct_uni = createBinding<nap::UniformStructArray>(declaration.mName, RTTI_OF(nap::UniformStructArray), path, *doc);

			// Get path to structs property
			PropertyPath structs_path(*struct_uni,
				struct_uni->get_type().get_property(nap::uniform::structs), *doc);

			// Add value entries
			for (const auto& entry : array_dec.mElements)
				addVariableBinding(*entry, structs_path);
		}
	}


	const nap::BufferObjectDeclaration* MaterialPropertyMapper::selectBufferDeclaration(const nap::BufferObjectDeclarationList& list, QWidget* parent)
	{
		QStringList names;
		std::unordered_map<std::string, const nap::BufferObjectDeclaration*> dec_map;
		dec_map.reserve(list.size());
		for (const auto& dec : list)
		{
			dec_map.emplace(dec.mName, &dec);
			names << QString::fromStdString(dec.mName);
		}
		auto selection = nap::qt::FilterPopup::show(parent, names);
		return selection.isEmpty() ? nullptr : dec_map[selection.toStdString()];
	}


	void MaterialPropertyMapper::addBufferBinding(const nap::BufferObjectDeclaration& declaration, const PropertyPath& propPath)
	{
		// Get document
		auto* doc = AppContext::get().getDocument();
		assert(doc != nullptr);

		// Handle struct buffer
		const auto& buffer_dec = declaration.getBufferDeclaration();
		if (buffer_dec.get_type().is_derived_from(RTTI_OF(nap::ShaderVariableStructBufferDeclaration)))
		{
			const auto& struct_buf_dec = static_cast<const nap::ShaderVariableStructBufferDeclaration&>(buffer_dec);
			createBinding<nap::BufferBindingStruct>(declaration.mName, RTTI_OF(nap::BufferBindingStruct), propPath, *doc);
			return;
		}

		// Handle value array buffer
		if (buffer_dec.get_type().is_derived_from(RTTI_OF(nap::ShaderVariableValueArrayDeclaration)))
		{
			const auto& array_dec = static_cast<const nap::ShaderVariableValueArrayDeclaration&>(buffer_dec);
			static const std::unordered_map<nap::EShaderVariableValueType, nap::rtti::TypeInfo> vuni_map =
			{
				{ nap::EShaderVariableValueType::Float,	RTTI_OF(nap::BufferBindingFloat)	},
				{ nap::EShaderVariableValueType::Int,	RTTI_OF(nap::BufferBindingInt)		},
				{ nap::EShaderVariableValueType::UInt,	RTTI_OF(nap::BufferBindingUInt)		},
				{ nap::EShaderVariableValueType::Vec2,	RTTI_OF(nap::BufferBindingVec2)		},
				{ nap::EShaderVariableValueType::Vec3,	RTTI_OF(nap::BufferBindingVec3)		},
				{ nap::EShaderVariableValueType::Vec4,	RTTI_OF(nap::BufferBindingVec4)		},
				{ nap::EShaderVariableValueType::IVec4,	RTTI_OF(nap::BufferBindingIVec4)	},
				{ nap::EShaderVariableValueType::UVec4,	RTTI_OF(nap::BufferBindingUVec4)	},
				{ nap::EShaderVariableValueType::Mat4,	RTTI_OF(nap::BufferBindingMat4)		}
			};

			// Make sure the declared type is supported
			// TODO: Add support for Mat2 & Mat3
			auto found_it = vuni_map.find(array_dec.mElementType);
			if (found_it == vuni_map.end())
			{
				nap::Logger::warn("Data type of shader variable %s is not supported", array_dec.mName.c_str());
				return;
			}
			createBinding<nap::BufferBindingStruct>(declaration.mName, found_it->second, propPath, *doc);
			return;
		}

		nap::Logger::warn("Unable to create buffer binding");
		nap::Logger::warn("Unsupported shader variable declaration '%s'", declaration.get_type().get_name().data());
	}


	void MaterialPropertyMapper::addUserBinding(QWidget* parent)
	{
		auto array_type = mPath.getArrayElementType();
		TypePredicate predicate = [array_type](auto t) { return t.is_derived_from(array_type); };
		rttr::type selected_type = showTypeSelector(parent, predicate);
		if (selected_type.is_valid())
		{
			AppContext::get().executeCommand(new ArrayAddNewObjectCommand(mPath, selected_type));
		}
	}
}
