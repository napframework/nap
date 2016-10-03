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

#include <rtti/base/staticassert.h>

namespace RTTI
{

	RTTI_INLINE TypeInfo::TypeInfo() : m_id(0) {}

	/////////////////////////////////////////////////////////////////////////////////////////

	RTTI_INLINE TypeInfo::TypeInfo(TypeInfo::TypeId id) : m_id(id) {}

	/////////////////////////////////////////////////////////////////////////////////////////

	RTTI_INLINE TypeInfo::TypeInfo(const TypeInfo& other) : m_id(other.m_id) {}

	/////////////////////////////////////////////////////////////////////////////////////////

	RTTI_INLINE TypeInfo& TypeInfo::operator=(const TypeInfo& other)
	{
		m_id = other.m_id;
		return *this;
	}

	/////////////////////////////////////////////////////////////////////////////////////////

	RTTI_INLINE bool TypeInfo::operator<(const TypeInfo& other) const { return (m_id < other.m_id); }

	/////////////////////////////////////////////////////////////////////////////////////////

	RTTI_INLINE bool TypeInfo::operator>(const TypeInfo& other) const { return (m_id > other.m_id); }

	/////////////////////////////////////////////////////////////////////////////////////////

	RTTI_INLINE bool TypeInfo::operator>=(const TypeInfo& other) const { return (m_id >= other.m_id); }

	/////////////////////////////////////////////////////////////////////////////////////////

	RTTI_INLINE bool TypeInfo::operator<=(const TypeInfo& other) const { return (m_id <= other.m_id); }

	/////////////////////////////////////////////////////////////////////////////////////////

	RTTI_INLINE bool TypeInfo::operator==(const TypeInfo& other) const { return (m_id == other.m_id); }

	/////////////////////////////////////////////////////////////////////////////////////////

	RTTI_INLINE bool TypeInfo::operator!=(const TypeInfo& other) const { return (m_id != other.m_id); }

	/////////////////////////////////////////////////////////////////////////////////////////

	RTTI_INLINE TypeInfo::TypeId TypeInfo::getId() const { return m_id; }

	/////////////////////////////////////////////////////////////////////////////////////////

	RTTI_INLINE bool TypeInfo::isValid() const { return (m_id != 0); }


	/////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////

	namespace impl
	{

		template <typename T>
		struct MetaTypeInfo {
			enum { Defined = 0 };
			static TypeInfo getTypeInfo()
			{
				// when you get this error, you have to declare first the type with this macro
				return T::TYPE_NOT_REGISTERED__USE_RTTI_DECLARE_META_TYPE();
			}
		};

		template <typename T>
		struct AutoRegisterType;

		/////////////////////////////////////////////////////////////////////////////////

		template <typename T>
		static RTTI_INLINE TypeInfo getTypeInfoFromInstance(const T*)
		{
			return impl::MetaTypeInfo<T>::getTypeInfo();
		}

		/////////////////////////////////////////////////////////////////////////////////

		template <typename T, bool = Traits::is_same<T, typename raw_type<T>::type>::value>
		struct RawTypeInfo {
			static RTTI_INLINE TypeInfo get()
			{
				return TypeInfo();
			} // we have to return an empty TypeInfo, so we can stop the recursion
		};

		/////////////////////////////////////////////////////////////////////////////////

		template <typename T>
		struct RawTypeInfo<T, false> {
			static RTTI_INLINE TypeInfo get() { return MetaTypeInfo<typename raw_type<T>::type>::getTypeInfo(); }
		};

		/////////////////////////////////////////////////////////////////////////////////

		/*!
		 * Determine if the given type \a T has the method
		 * 'TypeInfo getTypeInfo() const' declared.
		 */
		template <typename T>
		class has_getTypeInfo_func_impl
		{
			typedef char YesType[1];
			typedef char NoType[2];

			template <typename U, RTTI::TypeInfo (U::*)() const>
			class check
			{
			};

			template <typename C>
			static YesType& f(check<C, &C::getTypeInfo>*);

			template <typename C>
			static NoType& f(...);

		public:
			static const bool value = (sizeof(f<typename raw_type<T>::type>(0)) == sizeof(YesType));
		};

