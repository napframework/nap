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
			if (dec == nullptr)
				return;
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
		int iidx = mPath.getArrayLength();
		auto* doc = AppContext::get().getDocument();
		assert(doc != nullptr);
		int oidx = doc->arrayAddNewObject(mPath, sampler_type, iidx);
		assert(oidx == iidx);
		auto& new_sampler = mMaterial->mSamplers.back();
		new_sampler->mID = doc->getUniqueName(declaration.mName, *new_sampler, true);
		new_sampler->mName = declaration.mName;
	}
}
