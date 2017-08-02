#pragma once

#include <rttr/type>
#include <rttr/registration>
#include "rtti/pythonmodule.h"
#include "utility/dllexport.h"

/**
 * This file contains the macros necessary to register types and their attributes with the RTTI system. There are only a few macros important for the user of the RTTI system:
 * - RTTI_OF - This is a convenience macro used to get the underlying TypeInfo of the named type. Usage example: RTTI_OF(rtti::RTTIObject).
 * - RTTI_ENABLE - This macro must be used when you have a class that is part of an inheritance hierarchy. The argument to the macro is a comma-separated list of base classes (empty if the macro is being used in the base class itself).
 * - RTTI_BEGIN_CLASS, RTTI_END_CLASS, RTTI_PROPERTY - These macros are used to register a type in the RTTI system and must be placed in a .cpp file.
 * - RTTI_DEFINE_CLASS/RTTI_DEFINE_BASE - Wrapper around RTTI_BEGIN_CLASS/RTTI_END_CLASS for backwards compatibility
 * - RTTI_BEGIN_ENUM/RTTI_END_ENUM - These macros are used to register an enum in the RTTI system and must be placed in a .cpp file
 *
 * See the following example for a typical usage scenario of these macros:
 *
  *		enum class ETestEnum
 *		{
 *			One,
 *			Two,
 *			Three,
 *			Four
 *		};
 *
 *		// RTTIClasses.h
 *		struct DataStruct
 *		{
 *				float		mFloatProperty;
 *				std::string mStringProperty;
 *				ETestEnum	mEnumProperty;
 *		};
 *
 *		class BaseClass
 *		{
 *				RTTI_ENABLE()
 *		private:
 *				float		mFloatProperty;
 *		};
 *
 *		class DerivedClass : public BaseClass
 *		{
 *				RTTI_ENABLE(SomeBaseClass)
 *
 *		private:
 *				int			mIntProperty;
 *		};
 *
 * The above code defines four new types:
 * - ETestEnum
 *		An enum that will be used in one of the other RTTI classes
 *
 * - DataStruct:
 *      A class without base or derived classes. Note that the RTTI_ENABLED macro is not used for this class.
 *      The fact that the RTTI_ENABLED macro is optional (it's only required when the class is part of an inheritance hierarchy) makes it possible to *add* RTTI to third party classes, since no modification of the class itself is required.
 *		This is also very efficient, because when RTTI_ENABLED is not used, no vtable is required in the class.
 *      A good example of this is adding RTTI support to classes such as glm::vec2, glm::vec3, etc; types that we have no control over, but we want to add RTTI to, without adding a vtable to them.
 *
 * - BaseClass
 *		This class is designed to be the base-class of an inheritance hierarchy. Because of this, a RTTI_ENABLED macro is required in the class definition in order for RTTI to properly work
 *		Note that because this is the base class, no arguments to the RTTI_ENABLED macro are needed
 *
 * - DerivedClass
 *		This class is part of an inheritance hierarchy and in this case inherits from BaseClass. Again, this means a RTTI_ENABLED macro is required in the class definition.
 *		Note that because this class derives from another RTTI class (BaseClass), the class it derives from must be specified as argument to the RTTI_ENABLE macro.
 *
 * Note that the code in the header does not actually register these types with the RTTI system; the RTTI_ENABLED macro is only used to add some plumbing (virtual calls) to the class, not to do actual registration.
 * In order to actually register the types with the RTTI system, the following code must be added to the cpp file:
 *
 *		// RTTIClasses.cpp
 *		RTTI_BEGIN_ENUM(ETestEnum)
 *			RTTI_ENUM_VALUE(ETestEnum::One,		"One"),
 *			RTTI_ENUM_VALUE(ETestEnum::Two,		"Two"),
 *			RTTI_ENUM_VALUE(ETestEnum::Three,	"Three"),
 *			RTTI_ENUM_VALUE(ETestEnum::Four,	"Four")
 *		RTTI_END_ENUM

 *		RTTI_BEGIN_CLASS(DataStruct)
 *				RTTI_PROPERTY("FloatProperty",	&DataStruct::mFloatProperty,	nap::rtti::EPropertyMetaData::None);
 *				RTTI_PROPERTY("StringProperty", &DataStruct::mStringProperty,	nap::rtti::EPropertyMetaData::Required);
 *				RTTI_PROPERTY("EnumProperty",	&DataStruct::mEnumProperty,		nap::rtti::EPropertyMetaData::Required);
 *		RTTI_END_CLASS
 *
 *		RTTI_BEGIN_CLASS(BaseClass)
 *				RTTI_PROPERTY("FloatProperty",	&BaseClass::mFloatProperty, nap::rtti::EPropertyMetaData::None);
 *		RTTI_END_CLASS
 *
 *		RTTI_BEGIN_CLASS(DerivedClass)
 *				RTTI_PROPERTY("IntProperty",	&DerivedClass::mIntProperty, nap::rtti::EPropertyMetaData::None)
 *		RTTI_END_CLASS
 *
 * The above code, which *must* be located in the cpp, is responsible for the registration. As you can see, it is very straightforward.
 * In general, to register a type and its attributes with the RTTI system, you simply use the RTTI_BEGIN_CLASS/RTTI_END_CLASS pair and add RTTI_PROPERTY calls to register the properties you need.
 *
 * Once registered, the type can be looked up in the RTTI system and can be inspected for properties etc. A simple example that prints out the names of all properties of an RTTI class:
 *
 *		template<class T>
 *		void printProperties()
 *		{
 *			rtti::TypeInfo type = RTTI_OF(T); // Could also be rtti::TypeInfo::get<T>()
 *		
 *			std::cout << "Properties of " << type.get_name().data() << std::endl;
 *			for (const rtti::Property& property : type.get_properties())
 *			{
 *				std::cout << " -- " << property.get_name().data() << std::endl;
 *			}
 *		}
 *		
 *		printProperties<DataStruct>();
 */

