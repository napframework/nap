/************************************************************************************
*                                                                                   *
*   Copyright (c) 2013 Axel Menzel <info@axelmenzel.de>                             *
*                                                                                   *
*   This file is part of the Runtime Type Reflection System (RTTI).                 *
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

#include "rtti/typeinfo.h"
#include "rtti/base/typetraits.h"
#include "rtti/base/staticassert.h"

template<typename T, typename Arg>
RTTI_INLINE T rttr_cast(Arg object)
{
    using namespace RTTI::Traits;

    RTTI_STATIC_ASSERT(is_pointer<T>::value, RETURN_TYPE_MUST_BE_A_POINTER);
    RTTI_STATIC_ASSERT(is_pointer<Arg>::value, ARGUMENT_TYPE_MUST_BE_A_POINTER);
    RTTI_STATIC_ASSERT(RTTI::impl::has_getTypeInfo_func<Arg>::value, CLASS_HAS_NO_TYPEINFO_DEFINIED__USE_MACRO_ENABLE_RTTI);

    typedef typename remove_pointer<T>::type ReturnType;
    typedef typename remove_pointer<Arg>::type ArgType;
    RTTI_STATIC_ASSERT( (is_const<ArgType>::value && is_const<ReturnType>::value) ||
                        (!is_const<ArgType>::value && is_const<ReturnType>::value) ||
                        (!is_const<ArgType>::value && !is_const<ReturnType>::value), RETURN_TYPE_MUST_HAVE_CONST_QUALIFIER);
    if (object && object->getTypeInfo().template isTypeDerivedFrom<T>())
        return static_cast<T>(object);
    else
        return NULL;
}
