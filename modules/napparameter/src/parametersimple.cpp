#include <parametersimple.h>

#define DEFINE_SIMPLE_PARAMETER(Type)																			\
	RTTI_BEGIN_CLASS(Type)																						\
		RTTI_PROPERTY("Value",		&Type::mValue,		nap::rtti::EPropertyMetaData::Default)					\
	RTTI_END_CLASS

DEFINE_SIMPLE_PARAMETER(nap::ParameterBool)
DEFINE_SIMPLE_PARAMETER(nap::ParameterRGBColorFloat)

namespace nap
{
}