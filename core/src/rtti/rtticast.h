#pragma once

template<typename T, typename Arg>
T* rtti_cast(Arg object) {
    if (!object)
        return nullptr;
    if (object->get_type().template is_derived_from<T>())
        return reinterpret_cast<T*>(object);
    return nullptr;
};

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