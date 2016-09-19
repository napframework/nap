#pragma once

namespace RTTI
{

namespace impl
{
    template<class T>
    struct raw_type { typedef T type; };

    template<class T> struct raw_type<const T>      { typedef typename raw_type<T>::type type; };

    template<class T> struct raw_type<T*>           { typedef typename raw_type<T>::type type; };
    template<class T> struct raw_type<T* const>     { typedef typename raw_type<T>::type type; };
    template<class T> struct raw_type<T* volatile>  { typedef typename raw_type<T>::type type; };

    template<class T> struct raw_type<T&>           { typedef typename raw_type<T>::type type; };
} // end namespace impl

} // end namespace RTTI
