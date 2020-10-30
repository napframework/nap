/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

/**
 * Cast a pointer to another pointer when the given pointer is derived from T.
 * @param object pointer to the object to cast.
 * @return the cast object, nullptr if the object is not derived from requested type.
 */
template<typename T, typename Arg>
T* rtti_cast(Arg object) 
{
    if (!object)
        return nullptr;
    if (object->get_type().template is_derived_from<T>())
        return reinterpret_cast<T*>(object);
    return nullptr;
};

/**
 * Cast a unique ptr to another unique ptr when the given pointer is derived from 'Derived'.
 * When the object can be cast the the original pointer is released. 
 * @param object unique ptr to the object to cast.
 * @return the cast object as a unique ptr, nullptr if the object is not derived from requested type.
 */
template<typename Derived, typename Base>
std::unique_ptr<Derived> rtti_cast(std::unique_ptr<Base>& pointer)
{
	if (Derived *result = rtti_cast<Derived>(pointer.get()))
	{
		pointer.release();
		return std::unique_ptr<Derived>(result);
	}
	return std::unique_ptr<Derived>(nullptr);
}