		/*!
		 * If T has a member function 'TypeInfo getTypeInfo() const;' then inherits from true_type, otherwise inherits
		 * from false_type.
		 */
		template <class T, class = void>
		struct has_getTypeInfo_func : Traits::integral_constant<bool, false> {
		};

		template <class T>
		struct has_getTypeInfo_func<T, typename enable_if<has_getTypeInfo_func_impl<T>::value>::type>
			: Traits::integral_constant<bool, true> {
		};

		/////////////////////////////////////////////////////////////////////////////////

		template <typename T, bool>
		struct TypeInfoFromInstance;

		//! Specialization for retrieving the TypeInfo from the instance directly
		template <typename T>
		struct TypeInfoFromInstance<T, false> // the typeInfo function is not available
		{
			static RTTI_INLINE TypeInfo get(T&)
			{
				return impl::MetaTypeInfo<
					typename Traits::remove_cv<typename Traits::remove_reference<T>::type>::type>::getTypeInfo();
			}
		};

		//! Specialization for retrieving the TypeInfo from the instance directly
		template <typename T>
		struct TypeInfoFromInstance<T, true> {
			static RTTI_INLINE TypeInfo get(T& object) { return object.getTypeInfo(); }
		};

	} // end namespace impl

	/////////////////////////////////////////////////////////////////////////////////////////

	template <typename T>
	RTTI_INLINE TypeInfo TypeInfo::get()
	{
		return impl::MetaTypeInfo<
			typename Traits::remove_cv<typename Traits::remove_reference<T>::type>::type>::getTypeInfo();
	}

	/////////////////////////////////////////////////////////////////////////////////////////

	template <typename T>
	RTTI_INLINE TypeInfo TypeInfo::get(T*)
	{
		return impl::MetaTypeInfo<T*>::getTypeInfo();
	}

	/////////////////////////////////////////////////////////////////////////////////////////

	template <typename T>
	RTTI_INLINE TypeInfo TypeInfo::get(T& object)
	{
		return impl::TypeInfoFromInstance<T, impl::has_getTypeInfo_func<T>::value>::get(object);
	}

	/////////////////////////////////////////////////////////////////////////////////////////

	template <typename T>
	RTTI_INLINE bool TypeInfo::isKindOf() const
	{
		return isKindOf(impl::MetaTypeInfo<T>::getTypeInfo());
	}

	/////////////////////////////////////////////////////////////////////////////////////////

	template <typename T>
	RTTI_INLINE T* TypeInfo::create()
	{
		TypeInfo type_info = TypeInfo::get<T>();
		if (!type_info.isValid()) return nullptr;
		return type_info.createInstance<T>();
	}

	/////////////////////////////////////////////////////////////////////////////////////////

	template <typename T>
	RTTI_INLINE T* TypeInfo::create(const std::string& name)
	{
		TypeInfo type_info = TypeInfo::getByName(name);
		if (!type_info.isValid()) return nullptr;
		return type_info.createInstance<T>();
	}



	/////////////////////////////////////////////////////////////////////////////////////////

} // end namespace RTTI

#define RTTI_CAT_IMPL(a, b) a##b
#define RTTI_CAT(a, b) RTTI_CAT_IMPL(a, b)



#define RTTI_DECLARE_META_TYPE(T)                                                                                      \
                                                                                                                       \
	namespace RTTI                                                                                                     \
                                                                                                                       \
	{                                                                                                                  \
		namespace impl                                                                                                 \
		{                                                                                                              \
			template <>                                                                                                \
			struct MetaTypeInfo<T> {                                                                                   \
				enum { Defined = 1 };                                                                                  \
				static RTTI_INLINE RTTI::TypeInfo getTypeInfo()                                                        \
				{                                                                                                      \
					static const TypeInfo val = registerOrGetType(#T, RawTypeInfo<T>::get(),                           \
																  BaseClasses<T>::retrieve(), []() { return new T; }); \
					return val;                                                                                        \
				}                                                                                                      \
			};                                                                                                         \
		}                                                                                                              \
                                                                                                                       \
	} // end namespace RTTI

