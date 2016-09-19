#pragma once

#include "rtti/base/typetraits.h"
#include "rtti/base/staticassert.h"

namespace RTTI
{
namespace impl
{
//! A simple typelist
struct nill {};
template<class T, class U = nill> struct typelist
{
    typedef T head;
    typedef U tail;
};

/////////////////////////////////////////////////////////////////////////////////////
/*!
 * This trait checks if a given type T has a typedef named \a baseClassList.
 * has_base_class_list_impl::value is true, when it has this type, otherwise false.
 */
template <typename T>
class has_base_class_list_impl
{
    typedef char YesType[1];
    typedef char NoType[2] ;

    template <typename C>
    static YesType& test(typename C::baseClassList*);

    template <typename>
    static NoType& test(...);

public:
    static const bool value = (sizeof(YesType) == sizeof(test<T>(0)));
};

/*!
 * If T has a typedef called \a 'baseClassList' then inherits from true_type, otherwise inherits from false_type. 
 */
template<class T, class = void>
struct has_base_class_list : Traits::integral_constant<bool, false> 
{};

template<class T>
struct has_base_class_list<T, typename enable_if<has_base_class_list_impl<T>::value>::type > : Traits::integral_constant<bool, true>
{};

/*!
 * This class fills from a given typelist the corresponding TypeInfo objects into a std::vector.
 */
template<class> 
struct TypeInfoFromBaseClassList;

template<>
struct TypeInfoFromBaseClassList<typelist<nill> > 
{
    static RTTI_INLINE void fill(std::vector<TypeInfo>&) 
    { 
    }
};

template<class T, class U> 
struct TypeInfoFromBaseClassList<typelist<T, U> > 
{
    static RTTI_INLINE void fill(std::vector<TypeInfo>& v)
    {
        RTTI_STATIC_ASSERT(has_base_class_list<T>::value, PARENT_CLASS_HAS_NO_BASE_CLASS_LIST_DEFINIED__USE_RTTI_ENABLE);
        v.push_back(MetaTypeInfo<T>::getTypeInfo());
        // retrieve also the TypeInfo of all base class of the base classes
        TypeInfoFromBaseClassList<typename T::baseClassList>::fill(v);
        TypeInfoFromBaseClassList<U>::fill(v);
    }
};

/*!
 * This helper trait returns a vector with TypeInfo object of all base classes.
 * When there is no typelist defined or the class has no base class, an empty vector is returned.
 */
template<class T>
struct BaseClasses
{
    private:
        // extract the info
        static RTTI_INLINE void retrieve_impl(std::vector<TypeInfo>& v, Traits::true_type)
        {
            TypeInfoFromBaseClassList<typename T::baseClassList>::fill(v);
        }

        // no type list defined
        static RTTI_INLINE void retrieve_impl(std::vector<TypeInfo>&, Traits::false_type)
        {
        }
    public:
        static RTTI_INLINE std::vector<TypeInfo> retrieve()
        {
            std::vector<TypeInfo> result;
            retrieve_impl(result, typename has_base_class_list<T>::type());
            return result;
        }
};

} // end namespace impl
} // end namespace RTTI

#define TYPE_LIST()             RTTI::impl::typelist<RTTI::impl::nill>
#define TYPE_LIST_1(A)          RTTI::impl::typelist<A, TYPE_LIST() >
#define TYPE_LIST_2(A,B)        RTTI::impl::typelist<A, TYPE_LIST_1(B) >
#define TYPE_LIST_3(A,B,C)      RTTI::impl::typelist<A, TYPE_LIST_2(B,C) >
#define TYPE_LIST_4(A,B,C,D)    RTTI::impl::typelist<A, TYPE_LIST_3(B,C,D) >
#define TYPE_LIST_5(A,B,C,D,E)  RTTI::impl::typelist<A, TYPE_LIST_4(B,C,D,E) >

#define RTTI_ENABLE() \
public:\
    virtual RTTI_INLINE RTTI::TypeInfo getTypeInfo() const { return RTTI::impl::getTypeInfoFromInstance(this); }  \
    typedef TYPE_LIST() baseClassList;\
private:

#define RTTI_ENABLE_DERIVED_FROM(A) \
public:\
    virtual RTTI_INLINE RTTI::TypeInfo getTypeInfo() const override { return RTTI::impl::getTypeInfoFromInstance(this); }  \
    typedef TYPE_LIST_1(A) baseClassList;\
private:

#define RTTI_ENABLE_DERIVED_FROM_2(A,B) \
public:\
    virtual RTTI_INLINE RTTI::TypeInfo getTypeInfo() const override { return RTTI::impl::getTypeInfoFromInstance(this); }  \
    typedef TYPE_LIST_2(A,B) baseClassList;\
private:

#define RTTI_ENABLE_DERIVED_FROM_3(A,B,C) \
public:\
    virtual RTTI_INLINE RTTI::TypeInfo getTypeInfo() const override { return RTTI::impl::getTypeInfoFromInstance(this); }  \
    typedef TYPE_LIST_3(A,B,C) baseClassList;\
private:

#define RTTI_ENABLE_DERIVED_FROM_4(A,B,C,D) \
public:\
    virtual RTTI_INLINE RTTI::TypeInfo getTypeInfo() const override { return RTTI::impl::getTypeInfoFromInstance(this); }  \
    typedef TYPE_LIST_4(A,B,C,D) baseClassList;\
private:

#define RTTI_ENABLE_DERIVED_FROM_5(A,B,C,D,E) \
public:\
    virtual RTTI_INLINE RTTI::TypeInfo getTypeInfo() const override { return RTTI::impl::getTypeInfoFromInstance(this); }  \
    typedef TYPE_LIST_5(A,B,C,D,E) baseClassList;\
private:
