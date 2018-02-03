#include "resourcefactory.h"
#include <entity.h>
#include <scene.h>
#include <FreeImage.h>
#include <nap/logger.h>

using namespace nap;
using namespace nap::rtti;
koek
/**
 * Simply get all file extensions registered by FreeImage
 * @return A list of file extensions
 */
QStringList getFreeImageExtensions()
{
	QStringList extensions;
	for (int i=0, len=FreeImage_GetFIFCount(); i < len; i++)
	{
		const char* exts = FreeImage_GetFIFExtensionList(static_cast<FREE_IMAGE_FORMAT>(i));
		for (auto ext : QString::fromUtf8(exts).split(","))
		{
			extensions << ext;
		}
	}
	return extensions;
}


napkin::ResourceFactory::ResourceFactory()
{
	mObjectIconMap = {
			{RTTI_OF(Entity),     ":/icons/cube-blue.png"},
			{RTTI_OF(Scene),      ":/icons/bricks.png"},
			{RTTI_OF(Component),  ":/icons/diamond-orange.png"},
			{RTTI_OF(RTTIObject), ":/icons/bullet_white.png"},
	};

	mFileTypes = {
			{EPropertyFileType::Image,      "Image Files",      getFreeImageExtensions()},
			{EPropertyFileType::FragShader, "Fragment Shaders", {"frag"}},
			{EPropertyFileType::VertShader, "Vertex Shaders",   {"vert"}},
			{EPropertyFileType::Python,     "Python Files",     {"py"}},
	};
}


QIcon napkin::ResourceFactory::getIcon(const nap::rtti::RTTIObject& object) const
{
	for (auto entry : mObjectIconMap)
	{
		rttr::type obj_type = object.get_type();
		if (obj_type.is_derived_from(entry.first))
		{
			QIcon icon(entry.second);
			return icon;
		}
	}

	return QIcon();
}


QString napkin::ResourceFactory::getFileFilter(const nap::rtti::Property& prop) const
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
		if (nap::rtti::hasFiletypeFlag(prop, ftype.mFileType))
			return ftype;
	}
	return mAnyFileType;
}
