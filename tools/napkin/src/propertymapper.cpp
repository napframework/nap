#include "propertymapper.h"

#include <QStringList>
#include <napqt/filterpopup.h>
#include <rtti/path.h>
#include <renderglobals.h>
#include <appcontext.h>

namespace napkin
{
	MaterialPropertyMapper::MaterialPropertyMapper(const PropertyPath& propPath, nap::BaseMaterial& material) :
		mPath(propPath), mMaterial(&material)
	{
		// Fetch shader using RTTI
		auto property_path = nap::rtti::Path::fromString(nap::material::shader);
		nap::rtti::ResolvedPath resolved_path;
		property_path.resolve(mMaterial, resolved_path);
		assert(resolved_path.isValid());
		nap::rtti::Variant prop_value = resolved_path.getValue();
		assert(prop_value.get_type().is_wrapper());
		mShader = prop_value.extract_wrapped_value().get_value<nap::BaseShader*>();
	}


	void MaterialPropertyMapper::map(QWidget* parent)
	{
		// Shader must be assigned
		if (mShader == nullptr)
		{
			nap::Logger::warn("Can't map '%s', missing shader", mPath.getName().c_str());
			return;
		}

		// Now handle the various mapping types
		if (mPath.getName() == nap::material::uniforms)
		{
			const auto* dec = selectVariableDeclaration(mShader->getUBODeclarations(), parent);
			if (dec != nullptr)
				addVariableBinding(*dec, mPath);
		}
		else if (mPath.getName() == nap::material::samplers)
		{
			const auto* dec = selectSamplerDeclaration(parent);
			if (dec != nullptr)
				addSamplerBinding(*dec);
		}
		else if (mPath.getName() == nap::material::buffers)
		{
			const auto* dec = selectVariableDeclaration(mShader->getSSBODeclarations(), parent);
			if (dec == nullptr)
				return;
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


	void MaterialPropertyMapper::addSamplerBinding(const nap::SamplerDeclaration& declaration)
	{
		// Figure out if we need to add an array or single instance
		nap::rtti::TypeInfo sampler_type = declaration.mNumArrayElements > 1 ?
			RTTI_OF(nap::Sampler2DArray) : RTTI_OF(nap::Sampler2D);

		// Add it
		assert(mPath.isArray());
		int iidx = mPath.getArrayLength();
		auto* doc = AppContext::get().getDocument(); assert(doc != nullptr);
		int oidx = doc->arrayAddNewObject(mPath, sampler_type, iidx); assert(oidx == iidx);

		// Set name and ID
		auto& new_sampler = mMaterial->mSamplers.back();
		doc->setObjectName(*new_sampler, nap::utility::stringFormat("%s:%s",
			mPath.getObject()->mID.c_str(), declaration.mName.c_str()));
		new_sampler->mName = declaration.mName;
	}

	
	void MaterialPropertyMapper::addVariableBinding(const nap::ShaderVariableDeclaration& declaration, const PropertyPath& path)
	{
		assert(path.isArray());
		auto* doc = AppContext::get().getDocument(); assert(doc != nullptr);

		// Handle struct declaration
		auto dec_type = declaration.get_type();
		if (dec_type.is_derived_from(RTTI_OF(nap::ShaderVariableStructDeclaration)))
		{
			// Create uniform struct
			int iidx = path.getArrayLength();
			int oidx = doc->arrayAddNewObject(path, RTTI_OF(nap::UniformStruct), iidx);
			assert(iidx == oidx);

			// Fetch created item
			auto child_path = path.getArrayElement(oidx);
			auto child_valu = child_path.getValue();
			assert(child_valu.get_type().is_wrapper());
			auto* child_uni = child_valu.extract_wrapped_value().get_value<nap::UniformStruct*>();
			assert(child_uni != nullptr);

			// Assign name and ID
			child_uni->mName = declaration.mName;
			doc->setObjectName(*child_uni, nap::utility::stringFormat("%s:%s",
				path.getObject()->mID.c_str(),
				declaration.mName.c_str()));

			// Create path to members property
			PropertyPath members_path(*child_uni,
				child_uni->get_type().get_property(nap::uniform::uniforms), *doc);

			// Add variable binding for every member
			const auto& struct_dec = static_cast<const nap::ShaderVariableStructDeclaration&>(declaration);
			for (const auto& member_dec : struct_dec.mMembers)
			{
				addVariableBinding(*member_dec, members_path);
			}
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

			// Create uniform struct
			int iidx = path.getArrayLength();
			int oidx = doc->arrayAddNewObject(path, found_it->second, iidx);
			assert(iidx == oidx);

			// Fetch created item
			auto child_path = path.getArrayElement(oidx);
			auto child_valu = child_path.getValue();
			assert(child_valu.get_type().is_wrapper());
			auto* child_uni = child_valu.extract_wrapped_value().get_value<nap::UniformStruct*>();
			assert(child_uni != nullptr);

			// Assign name and ID
			child_uni->mName = value_dec.mName;
			doc->setObjectName(*child_uni,
				nap::utility::stringFormat("%s:%s", path.getObject()->mID.c_str(), value_dec.mName.c_str()));
		}
	}
}
