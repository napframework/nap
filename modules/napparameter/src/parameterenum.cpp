#include <parameterenum.h>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::ParameterEnumBase)
RTTI_END_CLASS

namespace nap
{
	ParameterEnumBase::ParameterEnumBase(rtti::TypeInfo enumType) :
		mEnumType(enumType)
	{
	}
}