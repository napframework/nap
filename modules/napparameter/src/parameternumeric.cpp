#include "parameternumeric.h"

#define DEFINE_NUMERIC_PARAMETER(Type)																			\
	RTTI_BEGIN_CLASS(Type)																						\
		RTTI_PROPERTY("Value",		&Type::mValue,			nap::rtti::EPropertyMetaData::Default)				\
		RTTI_PROPERTY("Minimum",	&Type::mMinimum,		nap::rtti::EPropertyMetaData::Default)				\
		RTTI_PROPERTY("Maximum",	&Type::mMaximum,		nap::rtti::EPropertyMetaData::Default)				\
	RTTI_END_CLASS

DEFINE_NUMERIC_PARAMETER(nap::ParameterFloat)
DEFINE_NUMERIC_PARAMETER(nap::ParameterDouble)
DEFINE_NUMERIC_PARAMETER(nap::ParameterLong)
DEFINE_NUMERIC_PARAMETER(nap::ParameterInt)
DEFINE_NUMERIC_PARAMETER(nap::ParameterChar)
DEFINE_NUMERIC_PARAMETER(nap::ParameterByte)

DEFINE_NUMERIC_PARAMETER(nap::ParameterVec2)
DEFINE_NUMERIC_PARAMETER(nap::ParameterIVec2)
DEFINE_NUMERIC_PARAMETER(nap::ParameterVec3)

