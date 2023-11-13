#include "propertymapper.h"
#include "naputils.h"
#include "commands.h"

#include <QStringList>
#include <QMessageBox>
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

		// Walk the tree and find the nap::Material or nap::BaseMaterialInstanceResource
		nap::rtti::Object* current_object = propertyPath.getObject();
		mNested = false;
		while (current_object != nullptr)
		{
			// Check if the nested property belongs to a material
			if (current_object->get_type().is_derived_from(RTTI_OF(nap::BaseMaterial)))
			{
				// Cast and find uniforms property
				auto& material = static_cast<nap::BaseMaterial&>(*current_object);
				auto uniform_prop = material.get_type().get_property(nap::material::uniforms);
				assert(uniform_prop.is_valid());
				mRootUniforms = uniform_prop.get_value(material);
				assert(mRootUniforms.is_valid());

				// Resolve shader (can be null)
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

				// Store uniforms property
				auto uniform_prop = variant.get_type().get_property(nap::material::uniforms);
				assert(uniform_prop.is_valid());
				mRootUniforms = uniform_prop.get_value(variant);
				assert(mRootUniforms.is_valid());

				// Locate material
				const auto& resource = variant.get_value<nap::BaseMaterialInstanceResource>();
				auto material_property = resource.getMaterialProperty();
				assert(material_property.is_valid());
				auto material_variant = material_property.get_value(resource);
				assert(material_variant.is_valid());
				assert(material_variant.get_type().is_wrapper());
				material = material_variant.extract_wrapped_value().get_value<nap::BaseMaterial*>();
				if (material != nullptr)
				{
					resolveShader(*material);
				}
				return;
			}

			// Try parent
			mNested = true;
			current_object = doc->getEmbeddedObjectOwner(*current_object);
		}
	}


	void MaterialPropertyMapper::resolveShader(const nap::BaseMaterial& material)
	{
		// Fetch shader using RTTI, can be null -> in that case the property can't be mapped
		auto property_path = nap::rtti::Path::fromString(nap::material::shader);
		nap::rtti::ResolvedPath resolved_path;
		property_path.resolve(&material, resolved_path);
		assert(resolved_path.isValid());
		nap::rtti::Variant prop_value = resolved_path.getValue();
		assert(prop_value.get_type().is_wrapper());
		mShader = prop_value.extract_wrapped_value().get_value<nap::BaseShader*>();
	}


	static bool resolveUniformStruct(const nap::UniformStruct& uniformStruct, const nap::Uniform& target, std::vector<const nap::Uniform*>& ioPath)
	{
		// Push struct name and validate
		ioPath.emplace_back(&uniformStruct);
		if (&uniformStruct == &target)
			return true;

		// Check children
		for (const auto& uniform : uniformStruct.mUniforms)
		{
			// Resolve struct
			if (uniform->get_type().is_derived_from(RTTI_OF(nap::UniformStruct)))
			{
				const auto& nested_struct = static_cast<const nap::UniformStruct&>(*uniform);
				if (resolveUniformStruct(nested_struct, target, ioPath))
					return true;
			}

			// Resolve other members
			{
				// Push uniform name and check for a match
				ioPath.emplace_back(uniform.get());
				if (uniform.get() == &target)
					return true;

				// Resolve struct array
				if (uniform->get_type().is_derived_from(RTTI_OF(nap::UniformStructArray)))
				{
					const auto& struct_array = static_cast<const nap::UniformStructArray&>(*uniform);
					int index = 0;
					for (const auto& array_struct : struct_array.mStructs)
					{
						// Push index and check if it matches
						ioPath.emplace_back(array_struct.get());
						if (array_struct.get() == &target)
							return true;

						// Resolve member struct
						for (const auto& array_struct_child : array_struct->mUniforms)
						{
							auto tt = array_struct_child.get_type();
							if (array_struct_child->get_type().is_derived_from(RTTI_OF(nap::UniformStruct)))
							{
								const auto& nested_struct = static_cast<const nap::UniformStruct&>(*array_struct_child);
								if (resolveUniformStruct(nested_struct, target, ioPath))
									return true;
							}
						}

						// Pop index
						ioPath.pop_back(); index++;
					}
				}
				// Pop uniform
				ioPath.pop_back();
			}
		}

		// Pop struct if not valid
		ioPath.pop_back();
		return false;
	}


	static bool resolveUniformPath(const PropertyPath& path, rttr::variant root, std::vector<const nap::Uniform*>& outPath)
	{
		// Get target uniform
		auto* target_uniform = rtti_cast<nap::Uniform>(path.getObject());
		assert(target_uniform != nullptr);

		// Get all available ubos
		rttr::variant_array_view view = root.create_array_view();
		assert(view.is_valid());

		// Iterate and construct path
		for (int i = 0; i < view.get_size(); i++)
		{
			auto uniform_value = view.get_value(i);
			assert(uniform_value.is_valid());
			assert(uniform_value.get_type().is_wrapper());
			auto uniform_struct = uniform_value.extract_wrapped_value().get_value<nap::UniformStruct*>();
			assert(uniform_struct != nullptr);
		
			// Resolve
			if (resolveUniformStruct(*uniform_struct, *target_uniform, outPath))
				return true;
		}
		return false;
	}


	static const nap::ShaderVariableDeclaration* resolveShaderDeclaration(std::vector<const nap::Uniform*>& path, const nap::ShaderVariableDeclaration& dec)
	{
		// Get current uniform
		assert(!path.empty());
		const auto* uniform = *path.begin();

		// Check if the uniform name matches the declaration
		if (dec.mName == uniform->mName)
		{
			// Pop it and check if we are done resolving
			path.erase(path.begin());
			if (path.empty())
			{
				return &dec;
			}

			// Continue resolving for struct
			if (dec.get_type().is_derived_from(RTTI_OF(nap::ShaderVariableStructDeclaration)))
			{
				const auto& struct_dec = static_cast<const nap::ShaderVariableStructDeclaration&>(dec);
				for (const auto& member : struct_dec.mMembers)
				{
					const auto* resolved = resolveShaderDeclaration(path, *member);
					if (resolved != nullptr)
					{
						return resolved;
					}
				}
			}
			// Continue resolving for struct array
			else if (dec.get_type().is_derived_from(RTTI_OF(nap::ShaderVariableStructArrayDeclaration)))
			{
				const auto& array_dec = static_cast<const nap::ShaderVariableStructArrayDeclaration&>(dec);
				assert(!array_dec.mElements.empty());
				const auto* resolved = resolveShaderDeclaration(path, *array_dec.mElements[0]);
				if (resolved != nullptr)
				{
					return resolved;
				}
			}
		}
		return nullptr;
	}


	static const nap::ShaderVariableDeclaration* resolveShaderPath(std::vector<const nap::Uniform*>& unipPath, const nap::BufferObjectDeclarationList& declarations)
	{
		auto uniform_path = unipPath; auto ini_length = uniform_path.size();
		for (const auto& dec : declarations)
		{
			// Attempt to resolve this path
			const auto* resolved_dec = resolveShaderDeclaration(uniform_path, dec);
			if (resolved_dec != nullptr)
				return resolved_dec;

			// Partial resolves are not allowed
			if (uniform_path.size() != ini_length)
				break;
		}
		return nullptr;
	}


	void MaterialPropertyMapper::map(QWidget* parent)
	{
		// Make sure the shader is initialized
		assert(mShader != nullptr);
		if (mShader->getDescriptorSetLayout() == VK_NULL_HANDLE)
		{
			// Load shader
			nap::utility::ErrorState error;
			if (!loadShader(*mShader, AppContext::get().getCore(), error))
			{
				// Show msg
				std::string err_msg = nap::utility::stringFormat("Can't create '%s' binding", mPath.getName().c_str());
				QMessageBox msg(parent);
				msg.setText(QString::fromStdString(err_msg));
				msg.setInformativeText(QString("Failed to load %1").arg(QString::fromStdString(mShader->mID)));
				msg.setDetailedText(QString::fromStdString(error.toString()));
				msg.setStandardButtons(QMessageBox::Ok);
				msg.setIcon(QMessageBox::Critical);
				msg.setWindowTitle("Error");
				QSpacerItem* spacer = new QSpacerItem(300, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);
				QGridLayout* layout = (QGridLayout*)msg.layout();
				layout->addItem(spacer, layout->rowCount(), 0, 1, layout->columnCount());
				msg.exec();

				// Log
				nap::Logger::error(err_msg);
				nap::Logger::error(error.toString());
				return;
			}
		}

		// Uniforms
		assert(mPath.isArray());
		if (!mNested && mPath.getName() == nap::material::uniforms)
		{
			const auto* dec = selectVariableDeclaration(mShader->getUBODeclarations(), parent);
			if (dec != nullptr)
				addVariableBinding(*dec, mPath);
			return;
		}

		// Samplers
		if (!mNested && mPath.getName() == nap::material::samplers)
		{
			const auto* dec = selectSamplerDeclaration(parent);
			if (dec != nullptr)
				addSamplerBinding(*dec, mPath);
			return;
		}

		// Buffers
		if (!mNested && mPath.getName() == nap::material::buffers)
		{
			const auto* dec = selectBufferDeclaration(mShader->getSSBODeclarations(), parent);
			if (dec != nullptr)
				addBufferBinding(*dec, mPath);
			return;
		}

		// Constants
		if (!mNested && mPath.getName() == nap::material::constants)
		{
			const auto* dec = selectConstantDeclaration(parent);
			if (dec != nullptr)
				addConstantBinding(*dec, mPath);
			return;
		}

		// Vertex binding
		auto array_type = mPath.getArrayElementType();
		if (!mNested && mPath.getName() == nap::material::vbindings)
		{
			const auto* dec = selectVertexAttrDeclaration(parent);
			if (dec != nullptr)
				addVertexBinding(*dec, mPath);
			return;
		}

		// Nested uniform type
		if (mPath.getObject()->get_type().is_derived_from(RTTI_OF(nap::Uniform)))
		{
			// Get 'material' path to uniform resource
			std::vector<const nap::Uniform*> outPath;
			if (resolveUniformPath(mPath, mRootUniforms, outPath))
			{
				// Find corresponding shader declaration
				assert(outPath.size() > 0); auto ini_length = outPath.size();
				const auto& ubo_decs = mShader->getUBODeclarations();
				const auto* resolved_dec = resolveShaderPath(outPath, mShader->getUBODeclarations());
				if (resolved_dec != nullptr)
				{
					if (resolved_dec->get_type().is_derived_from(RTTI_OF(nap::ShaderVariableStructDeclaration)))
					{
						// Show type selection for struct
						const auto& struct_dec = static_cast<const nap::ShaderVariableStructDeclaration&>(*resolved_dec);
						auto selected_dec = selectVariableDeclaration(struct_dec, parent);
						if (selected_dec != nullptr)
							addVariableBinding(*selected_dec, mPath);
						return;
					}
					else if (resolved_dec->get_type().is_derived_from(RTTI_OF(nap::ShaderVariableStructArrayDeclaration)))
					{
						// Insert new item into array
						const auto& array_dec = static_cast<const nap::ShaderVariableStructArrayDeclaration&>(*resolved_dec);
						assert(array_dec.mElements.size() > 0);
						addVariableBinding(*array_dec.mElements[0], mPath);
						return;
					}
				}
			}

			// Uniform can't be resolved -> fallback
			nap::Logger::warn("Can't map '%s' to '%s': Uniform binding can't be resolved",
				mPath.toString().c_str(), mShader->mID.c_str());

			addUserBinding(parent);
			return;
		}

		// Property can't be resolved by this mapper.
		NAP_ASSERT_MSG(false,
			nap::utility::stringFormat("%s can't be resolved by this mapper!", mPath.toString().c_str()).c_str());
	}


	std::unique_ptr<MaterialPropertyMapper> MaterialPropertyMapper::mappable(const PropertyPath& path)
	{
		if (!path.isArray())
			return nullptr;

		// All supported mappable types
		static const std::vector<nap::rtti::TypeInfo> map_types =
		{
			RTTI_OF(nap::Uniform),
			RTTI_OF(nap::Sampler),
			RTTI_OF(nap::BufferBinding),
			RTTI_OF(nap::ShaderConstant),
			RTTI_OF(nap::Material::VertexAttributeBinding)
		};

		// Check to see if the array type is mappable
		auto array_type = path.getArrayElementType();
		auto compatible = std::find_if(map_types.begin(), map_types.end(), [&array_type](const auto& it)
			{
				return array_type.is_derived_from(it);
			}
		);
		if (compatible == map_types.end())
			return nullptr;

		// Now create it
		auto material_mapper = std::make_unique<MaterialPropertyMapper>(path);

		// Mapper is only valid when shader is resolved from object 
		if (material_mapper->mShader == nullptr)
			return nullptr;

		// Valid mapper
		return std::move(material_mapper);
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


	const nap::ShaderVariableDeclaration* MaterialPropertyMapper::selectVariableDeclaration(const nap::ShaderVariableStructDeclaration& uniform, QWidget* parent)
	{
		QStringList names;
		std::unordered_map<std::string, const nap::ShaderVariableDeclaration*> dec_map;
		dec_map.reserve(uniform.mMembers.size());
		for (const auto& dec : uniform.mMembers)
		{
			if (dec->mName != nap::uniform::mvpStruct)
			{
				dec_map.emplace(dec->mName, dec.get());
				names << QString::fromStdString(dec->mName);
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


	const nap::VertexAttributeDeclaration* MaterialPropertyMapper::selectVertexAttrDeclaration(QWidget* parent)
	{
		nap::Shader* shader = rtti_cast<nap::Shader>(mShader);
		assert(shader != nullptr);

		QStringList names;
		const auto& vert_attrs = shader->getAttributes();
		std::unordered_map<std::string, const nap::VertexAttributeDeclaration*> dec_map;
		dec_map.reserve(vert_attrs.size());
		for (const auto& attr : vert_attrs)
		{
			dec_map.emplace(attr.second->mName, attr.second.get());
			names << QString::fromStdString(attr.second->mName);
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
		AppContext::get().executeCommand(new ArrayAddNewObjectCommand(propertyPath, uniformType));
		int oidx = propertyPath.getArrayLength() - 1;
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
		bool is_array = declaration.mNumElements > 1;
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
			return;
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

			// Ask if entries should be created for the array
			if (QMessageBox::question(nullptr, "Array",
				QString("Create entries for array '%1'?").arg(declaration.mName.c_str()),
				QMessageBox::Yes, QMessageBox::No) == QMessageBox::No)
				return;

			// Get path to values property
			PropertyPath values_path(*array_uniform,
				array_uniform->get_type().get_property(nap::uniform::values), *doc);

			// Add value entries
			for (int i = 0; i < array_dec.mNumElements; i++)
				AppContext::get().executeCommand(new ArrayAddValueCommand(values_path, i));
			return;
		}

		// Handle struct array
		if (dec_type.is_derived_from(RTTI_OF(nap::ShaderVariableStructArrayDeclaration)))
		{
			const auto& array_dec = static_cast<const nap::ShaderVariableStructArrayDeclaration&>(declaration);

			// Create and add value binding
			auto* struct_uni = createBinding<nap::UniformStructArray>(declaration.mName, RTTI_OF(nap::UniformStructArray), path, *doc);

			// Ask if entries should be created for the array
			if (QMessageBox::question(nullptr, "Array",
				QString("Create entries for array '%1'?").arg(declaration.mName.c_str()),
				QMessageBox::Yes, QMessageBox::No) == QMessageBox::No)
				return;

			// Get path to structs property
			PropertyPath structs_path(*struct_uni,
				struct_uni->get_type().get_property(nap::uniform::structs), *doc);

			// Add value entries
			for (const auto& entry : array_dec.mElements)
				addVariableBinding(*entry, structs_path);
			return;
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


	const nap::ShaderConstantDeclaration* MaterialPropertyMapper::selectConstantDeclaration(QWidget* parent)
	{
		QStringList names;
		const auto& shader_decs = mShader->getConstantDeclarations();
		std::unordered_map<std::string, const nap::ShaderConstantDeclaration*> dec_map;
		dec_map.reserve(shader_decs.size());
		for (const auto& dec : shader_decs)
		{
			dec_map.emplace(dec.mName, &dec);
			names << QString::fromStdString(dec.mName);
		}
		auto selection = nap::qt::FilterPopup::show(parent, names);
		return selection.isEmpty() ? nullptr : dec_map[selection.toStdString()];
	}


	void MaterialPropertyMapper::addConstantBinding(const nap::ShaderConstantDeclaration& declaration, const PropertyPath& propPath)
	{
		// Get document
		auto* doc = AppContext::get().getDocument();
		assert(doc != nullptr);

		// Create binding
		auto* binding = createBinding<nap::ShaderConstant>(declaration.mName, RTTI_OF(nap::ShaderConstant), propPath, *doc);

		// Set it to the default value specified in the shader
		binding->mValue = declaration.mValue;
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


	void MaterialPropertyMapper::addVertexBinding(const nap::VertexAttributeDeclaration& declaration, const PropertyPath& propPath)
	{
		// Add new entry
		int clen = propPath.getArrayLength();
		AppContext::get().executeCommand(new ArrayAddValueCommand(propPath));
		int nidx = propPath.getArrayLength() - 1;
		assert(nidx == clen);

		// Get inserted element and update shader declaration.
		auto vert_path = propPath.getArrayElement(nidx);
		auto vert_vari = vert_path.getValue();
		assert(vert_vari.is_valid());
		auto vertex_binding = vert_vari.get_value<nap::Material::VertexAttributeBinding>();
		vertex_binding.mShaderAttributeID = declaration.mName;
		propPath.getArrayElement(nidx).setValue(vertex_binding);
	}
}