/**
 * This namespace is only used to redefine some RTTR types to our own types so that the rttr:: namespace does not leak out everywhere
 */
namespace nap
{
	namespace rtti
	{
		using TypeInfo = rttr::type;
		using Property = rttr::property;
		using Variant = rttr::variant;
		using Instance = rttr::instance;
		using VariantArray = rttr::variant_array_view;
		using VariantMap = rttr::variant_associative_view;

		enum class NAPAPI EPropertyMetaData : uint8_t
		{
			Default = 0,
			Required = 1,
			FileLink = 2,
			Embedded = 4
		};

		inline EPropertyMetaData NAPAPI operator&(EPropertyMetaData a, EPropertyMetaData b)
		{
			return static_cast<EPropertyMetaData>(static_cast<uint8_t>(a) & static_cast<uint8_t>(b));
		}
		inline EPropertyMetaData NAPAPI operator|(EPropertyMetaData a, EPropertyMetaData b)
		{
			return static_cast<EPropertyMetaData>(static_cast<uint8_t>(a) | static_cast<uint8_t>(b));
		}

		/**
		 * Helper function to determine whether the specified type is a primitive type (i.e. int, float, string, etc)
		 */
		inline bool NAPAPI isPrimitive(const rtti::TypeInfo& type)
		{
			return type.is_arithmetic() || type.is_enumeration() || type == rtti::TypeInfo::get<std::string>();
		}

		/**
		 * Helper function to check whether a property has the specified flag set
		 */
		inline bool NAPAPI hasFlag(const rtti::Property& property, EPropertyMetaData flags)
		{
			rtti::Variant meta_data = property.get_metadata("flags");
			if (!meta_data.is_valid())
				return false;

			uint8_t current_flags = meta_data.convert<uint8_t>();
			return (current_flags & (uint8_t)flags) != 0;
		}
	}

	namespace detail
	{
		template<typename T>
		struct void_ { typedef void type; };

		template<typename Type, typename = void>
		struct BaseClassList
		{
			using List = rttr::detail::type_list<>;
		};

		template<typename Type>
		struct BaseClassList<Type, typename void_<typename Type::base_class_list>::type>
		{
			using List = typename Type::base_class_list;
		};

		template <typename Return, typename... Args>
		bool isReturnTypeLValueReference(Return(*f)(Args...))
		{
			return std::is_lvalue_reference<Return>();
		}

