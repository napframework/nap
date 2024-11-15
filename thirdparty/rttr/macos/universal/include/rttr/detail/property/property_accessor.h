/************************************************************************************
*                                                                                   *
*   Copyright (c) 2014, 2015 - 2017 Axel Menzel <info@rttr.org>                     *
*                                                                                   *
*   This file is part of RTTR (Run Time Type Reflection)                            *
*   License: MIT License                                                            *
*                                                                                   *
*   Permission is hereby granted, free of charge, to any person obtaining           *
*   a copy of this software and associated documentation files (the "Software"),    *
*   to deal in the Software without restriction, including without limitation       *
*   the rights to use, copy, modify, merge, publish, distribute, sublicense,        *
*   and/or sell copies of the Software, and to permit persons to whom the           *
*   Software is furnished to do so, subject to the following conditions:            *
*                                                                                   *
*   The above copyright notice and this permission notice shall be included in      *
*   all copies or substantial portions of the Software.                             *
*                                                                                   *
*   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR      *
*   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,        *
*   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE     *
*   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER          *
*   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,   *
*   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE   *
*   SOFTWARE.                                                                       *
*                                                                                   *
*************************************************************************************/

#ifndef RTTR_PROPERTY_ACCESSOR_H_
#define RTTR_PROPERTY_ACCESSOR_H_

namespace rttr
{
namespace detail
{

template<typename T>
struct property_accessor
{
    static bool set_value(T& prop, argument& arg)
    {
        prop = arg.get_value<T>();
        return true;
    }
};

template<typename T>
struct property_accessor<std::reference_wrapper<T>>
{
    static bool set_value(T& prop, argument& arg)
    {
        prop = arg.get_value<std::reference_wrapper<T>>().get();
        return true;
    }
};

template<typename T, std::size_t N>
struct property_accessor<T[N]>
{
    static bool set_value(T (& prop)[N], argument& arg)
    {
        copy_array(arg.get_value<T[N]>(), prop);
        return true;
    }
};

template<typename T>
struct property_accessor<T const*>
{
    static bool set_value(T* prop, argument& arg)
    {
        *prop = *arg.get_value<T*>();
        return true;
    }
};

template<typename T, std::size_t N>
struct property_accessor<T(*)[N]>
{
    static bool set_value(T (* prop)[N], argument& arg)
    {
        copy_array(*arg.get_value<T(*)[N]>(), *prop);
        return true;
    }
};

} // end namespace detail
} // end namespace rttr

#endif // RTTR_PROPERTY_ACCESSOR_H_
