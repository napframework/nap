#include "rttiobject.h"

RTTI_BEGIN_BASE_CLASS(nap::rtti::RTTIObject)
	RTTI_PROPERTY("mID", &nap::rtti::RTTIObject::mID, nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

namespace nap
{
	namespace rtti
	{
		// Note: Even though the RTTIObject constructor is empty, we have to keep it in the CPP. 
		// This is because otherwise this CPP is empty, causing the RTTI registration code above to be optimized away, causing the ID property to not be registered in RTTI.
		RTTIObject::RTTIObject()
		{
		}
	}
}
