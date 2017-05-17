#pragma once

// Local Includes
#include "attribute.h"
#include "arrayattribute.h"
#include "compoundattribute.h"
#include "plug.h"

/**
 * RTTI macro's bound to NAP, file separation ensures
 * the headers are self contained, ie, don't need
 * other headers to function properly. When including
 * this header all other RTTI functionality is also
 * exposed.
 */

// Defines an object to be an attribute, together with the associated run time type information
#define RTTI_DEFINE_DATA(T)             \
	RTTI_DEFINE(T)                      \
	RTTI_DEFINE(nap::Attribute<T>)      \
    RTTI_DEFINE(nap::ArrayAttribute<T>) \
	RTTI_DEFINE(nap::OutputPullPlug<T>) \
	RTTI_DEFINE(nap::InputPullPlug<T>)


// Defines an object to be a numeric attribute, together with the associated run time type information
#define RTTI_DEFINE_NUMERIC_DATA(T) \
	RTTI_DEFINE_DATA(T)             \
	RTTI_DEFINE(nap::NumericAttribute<T>)

namespace nap
{
	rtti::TypeInfo getAttributeTypeFromValueType(const rtti::TypeInfo& valueType);
	rtti::TypeInfo getOutpullPlugFromValueType(const rtti::TypeInfo& valueType);
    rtti::TypeInfo getInputPullPlugFromValueType(const rtti::TypeInfo& valueType);
}