#pragma once

template<typename T, typename Arg>
T* rtti_cast(Arg object) {
    if (!object)
        return nullptr;
    if (object->get_type().template is_derived_from<T>())
        return static_cast<T*>(object);
    return nullptr;
};