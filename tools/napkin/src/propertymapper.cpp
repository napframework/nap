#include "propertymapper.h"

#include <QStringList>
#include <napqt/filterpopup.h>
#include <rtti/path.h>

namespace napkin
{
	MaterialPropertyMapper::MaterialPropertyMapper(const PropertyPath& propPath, nap::BaseMaterial& material) :
		mPath(propPath), mMaterial(&material)
	{
		// Fetch shader using RTTI
		auto property_path = nap::rtti::Path::fromString(nap::material::shader);
		nap::rtti::ResolvedPath resolved_path;
		property_path.resolve(&mMaterial, resolved_path);
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
			handleUniformBinding();
		}
		else if (mPath.getName() == nap::material::samplers)
		{
			handleSamplerBinding();
		}
		else if (mPath.getName() == nap::material::buffers)
		{
			handleBufferBinding();
		}
	}


	bool MaterialPropertyMapper::mappable() const
	{
		return mShader != nullptr;
	}


	void MaterialPropertyMapper::handleUniformBinding()
	{
		QStringList names;
		const auto& ubo_decs = mShader->getUBODeclarations();
		for (const auto& dec : ubo_decs)
			names << QString::fromStdString(dec.mName);
		nap::qt::FilterPopup::show(nullptr, names).toStdString();
	}


	void MaterialPropertyMapper::handleSamplerBinding()
	{
		QStringList names;
		const auto& ubo_decs = mShader->getSamplerDeclarations();
		for (const auto& dec : ubo_decs)
			names << QString::fromStdString(dec.mName);
		nap::qt::FilterPopup::show(nullptr, names).toStdString();
	}


	void MaterialPropertyMapper::handleBufferBinding()
	{
		QStringList names;
		const auto& ubo_decs = mShader->getSSBODeclarations();
		for (const auto& dec : ubo_decs)
			names << QString::fromStdString(dec.mName);
		nap::qt::FilterPopup::show(nullptr, names).toStdString();
	}
}
