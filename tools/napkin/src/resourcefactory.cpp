/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "resourcefactory.h"
#include "napkin-resources.h"
#include "appcontext.h"

#include <nap/logger.h>
#include <entity.h>
#include <scene.h>

using namespace nap;
using namespace nap::rtti;


napkin::FileType::FileType(const nap::rtti::EPropertyFileType filetype, const QString& desc, const QStringList& ext) :
	mType(filetype), mDescription(desc), mExtensions(ext)
{ }


napkin::Icon::Icon(const QString& path) : mIcon(path)
{
	if (mIcon.isNull())
		return;

	QImage img(path);
	assert(!img.isNull());
	img.invertPixels(QImage::InvertMode::InvertRgb);
	QPixmap pix;
	pix.convertFromImage(img);
	mIconInverted = QIcon(pix);
}


const QIcon napkin::ResourceFactory::getIcon(const nap::rtti::Object& object) const
{
	const static std::vector<std::pair<nap::rtti::TypeInfo, QString>> icon_map = {
		{ RTTI_OF(Entity),		QRC_ICONS_ENTITY },
		{ RTTI_OF(Component),	QRC_ICONS_COMPONENT },
		{ RTTI_OF(Scene),		QRC_ICONS_SCENE },
		{ RTTI_OF(Object),		QRC_ICONS_RTTIOBJECT },
	};

	// Try to find an icon.
	// Icon is valid when found in map (equal or derived from)
	// TODO: Create icon registration method and implement better type matching algorithm,
	// TODO: Right now order of declaration is important
	for (auto entry : icon_map)
	{
		rttr::type obj_type = object.get_type();
		if (obj_type.is_derived_from(entry.first))
			return getIcon(entry.second);
	}
	return QIcon();
}


const QIcon napkin::ResourceFactory::getIcon(const QString& path) const
{
	// Map that holds all the loaded icons
	static std::unordered_map<std::string, Icon> icons;

	// Find icon, if not part of set add & update iterator
	auto it = icons.find(path.toStdString());
	if (it == icons.end())
	{
		auto em = icons.emplace(std::make_pair(path.toStdString(), napkin::Icon(path)));
		it = em.first;
	}

	// Now return based on style
	const Theme* theme = AppContext::get().getThemeManager().getCurrentTheme();
	assert(theme != nullptr);
	return theme != nullptr && theme->invertIcons() ? it->second.mIconInverted : it->second.mIcon;
}


const QString napkin::ResourceFactory::getFileFilter(const nap::rtti::Property& prop) const
{
	QStringList wildcards;
	FileType type = getFiletype(prop);
	if (type.mExtensions.empty())
		return QString("%1 (*)").arg(type.mDescription);

	for (auto& ext : type.mExtensions)
		wildcards << QString("*.%1").arg(ext);
	auto filter = QString("%1 (%2)").arg(type.mDescription, wildcards.join(" "));
	return filter;
}


const napkin::FileType& napkin::ResourceFactory::getFiletype(const nap::rtti::Property& prop) const
{
	static const std::vector<FileType> file_types = {
		{EPropertyFileType::Any,			"All Files",		{ }},
		{EPropertyFileType::Image,			"Image Files",      { }},
		{EPropertyFileType::FragShader,		"Fragment Shaders", {"frag"}},
		{EPropertyFileType::VertShader,		"Vertex Shaders",   {"vert"}},
		{EPropertyFileType::Python,			"Python Files",     {"py"}},
		{EPropertyFileType::Mesh,			"NAP Mesh Files",   {"mesh"}},
		{EPropertyFileType::Video,			"Video Files",      {} },
		{EPropertyFileType::ImageSequence,	"Image Sequence",	{} },
		{EPropertyFileType::Font,			"Font",	{"ttf", "ttc", "otf", "otc", "pfa", "pfb"} }
	};


	for (auto& ftype : file_types)
	{
		if (nap::rtti::isFileType(prop, ftype.mType))
			return ftype;
	}
	return file_types[0];
}
