#pragma once

/*!
 * \brief Casts the given \a object to type \a T.
 *
 * Returns the given object cast to type T if the object is of type T (or of a subclass); 
 * otherwise returns 0. If object is 0 then it will also return 0.
 *
 * \return
 */
template<typename T, typename Arg>
T rttr_cast(Arg object);

#include "rtti/impl/rtticast_impl.h"
