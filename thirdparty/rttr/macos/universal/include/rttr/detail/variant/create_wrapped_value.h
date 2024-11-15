#ifndef RTTR_CREATE_WRAPPED_VALUE_H_
#define RTTR_CREATE_WRAPPED_VALUE_H_

#include "rttr/detail/misc/misc_type_traits.h"
#include "rttr/detail/misc/std_type_traits.h"
#include "rttr/detail/impl/wrapper_mapper_impl.h"
#include "rttr/argument.h"

namespace rttr
{
namespace detail
{

template<typename T>
using is_copyable = std::is_copy_constructible<T>;

/////////////////////////////////////////////////////////////////////////////////////////

template<typename T, typename Tp = decay_except_array_t<wrapper_mapper_t<T>> >
enable_if_t<is_copyable<Tp>::value &&
	is_wrapper<T>::value, variant> create_wrapped_value(argument& value)
{
	using raw_wrapper_type = remove_cv_t<remove_reference_t<T>>;
	using wrapped_type = typename wrapper_mapper<raw_wrapper_type>::wrapped_type;

	if (value.is_type<wrapped_type>())
		return variant(wrapper_mapper<raw_wrapper_type>::create(value.get_value<wrapped_type>()));
	else
		return variant();		
}

template<typename T, typename Tp = decay_except_array_t<wrapper_mapper_t<T>>>
enable_if_t<!is_copyable<Tp>::value ||
	!is_wrapper<T>::value, variant> create_wrapped_value(argument& value)
{
	return variant();
}

/////////////////////////////////////////////////////////////////////////////////////////

} // end namespace detail
} // end namespace rttr

#endif // RTTR_CREATE_WRAPPED_VALUE_H_