		template <typename Return, typename Class, typename... Arg>
		bool isReturnTypeLValueReference(Return(Class::*f)(Arg...))
		{
			return std::is_lvalue_reference<Return>();
		}

		template <typename Return, typename Class, typename... Arg>
		bool isReturnTypeLValueReference(Return(Class::*f)(Arg...) const)
		{
			return std::is_lvalue_reference<Return>();
		}

	}
}


// Macros
#define RTTI_OF(Type) nap::rtti::TypeInfo::get<Type>()

#define CONCAT_UNIQUE_NAMESPACE(x, y)				namespace x##y
#define UNIQUE_REGISTRATION_NAMESPACE(id)			CONCAT_UNIQUE_NAMESPACE(__rtti_registration_, id)

#define RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(Type)																				\
	UNIQUE_REGISTRATION_NAMESPACE(__COUNTER__)																	\
	{																											\
		RTTR_REGISTRATION																						\
		{																										\
			using namespace rttr;																				\
			namespace py = pybind11;																			\
																												\
			typedef Type ClassType;																				\
			using PythonClassType = nap::rtti::PythonClass<Type, nap::detail::BaseClassList<Type>::List>;		\
			std::string rtti_class_type_name = #Type;															\
			registration::class_<Type> rtti_class_type(#Type);													\
			PythonClassType python_class(#Type);

#define RTTI_PROPERTY(Name, Member, Flags)																		\
			rtti_class_type.property(Name, Member)(metadata("flags", (uint8_t)(Flags)));						\
			python_class.registerFunction([](pybind11::module& module, PythonClassType::PybindClass& cls)		\
			{																									\
				cls.def_readwrite(Name, Member);																\
			});		

#define RTTI_FUNCTION(Name, Member)																				\
			rtti_class_type.method(Name, Member);																\
			python_class.registerFunction([](pybind11::module& module, PythonClassType::PybindClass& cls)		\
			{																									\
 				cls.def(Name, Member, nap::detail::isReturnTypeLValueReference(Member) ? py::return_value_policy::reference : py::return_value_policy::automatic_reference);	\
			});		

#define RTTI_CUSTOM_REGISTRATION_FUNCTION(Func)																	\
			python_class.registerFunction(std::bind(&Func<PythonClassType::PybindClass>, std::placeholders::_1, std::placeholders::_2));

#define RTTI_CONSTRUCTOR(...)																					\
			rtti_class_type.constructor<__VA_ARGS__>()(policy::ctor::as_raw_ptr);								\
			python_class.registerFunction([](pybind11::module& module, PythonClassType::PybindClass& cls)		\
			{																									\
				cls.def(py::init<__VA_ARGS__>());																\
			});

#define RTTI_END_CLASS																							\
			nap::rtti::PythonModule& python_module = nap::rtti::PythonModule::get("nap");						\
			python_module.registerTypeImportCallback(rtti_class_type_name,										\
													 [](std::vector<std::string>& baseTypes)					\
													 {															\
														PythonClassType::GetBaseTypes(baseTypes);				\
													 },															\
													 [python_class](py::module& module)							\
													 {															\
														python_class.invoke(module);							\
													 });														\
		}																										\
	}

#define RTTI_BEGIN_CLASS(Type)																					\
	RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(Type)																\
	RTTI_CONSTRUCTOR()

#define RTTI_BEGIN_ENUM(Type)																					\
	UNIQUE_REGISTRATION_NAMESPACE(__COUNTER__)																	\
	{																											\
		RTTR_REGISTRATION																						\
		{																										\
			using namespace rttr;																				\
			registration::enumeration<Type>(#Type)																\
			(

#define RTTI_ENUM_VALUE(Value, String)																			\
				value(String, Value)

#define RTTI_END_ENUM																							\
			);																									\
		}																										\
	}

#define RTTI_ENABLE(...)																						\
	RTTR_ENABLE(__VA_ARGS__)																					\
	RTTR_REGISTRATION_FRIEND

// Legacy macros only used for backwards compatibility with the old RTTI system.
#define RTTI_DEFINE(Type)																						\
	RTTI_BEGIN_CLASS(Type)																						\
	RTTI_END_CLASS

#define RTTI_DEFINE_BASE(Type)																					\
	RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(Type)																\
	RTTI_END_CLASS