#define RTTI_DECLARE_META_BASE_TYPE(T)                                                           \
                                                                                                 \
	namespace RTTI                                                                               \
                                                                                                 \
	{                                                                                            \
                                                                                                 \
		namespace impl                                                                           \
                                                                                                 \
		{                                                                                        \
                                                                                                 \
			template <>                                                                          \
                                                                                                 \
			struct MetaTypeInfo<T>                                                               \
                                                                                                 \
			{                                                                                    \
                                                                                                 \
				enum { Defined = 1 };                                                            \
                                                                                                 \
				static RTTI_INLINE RTTI::TypeInfo getTypeInfo()                                  \
                                                                                                 \
				{                                                                                \
                                                                                                 \
					static const TypeInfo val =                                                  \
						registerOrGetType(#T, RawTypeInfo<T>::get(), BaseClasses<T>::retrieve(), \
                                                                                                 \
										  nullptr);                                              \
                                                                                                 \
					return val;                                                                  \
				}                                                                                \
			};                                                                                   \
		}                                                                                        \
                                                                                                 \
	} // end namespace RTTI


#define RTTI_DEFINE_META_TYPE(T)                                       \
                                                                       \
	namespace RTTI                                                     \
                                                                       \
	{                                                                  \
		namespace impl                                                 \
		{                                                              \
			template <>                                                \
			struct AutoRegisterType<T> {                               \
				AutoRegisterType() { MetaTypeInfo<T>::getTypeInfo(); } \
			};                                                         \
		}                                                              \
	}                                                                  \
                                                                       \
	static const RTTI::impl::AutoRegisterType<T> RTTI_CAT(autoRegisterType, __COUNTER__);


//////////////////////////////////////////////////////////////////////////


// Declares an object to have RTTI (RUN TIME TYPE INFO) with create function
// This declare assumes a default constructor used for initialization
#define RTTI_DECLARE(T)        \
	RTTI_DECLARE_META_TYPE(T)  \
	RTTI_DECLARE_META_TYPE(T*) \
	RTTI_DECLARE_META_TYPE(const T*)
	
// Declares an object to have RTTI (RUN TIME TYPE INFO) WITHOUT create function
// This works for objects without a default construction.
#define RTTI_DECLARE_BASE(T)        \
	RTTI_DECLARE_META_BASE_TYPE(T*) \
	RTTI_DECLARE_META_BASE_TYPE(T)  \
	RTTI_DECLARE_META_BASE_TYPE(const T*)

// Declares an object to be an attribute, together with the associated run time type information
#define RTTI_DECLARE_DATA(T)            \
	RTTI_DECLARE(T)                     \
	RTTI_DECLARE(nap::Attribute<T>)		\
	RTTI_DECLARE(nap::Attribute<T*>)    \
    RTTI_DECLARE(nap::ArrayAttribute<T>)

// Declares an object to be a numeric attribute, together with the associated run time type information
#define RTTI_DECLARE_NUMERIC_DATA(T)	\
	RTTI_DECLARE_DATA(T)				\
	RTTI_DECLARE(nap::NumericAttribute<T>)	\
	RTTI_DECLARE(nap::NumericAttribute<T*>)	\

// Defines an RTTI object with create function
#define RTTI_DEFINE(T)        \
	RTTI_DEFINE_META_TYPE(T)  \
	RTTI_DEFINE_META_TYPE(T*) \
	RTTI_DEFINE_META_TYPE(const T*)

// Defines an object to be an attribute, together with the associated run time type information
#define RTTI_DEFINE_DATA(T) \
	RTTI_DEFINE(T)          \
	RTTI_DEFINE(nap::Attribute<T>) \
	RTTI_DEFINE(nap::Attribute<T*>) \
    RTTI_DEFINE(nap::ArrayAttribute<T>)


// Defines an object to be a numeric attribute, together with the associated run time type information
#define RTTI_DEFINE_NUMERIC_DATA(T) \
	RTTI_DEFINE_DATA(T)	\
	RTTI_DEFINE(nap::NumericAttribute<T>)	\
	RTTI_DEFINE(nap::NumericAttribute<T*>)	\

