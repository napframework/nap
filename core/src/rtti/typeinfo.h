#pragma once

#include "rtti/base/coreprerequisites.h"
#include "rtti/base/typetraits.h"
#include <functional>
#include <string>
#include <vector>
#include <memory>
#include <map>
#include <cstring>
#include <functional>

namespace RTTI
{
	class TypeInfo;
	using CreateFunction = std::function<void*()>;

	namespace impl
	{
		/*!
		 * \brief Register the type info for the given name
		 *
		 * \remark When a type with the given name is already registered,
		 *         then the TypeInfo for the already registered type will be returned.
		 *
		 * \return A valid TypeInfo object.
		 */
		RTTI_API TypeInfo registerOrGetType(const char* name, const TypeInfo& rawTypeInfo,
											const std::vector<TypeInfo>& info, CreateFunction createFunction);

		RTTI_API TypeInfo getType(const char* name);

		template <typename T, bool>
		struct RawTypeInfo;
	} // end namespace impl

	/*!
	 * This class holds the type information for any arbitrary object.
	 *
	 * Every class or atomic data type can have an unique TypeInfo object.
	 * With the help of this object you can compare unknown types for equality at runtime.
	 *
	 * Preparation
	 * -----------
	 * Before you can retrieve data from TypeInfo, you have to register your struct or class.
	 * Therefore use the macro #RTTI_DECLARE_META_TYPE(Type) to make the type known to the TypeInfo system.
	 * To actual execute the registration process use the macro #RTTI_DEFINE_META_TYPE(Type) in global namespace.
	 *
	 * This example shows a typical usage:
	\code{.cpp}

	  // MyClass.h
	  class MyClass
	  {
		int i;
	  };

	  RTTI_DECLARE_META_TYPE(MyClass)

	  // MyClass.cpp; in global namespace
	  RTTI_DEFINE_META_TYPE(myClass)
	\endcode
	 *
	 * Retrieve %TypeInfo
	 * ------------------
	 * There are three static template member functions for retrieving the TypeInfo:
	 *
	 *  TypeInfo::get<T>()
		  \code{.cpp}
			TypeInfo::get<int>() == TypeInfo::get<int>()  // yields to true
			TypeInfo::get<int>() == TypeInfo::get<bool>() // yields to false
		  \endcode
	 *  TypeInfo::get<T>(T* ptr)
		 \code{.cpp}

			struct Base {};
			struct Derived : Base {};
			struct Other : Base {};
			Derived d;
			Base* base = &d;
			TypeInfo::get<Derived>() == TypeInfo::get(base) // yields to true

			Other o;
			base = &o;
			TypeInfo::get<Derived>() == TypeInfo::get(base) // yields to false
		 \endcode
	 *  TypeInfo::get<T>(T& ref)
		 \code{.cpp}
			Derived d;
			TypeInfo::get<Derived>() == TypeInfo::get(d) // yields to true
		 \endcode
	 *
	 */
	class RTTI_API TypeInfo
	{
	  public:
		typedef uint16 TypeId;
        
		/*!
		 * \brief Assigns a TypeInfo to another one.
		 *
		 */
		TypeInfo(const TypeInfo& other);


		/*!
		 * \brief Assigns a TypeInfo to another one.
		 *
		 * \return A TypeInfo object.
		 */
		TypeInfo& operator=(const TypeInfo& other);


		/*!
		 * \brief Comparison operator for sorting the TypeInfo data according to some internal criterion.
		 *
		 * \return True if this TypeInfo is less than the \a other.
		 */
		bool operator<(const TypeInfo& other) const;


		/*!
		 * \brief Comparison operator for sorting the TypeInfo data according to some internal criterion.
		 *
		 * \return True if this TypeInfo is greater than the \a other.
		 */
		bool operator>(const TypeInfo& other) const;


		/*!
		 * \brief Comparison operator for sorting the TypeInfo data according to some internal criterion.
		 *
		 * \return True if this TypeInfo is greater than or equal to \a other.
		 */
		bool operator>=(const TypeInfo& other) const;


		/*!
		 * \brief Comparison operator for sorting the TypeInfo data according to some internal criterion.
		 *
		 * \return True if this TypeInfo is less than or equal to \a other.
		 */
		bool operator<=(const TypeInfo& other) const;


		/*!
		 * \brief Compares this TypeInfo with the \a other TypeInfo and returns true
		 *        if both describe the same type, otherwise returns false.
		 *
		 * \return True if both TypeInfo are equal, otherwise false.
		 */
		bool operator==(const TypeInfo& other) const;


		/*!
		 * \brief Compares this TypeInfo with the \a other TypeInfo and returns true
		 *        if both describe different types, otherwise returns false.
		 *
		 * \return True if both TypeInfo are \b not equal, otherwise false.
		 */
		bool operator!=(const TypeInfo& other) const;


		/*!
		 * \brief Returns the id of this type.
		 *
		 * \note This id is unique at process runtime,
		 *       but the id can be changed every time the process is executed.
		 *
		 * \return The TypeInfo id.
		 */
		TypeId getId() const;


		/*!
		 * \brief Returns the unique and human-readable name of the type.
		 *
		 * \return TypeInfo name.
		 */
		std::string getName() const;


		/*!
		 * \brief Returns true if this TypeInfo is valid, that means the TypeInfo holds valid data to a type.
		 *
		 * \return True if this TypeInfo is valid, otherwise false.
		 */
		bool isValid() const;


