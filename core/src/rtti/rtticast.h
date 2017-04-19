#pragma once

template<typename T, typename Arg>
T* rtti_cast(Arg object) {
    if (!object)
        return nullptr;
    if (object->getTypeInfo().template isKindOf<T>())
        return static_cast<T*>(object);
    return nullptr;
};