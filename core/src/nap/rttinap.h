#pragma once

// Local Includes
#include "attribute.h"
#include "arrayattribute.h"

/**
 * RTTI macro's bound to NAP, file separation ensures
 * the headers are self contained, ie, don't need
 * other headers to function properly. When including
 * this header all other RTTI functionality is also 
 * exposed.
 */

 // Declares an object to be an attribute, together with the associated run time type information
#define RTTI_DECLARE_DATA(T)            \
	RTTI_DECLARE(T)                     \
	RTTI_DECLARE(nap::Attribute<T>)		\
	RTTI_DECLARE(nap::Attribute<T*>)    \
    RTTI_DECLARE(nap::ArrayAttribute<T>)

 // Declares an object to be a numeric attribute, together with the associated run time type information
#define RTTI_DECLARE_NUMERIC_DATA(T)	\
	RTTI_DECLARE_DATA(T)				\
	RTTI_DECLARE(nap::NumericAttribute<T>)	\

 // Defines an object to be an attribute, together with the associated run time type information
#define RTTI_DEFINE_DATA(T) \
	RTTI_DEFINE(T)          \
	RTTI_DEFINE(nap::Attribute<T>) \
	RTTI_DEFINE(nap::Attribute<T*>) \
    RTTI_DEFINE(nap::ArrayAttribute<T>)


 // Defines an object to be a numeric attribute, together with the associated run time type information
#define RTTI_DEFINE_NUMERIC_DATA(T) \
	RTTI_DEFINE_DATA(T)	\
	RTTI_DEFINE(nap::NumericAttribute<T>)	\