		/*!
		 * \brief Returns true if this TypeInfo is derived from the given type \a T, otherwise false.
		 *
		 * \return Returns true if this TypeInfo is a derived type from \a T, otherwise false.
		 */
		template <typename T>
		bool isKindOf() const;


		/*!
		* \brief Returns true if this TypeInfo is derived from the given TypeInfo \a other, otherwise false.
		*
		* \return Returns true if this TypeInfo is a derived type from \a other, otherwise false.
		*/
		bool isKindOf(const TypeInfo& other) const;


		/*!
		 * \brief Returns a TypeInfo object which represent the raw type.
		 *
		 * That means a the type without any qualifiers (const and volatile) nor any pointer.
		 *
		 * \remark When the current TypeInfo is already the raw type, it will return an copy from itself.
		 *
		 * \return The TypeInfo of the raw type.
		 */
		TypeInfo getRawType() const;


        bool canCreateInstance() const;

		/*!
		 * Create instance of the type using a default (no arguments) constructor
		 */
		void* createInstance() const;


		/*!
		 * Create instance of the type using a default (no arguments) constructor
		 */
		template <typename T>
		T* createInstance() const
		{
            return isKindOf<T>() ? static_cast<T*>(createInstance()) : nullptr;
		}


		/*!
		 * \brief Returns a TypeInfo object for the given template type \a T.
		 *
		 * \return TypeInfo for the template type \a T.
		 */
		template <typename T>
		static TypeInfo get();


		/*!
		* \brief creates a new instance for the given template type \a T.
		*
		* \return TypeInfo for the template type \a T.
		*/
		template <typename T>
		static T* create();

		/*!
		* \brief creates a new instance for the given type with name\a T.
		*
		* \return new instance of that type\a T.
		*/
		static void* create(const std::string& name);


		/*!
		 * \brief creates a new instance for the given type with name\a T.
		 *
		 * \return new instance of that type\a T.
		 */
		template <typename T>
		static T* create(const std::string& name);


		/*!
		 * \brief Returns a TypeInfo object for the given instance \a object.
		 *
		 * \remark If the type of the expression is a cv-qualified type, the result of the TypeInfo::get expression
		 * refers to a
		 *         TypeInfo object representing the cv-unqualified type.
		 *
		 * \return TypeInfo for an \a object of type \a T.
		 */
		template <typename T>
		static TypeInfo get(T* object);


		/*!
		 * \brief Returns a TypeInfo object for the given instance \a object.
		 *
		 * \remark When TypeInfo::get is applied to a glvalue expression whose type is a polymorphic class type,
		 *         the result refers to a TypeInfo object representing the type of the most derived object.
		 \code{.cpp}
		  class D { ... };
		  D d1;
		  const D d2;
		  TypeInfo::get(d1)  == TypeInfo::get(d2);         // yields true
		  TypeInfo::get<D>() == TypeInfo::get<const D>();  // yields true
		  TypeInfo::get<D>() == TypeInfo::get(d2);         // yields true
		  TypeInfo::get<D>() == TypeInfo::get<const D&>(); // yields true
		 \endcode
		 *
		 * \return TypeInfo for an \a object of type \a T.
		 */
		template <typename T>
		static TypeInfo get(T& object);
        
        // Return invalid (empty) type info
        static TypeInfo empty() { return TypeInfo(); }


		/*!
		* \brief Returns a TypeInfo object for the given name of the \a object.
		*
		* \remark if the name isn't bound to a valid type information object, an invalid typeinfo is returned
		*/
		RTTI_INLINE static TypeInfo getByName(const char* object) { return impl::getType(object); }



		/*!
		* \brief Returns a TypeInfo object for the given name of the \a object.
		*
		* \remark if the name isn't bound to a valid type information object, an invalid typeinfo is returned
		*/
		RTTI_INLINE static TypeInfo getByName(const std::string& object) { return impl::getType(object.c_str()); }

        /*!
        * \brief Returns an iterable of raw TypeInfos constrained by @kind
        */
		static std::vector<TypeInfo> getRawTypes(const TypeInfo &kind);

		static std::vector<TypeInfo> getRawTypes();

	private:
		/*!
		 * Constructs an empty and invalid TypeInfo object.
		 */
		TypeInfo();


		/*!
		 * \brief Constructs a valid TypeInfo object.
		 *
		 * \param id The unique id of the data type.
		 */
		TypeInfo(TypeId id);


		/*!
		* \brief Thread safe type registration of object of type T
		*/
		RTTI_API friend TypeInfo impl::registerOrGetType(const char* name, const TypeInfo& rawTypeInfo,
														 const std::vector<TypeInfo>& info,
														 CreateFunction createFunction);
		RTTI_API friend TypeInfo impl::getType(const char* name);
		template <typename T, bool>
		friend struct impl::RawTypeInfo;


		/*!
		* \brief Registered type id
		*/
		TypeId m_id;

	};

} // end namespace RTTI

  // RTTI Hash specialization

namespace std
{
	template<>
	struct hash<RTTI::TypeInfo> {
		size_t operator()(const RTTI::TypeInfo &k) const {
			return hash<int>()(k.getRawType().getId());
		}
	};
}
// Include template definitions
#include "rtti/impl/typeinfo_impl.h"

// Macros
#define RTTI_OF(Type) RTTI::TypeInfo::get<Type>()
