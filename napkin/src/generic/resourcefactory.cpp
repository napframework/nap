#include "resourcefactory.h"
#include <entity.h>
#include <scene.h>

napkin::ResourceFactory::ResourceFactory()
{
	mObjectIconMap = {
		{RTTI_OF(nap::Entity), ":/icons/cube-blue.png"},
		{RTTI_OF(nap::Scene), ":/icons/bricks.png"},
		{RTTI_OF(nap::Component), ":/icons/diamond-orange.png"},
		{RTTI_OF(nap::rtti::RTTIObject), ":/icons/bullet_white.png"},
	};
}


QIcon napkin::ResourceFactory::iconFor(const nap::rtti::RTTIObject& object) const
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
