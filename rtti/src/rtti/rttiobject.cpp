#include "rttiobject.h"

static const char* sIDPropertyName = "mID";

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::rtti::RTTIObject)
	RTTI_PROPERTY(sIDPropertyName, &nap::rtti::RTTIObject::mID, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

namespace nap
{
	namespace rtti
	{
		bool RTTIObject::isIDProperty(rtti::Instance& object, const rtti::Property& property)
		{
			return object.get_derived_type().is_derived_from<RTTIObject>() && std::string(property.get_name().data()) == sIDPropertyName;
		}

		// Note: Even though the RTTIObject constructor is empty, we have to keep it in the CPP. 
		// This is because otherwise this CPP is empty, causing the RTTI registration code above to be optimized away, causing the ID property to not be registered in RTTI.
		RTTIObject::RTTIObject()
		{
		}
	}
}
