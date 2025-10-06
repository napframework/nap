/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <rttr/type>
#include <rttr/registration>
#include <utility/dllexport.h>
#include <string.h>
#include <utility/module.h>

/**
 * This file contains the macros necessary to register types and their attributes with the RTTI system. When registering into the RTTI system
 *
 * There are only a few macros important for the user of the RTTI system:
 * - RTTI_OF - This is a convenience macro used to get the underlying TypeInfo of the named type. Usage example: RTTI_OF(rtti::RTTIObject).
 * - RTTI_ENABLE - This macro must be used when you have a class that is part of an inheritance hierarchy. The argument to the macro is a comma-separated list of base classes (empty if the macro is being used in the base class itself).
 * - RTTI_BEGIN_CLASS, RTTI_BEGIN_STRUCT, RTTI_END_CLASS, RTTI_END_STRUCT, RTTI_PROPERTY, RTTI_FUNCTION - These macros are used to register a type or function in the RTTI system and must be placed in a .cpp file.
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
 *			float		mFloatProperty;
 *			std::string mStringProperty;
 *			ETestEnum	mEnumProperty;
 *		};
 *
 *		class BaseClass
 *		{
 *			RTTI_ENABLE()
 *		private:
 *			float		mFloatProperty;
 *
 *		};
 *
 *		class DerivedClass : public BaseClass
 *		{
 *			RTTI_ENABLE(SomeBaseClass)
 *
 *			DerivedClass() = default;
 *			DerivedClass(int value) :
 *				mIntProperty(value)
 *			{
 *			}
 *
 *			int	getValue() const { return mIntProperty; }
 *
 *		private:
 *			int			mIntProperty;
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
 *		The class has two constructors: the default constructor (which will be registered by default) and another constructor that we will have to register manually.
 *		GetValue is a function that we are going to expose as well.
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
 *				RTTI_PROPERTY("FloatProperty",	&DataStruct::mFloatProperty,	nap::rtti::EPropertyMetaData::None,			"Property description");
 *				RTTI_PROPERTY("StringProperty", &DataStruct::mStringProperty,	nap::rtti::EPropertyMetaData::Required,		"Property description");
 *				RTTI_PROPERTY("EnumProperty",	&DataStruct::mEnumProperty,		nap::rtti::EPropertyMetaData::Required,		"Property description");
 *		RTTI_END_CLASS
 *
 *		RTTI_BEGIN_CLASS(BaseClass)
 *				RTTI_PROPERTY("FloatProperty",	&BaseClass::mFloatProperty, nap::rtti::EPropertyMetaData::None);
 *		RTTI_END_CLASS
 *
 *		RTTI_BEGIN_CLASS(DerivedClass, "Type description")
 *				RTTI_CONSTRUCTOR(int)
 *				RTTI_PROPERTY("IntProperty",	&DerivedClass::mIntProperty, nap::rtti::EPropertyMetaData::None)
 *				RTTI_FUNCTION("getValue",		&DerivedClass::getValue)
 *		RTTI_END_CLASS
 *
 * The above code, which *must* be located in the cpp, is responsible for the registration.
 * In general, to register a type and its attributes with the RTTI system, you simply use the RTTI_BEGIN_CLASS/RTTI_END_CLASS pair and add RTTI_PROPERTY, RTTI_CONSTRUCTOR and RTTI_FUNCTION calls to register the properties you need.
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
		/**
		 * Common rtti types
		 */
		using TypeInfo = rttr::type;
		using Enum = rttr::enumeration;
		using Property = rttr::property;
		using Variant = rttr::variant;
		using Instance = rttr::instance;
		using VariantArray = rttr::variant_array_view;
		using VariantMap = rttr::variant_associative_view;

		/**
		 * Common shared rtti defined methods
		 */
		namespace method
		{
			constexpr const char* description = "description";					///< rtti type description
			constexpr const char* moduleDescription = "moduleDescription";		///< nap module description
			constexpr const char* assign = "assign";							///< assignment
			constexpr const char* toObject = "toObject";						///< to object pointer
			constexpr const char* toString	= "toString";						///< to object path
			constexpr const char* translateTargetID = "translateTargetID";		///< transform id
		}

		/**
		 * Controls how an RTTI property is interpreted
		 */
		enum class EPropertyMetaData : uint8_t
		{
			Default  	= 0,		///< Uses the (class) default if the property isn't set
			Required 	= 1,		///< Load will fail if the property isn't set
			FileLink 	= 2,		///< Defines a relationship with an external file
			Embedded 	= 4,		///< An embedded pointer
			ReadOnly	= 8			///< A read only property
		};

		/**
		 * If EPropertyMetaData::FileLink is set, you can provide a file type. Used for tooling.
		 */
		enum class EPropertyFileType : uint8_t
		{
			Any				= 0,	///< Can point to any file, default.
			Image			= 1, 	///< Points to an image file, must be used with EPropertyMetaData::FileLink
			FragShader		= 2, 	///< Points to a .vert file, must be used with EPropertyMetaData::FileLink
			VertShader		= 3, 	///< Points to a .frag file, must be used with EPropertyMetaData::FileLink
			ComputeShader	= 4, 	///< Points to a .comp file, must be used with EPropertyMetaData::FileLink
			Mesh			= 6,	///< Points to a .mesh file, must be used with EPropertyMetaData::FileLink
			Video			= 7,	///< Points to a video file, must be used with EPropertyMetaData::FileLink
			ImageSequence	= 8,	///< Points to a an image sequence, must be used with EPropertyMetaData::FileLink
            Audio           = 9,	///< Points to an audio file, must be used with EPropertyMetaData::FileLink
			Font			= 10	///< Points to a true type font, must be used with EPropertyMetaData::FileLink
		};

		inline EPropertyMetaData operator&(EPropertyMetaData a, EPropertyMetaData b)
		{
			return static_cast<EPropertyMetaData>(static_cast<uint8_t>(a) & static_cast<uint8_t>(b));
		}

		inline EPropertyMetaData operator|(EPropertyMetaData a, EPropertyMetaData b)
		{
			return static_cast<EPropertyMetaData>(static_cast<uint8_t>(a) | static_cast<uint8_t>(b));
		}

		/**
		 * Helper function to determine whether the specified type is a primitive type (i.e. int, float, string, etc)
		 */
		inline bool isPrimitive(const rtti::TypeInfo& type)
		{
			return type.is_arithmetic() || type.is_enumeration() || type == rtti::TypeInfo::get<std::string>();
		}

		/**
		 * Helper function to check whether a property has the specified flag set
		 */
		inline bool hasFlag(const rtti::Property& property, EPropertyMetaData flags)
		{
			const rtti::Variant& meta_data = property.get_metadata("flags");
			if (!meta_data.is_valid())
				return false;

			uint8_t current_flags = meta_data.convert<uint8_t>();
			return (current_flags & (uint8_t)flags) != 0;
		}

		/**
		 * Helper function to check whether a property is associated with a specific type of file
		 * @return if the property is a specific type of file
		 */
		inline bool isFileType(const rtti::Property &property, EPropertyFileType filetype)
		{
			const rtti::Variant& meta_data = property.get_metadata("filetype");
			return meta_data.is_valid() ?
				meta_data.convert<uint8_t>() == (uint8_t)filetype : false;
		}

		/**
		 * Selects whether the type check should be an exact type match or whether
		 * the type should be derived from the given type.
		 */
		enum class ETypeCheck : uint8_t
		{
			EXACT_MATCH,				///< The type needs to be of the exact same kind
			IS_DERIVED_FROM				///< The type is derived from the specified type
		};

		/**
		 * Helper function to check whether two types match, based on a comparison mode
		 */
		inline bool isTypeMatch(const rtti::TypeInfo& typeA, const rtti::TypeInfo& typeB, ETypeCheck typeCheck)
		{
			return typeCheck == ETypeCheck::EXACT_MATCH ? typeA == typeB : typeA.is_derived_from(typeB);
		}

		/**
		 * Finds method recursively in class and its base classes.
		 * Note that if a function is defined twice the base class gets priority.
		 * 
		 * Note: this should normally work through the regular rttr::get_method function,
		 * but this does not seem to work properly. This function is used as a workaround until we solve the issue.
		 */
		inline rttr::method findMethodRecursive(const rtti::TypeInfo& type, const std::string& methodName)
		{
			for (const rtti::TypeInfo& base : type.get_base_classes())
			{
				auto result = base.get_method(methodName);
				if (result.is_valid())
					return result;
			}
			return type.get_method(methodName);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
// RTTI Macros
//////////////////////////////////////////////////////////////////////////

/**
 *	@return the rtti type of Type
 */
#define RTTI_OF(Type) nap::rtti::TypeInfo::get<Type>()


 //////////////////////////////////////////////////////////////////////////
 // RTTI_ENABLE
 //////////////////////////////////////////////////////////////////////////

 /**
  * Receives information about the inheritance graph of a class.
  * When class B is derived from A -> A uses: RTTI_ENABLE(), B uses: RTTI_ENABLE(A)
  * When D is derived from B and C, D uses: RTTI_ENABLE(B,C)
  */
#define RTTI_ENABLE(...)																						\
	RTTR_ENABLE(__VA_ARGS__)																					\
	RTTR_REGISTRATION_FRIEND


//////////////////////////////////////////////////////////////////////////
// RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR
//////////////////////////////////////////////////////////////////////////

#define CONCAT_UNIQUE_NAMESPACE(x, y)				namespace x##y
#define UNIQUE_REGISTRATION_NAMESPACE(id)			CONCAT_UNIQUE_NAMESPACE(__rtti_registration_, id)

/**
 * Defines the beginning of an RTTI enabled class.
 * This macro will register the class with the RTTI system.
 * It also enables the class to be available to python.
 * @param Type the type to register
 */

#define RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR_1(Type)														\
UNIQUE_REGISTRATION_NAMESPACE(__COUNTER__)																	\
{																											\
	static nap::ModuleDescriptor* getModuleDescriptor() { return NAP_MODULE_DESCIPTOR_HANDLE; }				\
	RTTR_REGISTRATION																						\
	{																										\
		using namespace rttr;																				\
		std::string rtti_class_type_name = #Type;															\
		registration::class_<Type> rtti_class_type(#Type);													\
		rtti_class_type.method(nap::rtti::method::moduleDescription, &getModuleDescriptor);


 /**
  * Defines the beginning of an RTTI enabled class with a description.
  * This macro will register the class of with the RTTI system.
  * It also enables the class to be available to python.
  * @param Type the type to register
  * @param Description type description
  */
#define RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR_2(Type, Description)											\
	UNIQUE_REGISTRATION_NAMESPACE(__COUNTER__)																	\
	{																											\
		static const char* getTypeDescription()				{ return Description; }								\
		static nap::ModuleDescriptor* getModuleDescriptor()	{ return NAP_MODULE_DESCIPTOR_HANDLE; }				\
		RTTR_REGISTRATION																						\
		{																										\
			using namespace rttr;																				\
			std::string rtti_class_type_name = #Type;															\
			registration::class_<Type> rtti_class_type(#Type);													\
			rtti_class_type.method(nap::rtti::method::description, &getTypeDescription);						\
			rtti_class_type.method(nap::rtti::method::moduleDescription, &getModuleDescriptor);					

// Selector
#define GET_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR_MACRO(_1,_2,NAME,...) NAME

  /**
   * Defines the beginning of an RTTI enabled class with an optional description
   * This macro will register the class with the RTTI system.
   * It also enables the class to be available to python.
   * @param Type the type to register
   * @param Description optional type description
   */
#define RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(...) GET_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR_MACRO(__VA_ARGS__, RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR_2, RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR_1)(__VA_ARGS__)


//////////////////////////////////////////////////////////////////////////
// RTTI_PROPERTY
//////////////////////////////////////////////////////////////////////////

/**
 * Registers a property that is readable and writable in C++ and Python
 * Call this after starting your class definition
 * @param Name RTTI name of the property
 * @param Member reference to the member variable
 * @param Flags flags associated with the property of type: EPropertyMetaData. these can be or'd
 */
#define RTTI_PROPERTY_3(Name, Member, Flags)																\
        rtti_class_type.property(Name, Member)( metadata("flags", (uint8_t)(Flags)));

/**
 * Registers a property that is readable and writable in C++ and Python.
 * Call this after starting your class definition.
 * @param Name RTTI name of the property
 * @param Member reference to the member variable
 * @param Flags flags associated with the property of type: EPropertyMetaData. these can be or'd\
 * @param Description property description
 */
#define RTTI_PROPERTY_4(Name, Member, Flags, Description)													\
			rtti_class_type.property(Name, Member)( 														\
								metadata("flags", (uint8_t)(Flags)),										\
								metadata("description", (const char*)(Description)));

#define GET_PROPERTY_MACRO(_1,_2,_3,_4,NAME,...) NAME

/**
 * Registers a property of that is readable and writable in C++ and Python, with or without description.
 * Call this after starting your class definition.
 * @param Name RTTI name of the property.
 * @param Member reference to the member variable.
 * @param Flags flags associated with the property of type: EPropertyMetaData. these can be or'd
 * @param Description optional 
 * @param Description property optional property description
 */
#define RTTI_PROPERTY(...) GET_PROPERTY_MACRO(__VA_ARGS__, RTTI_PROPERTY_4, RTTI_PROPERTY_3)(__VA_ARGS__)


//////////////////////////////////////////////////////////////////////////
// RTTI_PROPERTY_FILELINK
//////////////////////////////////////////////////////////////////////////

/**
 * Registers a property that will point to a file on disk.
 * Call this after starting your class definition.
 * @param Name RTTI name of the property
 * @param Member reference to the member variable
 * @param Flags additional flags of type 'EPropertyMetaData', these can be or'd.
 * @param FileType the type of file we're going to refer to (EPropertyFileType)
 */
#define RTTI_PROPERTY_FILELINK_4(Name, Member, Flags, FileType)													\
			rtti_class_type.property(Name, Member)( 															\
								metadata("flags", (uint8_t)(nap::rtti::EPropertyMetaData::FileLink | Flags)),	\
								metadata("filetype", (uint8_t)(FileType)));

/**
 * Registers a property that will point to a file on disk.
 * Call this after starting your class definition.
 * @param Name RTTI name of the property
 * @param Member reference to the member variable
 * @param Flags additional flags of type 'EPropertyMetaData', these can be or'd.
 * @param FileType the type of file we're going to refer to (EPropertyFileType)
 * @param Description property description 
 */
#define RTTI_PROPERTY_FILELINK_5(Name, Member, Flags, FileType, Description)									\
			rtti_class_type.property(Name, Member)( 															\
								metadata("flags", (uint8_t)(nap::rtti::EPropertyMetaData::FileLink | Flags)),	\
								metadata("filetype", (uint8_t)(FileType)),										\
								metadata("description", (const char*)(Description)));							

#define GET_PROPERTY_FILELINK_MACRO(_1,_2,_3,_4,_5,NAME,...) NAME

/**
 * Registers a property that will point to a file on disk with or without a description.
 * Call this after starting your class definition.
 * @param Name RTTI name of the property
 * @param Member reference to the member variable
 * @param Flags additional flags of type 'EPropertyMetaData', these can be or'd.
 * @param FileType the type of file we're going to refer to (EPropertyFileType)
 * @param Description optional property description
 */
#define RTTI_PROPERTY_FILELINK(...) GET_PROPERTY_FILELINK_MACRO(__VA_ARGS__, RTTI_PROPERTY_FILELINK_5, RTTI_PROPERTY_FILELINK_4)(__VA_ARGS__)


//////////////////////////////////////////////////////////////////////////
// RTTI_FUNCTION
//////////////////////////////////////////////////////////////////////////

/**
 * Registers a function of the given name.
 * Call this after starting your class definition.
 * @param Name RTTI name of the function
 * @param Member reference to the member function
 */
#define RTTI_FUNCTION(Name, Member)																				\
			rtti_class_type.method(Name, Member);

/**
 * Registers a set of custom functions that are exposed to python.
 * Call this after starting your class definition.
 * @param Func the name of the function that registers a set of python bindings
 */
#define RTTI_CUSTOM_REGISTRATION_FUNCTION(Func)


//////////////////////////////////////////////////////////////////////////
// RTTI_CONSTRUCTOR
//////////////////////////////////////////////////////////////////////////

#define RTTI_CONSTRUCTOR(...)																				\
		rtti_class_type.constructor<__VA_ARGS__>()(policy::ctor::as_raw_ptr);


 //////////////////////////////////////////////////////////////////////////
 // RTTI_VALUE_CONSTRUCTOR
 //////////////////////////////////////////////////////////////////////////

 /**
  * Registers a value based constructor. This is exposed to the RTTI system and python
  * This constructor creates an instance of a class with automatic storage.
  * For this to work the object must be copy-constructible.
  * Use this constructor in conjunction with simple struct like objects or objects that carry a limited set of data.
  * Objects with automatic storage duration are automatically destroyed when the variant is out of scope.
  * Call this after starting your class definition.
  */
#define RTTI_VALUE_CONSTRUCTOR(...)																				\
			rtti_class_type.constructor<__VA_ARGS__>()(policy::ctor::as_object);


//////////////////////////////////////////////////////////////////////////
// RTTI_END_CLASS
//////////////////////////////////////////////////////////////////////////

 /**
  * Signals the end of the class definition.
  * Define this after having defined the various constructors, properties, functions etc.
  */
#define RTTI_END_CLASS																							\
		}																										\
	}


//////////////////////////////////////////////////////////////////////////
// RTTI_BEGIN_STRUCT_NO_DEFAULT_CONSTRUCTOR
//////////////////////////////////////////////////////////////////////////

 /**
 * Defines the beginning of an RTTI enabled struct.
 * Use this in conjunction with light or copy-constructible objects.
 * This macro will register the class of Type with the RTTI system.
 * It also enables the class to be available to python.
 * @param Type the type to register
 */
#define RTTI_BEGIN_STRUCT_NO_DEFAULT_CONSTRUCTOR_1(Type)														\
	RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(Type)

/**
* Defines the beginning of an RTTI enabled struct.
* Use this in conjunction with light or copy-constructible objects.
* This macro will register the class of Type with the RTTI system.
* It also enables the class to be available to python.
* @param Type the type to register
* @param Description type description
*/
#define RTTI_BEGIN_STRUCT_NO_DEFAULT_CONSTRUCTOR_2(Type, Description)											\
	RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(Type, Description)

#define GET_RTTI_BEGIN_STRUCT_NO_DEFAULT_CONSTRUCTOR_MACRO(_1,_2,NAME,...) NAME

/**
 * Defines the beginning of an RTTI enabled struct.
 * Use this in conjunction with light or copy-constructible objects.
 * This macro will register the class of Type with the RTTI system.
 * It also enables the class to be available to python.
 * @param Type the type to register
 * @param Description optional type description
 */
#define RTTI_BEGIN_STRUCT_NO_DEFAULT_CONSTRUCTOR(...) GET_RTTI_BEGIN_STRUCT_NO_DEFAULT_CONSTRUCTOR_MACRO(__VA_ARGS__, RTTI_BEGIN_STRUCT_NO_DEFAULT_CONSTRUCTOR_2, RTTI_BEGIN_STRUCT_NO_DEFAULT_CONSTRUCTOR_1)(__VA_ARGS__)


 //////////////////////////////////////////////////////////////////////////
 // RTTI_END_STRUCT
 //////////////////////////////////////////////////////////////////////////

 /**
 * Signals the end of the class definition.
 * Use this in conjunction with light or copy-constructible objects.
 * Define this after having defined the various constructors, properties, functions etc.
 */
#define RTTI_END_STRUCT																							\
	RTTI_END_CLASS


 //////////////////////////////////////////////////////////////////////////
 // RTTI_BEGIN_CLASS
 //////////////////////////////////////////////////////////////////////////

/**
 * Utility that defines the beginning of a class of Type together with a default (no argument) constructor.
 * The rtti constructor creates the object with a new-expression and it's lifetime lasts until destroyed.
 * In order to invoke the delete expression use the corresponding rtti destructor.
 * @param Type the type to register
 */
#define RTTI_BEGIN_CLASS_1(Type)																					\
	RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(Type)																	\
	RTTI_CONSTRUCTOR()

/**
 * Utility that defines the beginning of a class of Type together with a default (no argument) constructor.
 * The rtti constructor creates the object with a new-expression and it's lifetime lasts until destroyed.
 * In order to invoke the delete expression use the corresponding rtti destructor.
 * @param Type the type to register
 * @param Description type description
 */
#define RTTI_BEGIN_CLASS_2(Type, Description)																		\
	RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(Type, Description)														\
	RTTI_CONSTRUCTOR()

#define GET_RTTI_BEGIN_CLASS_MACRO(_1,_2,NAME,...) NAME

/**
 * Utility that defines the beginning of a class of Type together with a default (no argument) constructor.
 * The rtti constructor creates the object with a new-expression and it's lifetime lasts until destroyed.
 * In order to invoke the delete expression use the corresponding rtti destructor.
 * @param Type the type to register
 * @param Description type description
 */
#define RTTI_BEGIN_CLASS(...) GET_RTTI_BEGIN_CLASS_MACRO(__VA_ARGS__, RTTI_BEGIN_CLASS_2, RTTI_BEGIN_CLASS_1)(__VA_ARGS__)


//////////////////////////////////////////////////////////////////////////
// RTTI_BEGIN_STRUCT
//////////////////////////////////////////////////////////////////////////

 /**
  * Utility that defines the beginning of a class of Type together with a default (no argument) constructor.
  * This constructor creates an instance of a class with automatic storage.
  * For this to work the object must be copy-constructible.
  * Use this definition in conjunction with simple struct like objects or objects that carry a limited set of data.
  * @param Type the type to register
  */
#define RTTI_BEGIN_STRUCT_1(Type)																					\
	RTTI_BEGIN_STRUCT_NO_DEFAULT_CONSTRUCTOR(Type)																	\
	RTTI_VALUE_CONSTRUCTOR()

/**
 * Utility that defines the beginning of a class of Type together with a default (no argument) constructor.
 * This constructor creates an instance of a class with automatic storage.
 * For this to work the object must be copy-constructible.
 * Use this definition in conjunction with simple struct like objects or objects that carry a limited set of data.
 * @param Type the type to register
 * @param Description the type description
 */
#define RTTI_BEGIN_STRUCT_2(Type, Description)																		\
	RTTI_BEGIN_STRUCT_NO_DEFAULT_CONSTRUCTOR(Type, Description)														\
	RTTI_VALUE_CONSTRUCTOR()

#define GET_RTTI_BEGIN_STRUCT_MACRO(_1,_2,NAME,...) NAME

 /**
  * Utility that defines the beginning of a class of Type together with a default (no argument) constructor.
  * The rtti constructor creates the object with a new-expression and it's lifetime lasts until destroyed.
  * In order to invoke the delete expression use the corresponding rtti destructor.
  * @param Type the type to register
  * @param Description type description
  */
#define RTTI_BEGIN_STRUCT(...) GET_RTTI_BEGIN_STRUCT_MACRO(__VA_ARGS__, RTTI_BEGIN_STRUCT_2, RTTI_BEGIN_STRUCT_1)(__VA_ARGS__)


 //////////////////////////////////////////////////////////////////////////
 // RTTI_BEGIN_ENUM
 //////////////////////////////////////////////////////////////////////////

 /**
  * Starts the RTTI definition of an enumerator of Type.
  * @param Type the enumeration type
  */
#define RTTI_BEGIN_ENUM(Type)																					\
	UNIQUE_REGISTRATION_NAMESPACE(__COUNTER__)																	\
	{																											\
		RTTR_REGISTRATION																						\
		{																										\
			using namespace rttr;																				\
			registration::enumeration<Type>(#Type)																\
			(

  /**
   * Adds a enumeration value, every value has a name.
   * @param Value the enumeration value declaration
   * @param String the rtti name of the enumeration value
   */
#define RTTI_ENUM_VALUE(Value, String)																			\
				value(String, Value)

   /**
	* Ends the RTTI definition
	*/
#define RTTI_END_ENUM																							\
			);																									\
		}																										\
	}


//////////////////////////////////////////////////////////////////////////
// RTTI_DEFINE_BASE
//////////////////////////////////////////////////////////////////////////

/**
 * Defines an abstract (not create-able) class of Type.
 * This class can't be constructed using the RTTI system.
 * Use this macro for defining abstract or base classes.
 * @param Type the type to register
 */
#define RTTI_DEFINE_BASE_1(Type)																				\
	RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(Type)																\
	RTTI_END_CLASS

 /**
  * Defines an abstract (not create-able) class of Type.
  * This class can't be constructed using the RTTI system.
  * Use this macro for defining abstract or base classes.
  * @param Type the type to register
  * @param Description type description
  */
#define RTTI_DEFINE_BASE_2(Type, Description)																	\
	RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(Type, Description)													\
	RTTI_END_CLASS

#define GET_RTTI_DEFINE_BASE_MACRO(_1,_2,NAME,...) NAME

  /**
   * Defines an abstract (not create-able) class of Type.
   * This class can't be constructed using the RTTI system.
   * Use this macro for defining abstract or base classes.
   * @param Type the type to register
   * @param Description optional type description
   */
#define RTTI_DEFINE_BASE(...) GET_RTTI_DEFINE_BASE_MACRO(__VA_ARGS__, RTTI_DEFINE_BASE_2, RTTI_DEFINE_BASE_1)(__VA_ARGS__)


//////////////////////////////////////////////////////////////////////////
// RTTI_DEFINE_CLASS
//////////////////////////////////////////////////////////////////////////

/**
 * Registers a class of Type without properties (legacy).
 * @param Type the type to register
 */
#define RTTI_DEFINE_CLASS_1(Type)																				\
	RTTI_BEGIN_CLASS(Type)																						\
	RTTI_END_CLASS

/**
 * Registers a class of Type without properties (legacy).
 * @param Type the type to register
 * @param Description type description
 */
#define RTTI_DEFINE_CLASS_2(Type, Description)																	\
	RTTI_BEGIN_CLASS(Type, Description)																			\
	RTTI_END_CLASS

#define GET_RTTI_DEFINE_CLASS_MACRO(_1,_2,NAME,...) NAME

/**
 * Registers a class of Type without properties (legacy).
 * @param Type the type to register
 * @param Description optional type description
 */
#define RTTI_DEFINE_CLASS(...) GET_RTTI_DEFINE_CLASS_MACRO(__VA_ARGS__, RTTI_DEFINE_CLASS_2, RTTI_DEFINE_CLASS_1)(__VA_ARGS__)


//////////////////////////////////////////////////////////////////////////
// RTTI_DEFINE_STRUCT
//////////////////////////////////////////////////////////////////////////

/**
 * Registers a struct of Type without properties (legacy).
 * @param Type the type to register
 */
#define RTTI_DEFINE_STRUCT_1(Type)																				\
	RTTI_BEGIN_STRUCT(Type)																						\
	RTTI_END_STRUCT

/**
 * Registers a struct of Type without properties (legacy).
 * @param Type the type to register
 * @param Description type description
 */
#define RTTI_DEFINE_STRUCT_2(Type, Description)																	\
	RTTI_BEGIN_STRUCT(Type, Description)																		\
	RTTI_END_STRUCT

#define GET_RTTI_DEFINE_STRUCT_MACRO(_1,_2,NAME,...) NAME

/**
 * Registers a struct of Type without properties (legacy).
 * @param Type the type to register
 * @param Description optional type description
 */
#define RTTI_DEFINE_STRUCT(...) GET_RTTI_DEFINE_STRUCT_MACRO(__VA_ARGS__, RTTI_DEFINE_STRUCT_2, RTTI_DEFINE_STRUCT_1)(__VA_ARGS__)
