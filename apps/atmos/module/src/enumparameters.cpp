#include "enumparameters.h"

RTTI_BEGIN_ENUM(nap::EControlMethod)
	RTTI_ENUM_VALUE(nap::EControlMethod::Orbit, "Orbit"),
	RTTI_ENUM_VALUE(nap::EControlMethod::FirstPerson, "FirstPerson"),
	RTTI_ENUM_VALUE(nap::EControlMethod::Path, "Path")
RTTI_END_ENUM

DEFINE_ENUM_PARAMETER(nap::ParameterControlMethod);
DEFINE_ENUM_PARAMETER(nap::ParameterPolygonMode);