#include "resourcefactory.h"

napkin::ResourceFactory::ResourceFactory()
{
	mObjectIconMap = {{RTTI_OF(nap::rtti::RTTIObject), ":/icons/bullet_white.png"}};
}


QIcon napkin::ResourceFactory::iconFor(const nap::rtti::RTTIObject& object) const
{
	for (auto factory_type : mObjectIconMap.keys())
	{
		rttr::type obj_type = object.get_type();
		if (obj_type.is_derived_from(factory_type))
			return QIcon(mObjectIconMap[factory_type]);
	}

	return QIcon();
}
