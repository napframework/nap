#pragma once

#include <rttr/type>
#include <rttr/registration>

namespace RTTI
{
	using TypeInfo = rttr::type;
}

// Macros
#define RTTI_OF(Type) rttr::type::get<Type>()

// Declares an object to have RTTI (RUN TIME TYPE INFO) with create function
// This declare assumes a default constructor used for initialization
#define RTTI_DECLARE(T)

// Declares an object to have RTTI (RUN TIME TYPE INFO) WITHOUT create function
// This works for objects without a default construction.
#define RTTI_DECLARE_BASE(T)

// Defines an RTTI object with create function
#define CONCAT_UNIQUE_NAMESPACE(x, y)				namespace x##y
#define UNIQUE_REGISTRATION_NAMESPACE(id)			CONCAT_UNIQUE_NAMESPACE(__rtti_registration_, id)

#define RTTI_BEGIN_CLASS(Type)							\
	UNIQUE_REGISTRATION_NAMESPACE(__COUNTER__)			\
	{													\
		RTTR_REGISTRATION								\
		{												\
			using namespace rttr;						\
			registration::class_<Type>(#Type)			\

#define RTTI_CONSTRUCTOR								\
					  .constructor<>()(policy::ctor::as_raw_ptr)

#define RTTI_END_CLASS									\
		;												\
		}												\
	}													\

#define RTTI_DEFINE(Type)							\
	RTTI_BEGIN_CLASS(Type)							\
		RTTI_CONSTRUCTOR							\
	RTTI_END_CLASS

#define RTTI_DEFINE_BASE(Type)						\
	RTTI_BEGIN_CLASS(Type)							\
	RTTI_END_CLASS

#define RTTI_ENABLE(...) \
	RTTR_ENABLE(__VA_ARGS__) \
	RTTR_REGISTRATION_FRIEND

#define RTTI_ENABLE_DERIVED_FROM(...) \
	RTTI_ENABLE(__VA_ARGS__)