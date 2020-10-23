/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "resourcefactory.h"
#include "napkin-resources.h"

#include <nap/logger.h>
#include <entity.h>
#include <scene.h>

using namespace nap;
using namespace nap::rtti;


napkin::ResourceFactory::ResourceFactory()
{
	mObjectIconMap = {
			{RTTI_OF(Entity),    QRC_ICONS_ENTITY},
			{RTTI_OF(Scene),     QRC_ICONS_SCENE},
			{RTTI_OF(Component), QRC_ICONS_COMPONENT},
			{RTTI_OF(Object),    QRC_ICONS_RTTIOBJECT},
	};

	mFileTypes = {
			{EPropertyFileType::Image,			"Image Files",      getImageExtensions()},
			{EPropertyFileType::FragShader,		"Fragment Shaders", {"frag"}},
			{EPropertyFileType::VertShader,		"Vertex Shaders",   {"vert"}},
			{EPropertyFileType::Python,			"Python Files",     {"py"}},
			{EPropertyFileType::Mesh,			"NAP Mesh Files",   {"mesh"}},
			{EPropertyFileType::Video,			"Video Files",      getVideoExtensions()},
			{EPropertyFileType::ImageSequence,	"Image Sequence",	getImageExtensions()},
			{EPropertyFileType::Font,			"True Type Font",	getFontExtensions() }
	};
}


const QIcon napkin::ResourceFactory::getIcon(const nap::rtti::Object& object) const
{
	for (auto entry : mObjectIconMap)
	{
		rttr::type obj_type = object.get_type();
		if (obj_type.is_derived_from(entry.first))
		{
			QIcon icon(entry.second);
			assert(!icon.isNull());
			return icon;
		}
	}

	return QIcon();
}


const QString napkin::ResourceFactory::getFileFilter(const nap::rtti::Property& prop) const
{
	QStringList wildcards;
	FileType type = getFiletype(prop);
	if (type == mAnyFileType)
		return QString();

	for (auto& ext : type.mExtensions)
		wildcards << QString("*.%1").arg(ext);

	auto filter = QString("%1 (%2)").arg(type.mDescription, wildcards.join(" "));
	nap::Logger::info(filter.toStdString());
	return filter;
}

const napkin::FileType& napkin::ResourceFactory::getFiletype(const nap::rtti::Property& prop) const
{
	if (!nap::rtti::hasFlag(prop, EPropertyMetaData::FileLink))
		return mAnyFileType;

	for (auto& ftype : mFileTypes)
	{
		if (nap::rtti::isFileType(prop, ftype.mFileType))
			return ftype;
	}
	return mAnyFileType;
}

const QStringList napkin::ResourceFactory::getImageExtensions()
{
	return {};
//	if (mImageExtensions.isEmpty())
//	{
//		for (int i = 0, len = FreeImage_GetFIFCount(); i < len; i++)
//		{
//			const char *exts = FreeImage_GetFIFExtensionList(static_cast<FREE_IMAGE_FORMAT>(i));
//			for (auto ext : QString::fromUtf8(exts).split(","))
//				mImageExtensions << ext;
//		}
//	}
//	return mImageExtensions;
}

const QStringList napkin::ResourceFactory::getVideoExtensions()
{
	return {};
//	if (mVideoExtensions.isEmpty()) {
//		av_register_all();
//
//		AVInputFormat *fmt = av_iformat_next(nullptr); // first format
//		while (fmt != nullptr)
//		{
//			auto exts = QString::fromUtf8(fmt->extensions);
//
//			if (!exts.isEmpty())
//				mVideoExtensions << exts.split(",");
//
//			fmt = av_iformat_next(fmt); // next format
//		}
//	}
//	return mVideoExtensions;
}


const QStringList napkin::ResourceFactory::getFontExtensions()
{
	if (mFontExtensions.isEmpty())
	{
		mFontExtensions << "ttf" << "ttc" << "otf" << "otc" << "pfa" << "pfb";
	}
	return mFontExtensions;
}
