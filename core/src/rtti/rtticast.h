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

template<typename T, typename Arg>
T* rtti_cast(Arg object) {
    if (!object)
        return nullptr;
    if (object->getTypeInfo().template isKindOf<T>())
        return static_cast<T*>(object);
    return nullptr;
};


#include "rtti/impl/rtticast_impl.h"
