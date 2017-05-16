#include "rttiobject.h"

RTTI_BEGIN_BASE_CLASS(rtti::RTTIObject)
	RTTI_PROPERTY("mID", &rtti::RTTIObject::mID, rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

// Note: Even though the RTTIObject constructor is empty, we have to keep it in the CPP. 
// This is because otherwise this CPP is empty, causing the RTTI registration code above to be optimized away, causing the ID property to not be registered in RTTI.
namespace rtti
{
	RTTIObject::RTTIObject()
	{
	}
}
