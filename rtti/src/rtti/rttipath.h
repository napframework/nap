#pragma once

#include <string>
#include <assert.h>
#include <rtti/typeinfo.h>

namespace rtti
{
	class RTTIObject;

	/**
	 * Represents an element on an RTTIPath. Each element is of a specific type and has different data, depending on the type of the element.
	 * In order to be able to have an array (without dynamic allocs) of these elements, it makes use of an anonymous union to store the data.
	 * Then, depending on the type of the element, you can access the data through Element.Attribute or Element.ArrayElement.
	 *
	 * One important thing to note is that we have non-POD types in the union; this used to be impossible, but since C++11 this is allowed in the form of
	 * 'unrestricted unions'. However, as soon as you use unrestricted unions, the compiler deletes the default constructors, copy constructors and assignment operators.
	 *
	 * So, we need to reimplement all these functions.
	 *
	 * For more information on unrestricted unions, see https://faouellet.github.io/unrestricted-unions/
	 *
	 * Note: this class should be used through RTTIPath, not directly
	 */
	class RTTIPathElement
	{
	public:
		/**
		 * The type of this element
		 */
		enum class Type
		{
			INVALID,		// Not a valid element
			ATTRIBUTE,		// Attribute
			ARRAY_ELEMENT	// Array index (element)
		};

		/**
		 * Equality operator
		 */
		bool operator==(const RTTIPathElement& lhs) const
		{
			// Check type first
			if (mType != lhs.mType)
				return false;

			// Compare properties
			if (mType == Type::ATTRIBUTE)
				return Attribute.Name == lhs.Attribute.Name;
			else if (mType == Type::ARRAY_ELEMENT)
				return ArrayElement.Index == lhs.ArrayElement.Index;

			assert(false);
			return false;
		}

		/**
		 * Inequality operator
		 */
		bool operator!=(const RTTIPathElement& lhs) const
		{
			return !(*this == lhs);
		}

		Type mType = Type::INVALID;		// The type of this element
		union 
		{
			struct 
			{
				std::string Name;		// The name of the attribute we're representing
			} Attribute;

			struct 
			{
				int Index;				// The index of the array element we're representing
			} ArrayElement;
		};

	private:
		friend class RTTIPath;

		/**
		 * Default constructor
		 */
		RTTIPathElement() :
			mType(Type::INVALID)
		{
		}

		/**
		 * Constructor for the attribute type
		 *
		 * @param attributeName The name of the attribute represented by this element
		 */
		RTTIPathElement(const std::string& attributeName) : 
			mType(Type::ATTRIBUTE)		
		{ 
			initNonPOD(Attribute.Name, attributeName); 
		}

		/**
		 * Constructor for the array element type
		 *
		 * @param arrayIndex The index of the array element represented by this element
		 */
		RTTIPathElement(int arrayIndex) : 
			mType(Type::ARRAY_ELEMENT)
		{ 
			ArrayElement.Index = arrayIndex; 
		}

		/**
		 * Copy constructor
		 */
		RTTIPathElement(const RTTIPathElement& other)
		{ 
			copyFrom(other); 
		}

		/**
		 * Destructor
		 */
		~RTTIPathElement()
		{
			destroy(); 
		}

		/**
		 * Assignment operator
		 */
		RTTIPathElement& operator=(const RTTIPathElement& other)
		{
			// Prevent self-assignment
			if (&other == this)
				return *this;

			// Destroy current value and copy over new data
			destroy();
			copyFrom(other);

			return *this;
		}

		/**
		 * Helper function to copy-construct a non-pod from a value. This is necessary because non-POD members of unrestricted unions must be placement new'd
		 *
		 * @param member The non-POD to construct
		 * @param val The value to construct the non-POD from
		 */
		template <typename T>
		inline void initNonPOD(T& member, const T& val)
		{
			new (&member) T(val);
		}

		/**
		 * Helper function to correctly destroy the data
		 */
		inline void destroy()
		{
			if (mType == Type::ATTRIBUTE)
			{
				// The Attribute.Name property is placement new'd; we must destroy it explicitly.
				using std::string;
				Attribute.Name.~string();
			}
		}

		/**
		 * Helper function to copy data
		 */
		inline void copyFrom(const RTTIPathElement& other)
		{
			mType = other.mType;
			if (mType == Type::ATTRIBUTE)
			{
				initNonPOD(Attribute.Name, other.Attribute.Name);
			}
			else if (mType == Type::ARRAY_ELEMENT)
			{
				ArrayElement.Index = other.ArrayElement.Index;
			}
		}
	};

	/**
	 * This is the analogue to RTTIPathElement; it is used to represent an element on a ResolvedRTTIPath. See RTTIPathElement for more documentation.
	 */
	class ResolvedRTTIPathElement
	{
	private:
		friend class ResolvedRTTIPath;
		/**
		 * The type of this element
		 */
		enum class Type
		{
			INVALID,		// Invalid type
			ROOT,			// Root
			ATTRIBUTE,		// Attribute
			ARRAY_ELEMENT	// Array index
		};
		
		/**
		 * Default constructor
		 */
		ResolvedRTTIPathElement() :
			mType(Type::INVALID)
		{
		}

		/**
		 * Constructor for root element
		 *
		 * @param instance The root object
		 * @param property The property on the root object
		 */
		ResolvedRTTIPathElement(const rtti::Instance& instance, const rtti::Property& property) :
			mType(Type::ROOT)
		{
			initNonPOD(Root.Instance, instance);
			initNonPOD(Root.Property, property);
		}

		/**
		 * Constructor for attribute element
		 *
		 * @param variant The object the property is on
		 * @param property The property on the object
		 */
		ResolvedRTTIPathElement(const rtti::Variant& variant, const rtti::Property& property) :
			mType(Type::ATTRIBUTE)
		{
			initNonPOD(Attribute.Variant, variant);
			initNonPOD(Attribute.Property, property);
		}

		/**
		 * Constructor for array element
		 *
		 * @param array The array we're indexing into
		 * @param index The index in the array
		 */
		ResolvedRTTIPathElement(const rtti::Variant& array, int index) :
			mType(Type::ARRAY_ELEMENT)
		{
			initNonPOD(ArrayElement.Array, array);
			ArrayElement.Index = index;
		}

		/**
		 * Copy constructor
		 */
		ResolvedRTTIPathElement(const ResolvedRTTIPathElement& other)
		{
			copyFrom(other);
		}

		/** 
		 * Destructor
		 */
		~ResolvedRTTIPathElement()
		{
			destroy();
		}

		/**
		 * Assignment operator
		 */
		ResolvedRTTIPathElement& operator=(const ResolvedRTTIPathElement& other)
		{
			if (&other == this)
				return *this;

			destroy();
			copyFrom(other);

			return *this;
		}
		
		/**
		 * Helper function to copy-construct a non-pod from a value. This is necessary because non-POD members of unrestricted unions must be placement new'd
		 *
		 * @param member The non-POD to construct
		 * @param val The value to construct the non-POD from
		 */
		template <typename T>
		inline void initNonPOD(T& member, const T& val)
		{
			new (&member) T(val);
		}

		/**
		 * Helper function to correctly destroy the data
		 */
		inline void destroy()
		{
			// Because all data is placement new'd, we have to call destructors manually
			if (mType == Type::ROOT)
			{
				Root.Instance.~instance();
				Root.Property.~property();
			}
			else if (mType == Type::ATTRIBUTE)
			{
				Attribute.Variant.~variant();
				Attribute.Property.~property();
			}
			else if (mType == Type::ARRAY_ELEMENT)
			{
				ArrayElement.Array.~variant();
			}
		}

		/**
		 * Helper function to copy data
		 */
		inline void copyFrom(const ResolvedRTTIPathElement& other)
		{
			mType = other.mType;
			if (mType == Type::ROOT)
			{
				initNonPOD(Root.Instance, other.Root.Instance);
				initNonPOD(Root.Property, other.Root.Property);
			}
			else if (mType == Type::ATTRIBUTE)
			{
				initNonPOD(Attribute.Variant, other.Attribute.Variant);
				initNonPOD(Attribute.Property, other.Attribute.Property);
			}
			else if (mType == Type::ARRAY_ELEMENT)
			{
				initNonPOD(ArrayElement.Array, other.ArrayElement.Array);
				ArrayElement.Index = other.ArrayElement.Index;
			}
		}

		Type mType;							// The type of this element
		union
		{
			struct
			{
				rtti::Instance Instance;	// The root object (i.e. pointer) that this path was resolved against
				rtti::Property Property;	// The property on the root object that we're setting
			} Root;

			struct
			{
				rtti::Variant Variant;		// The object (can be a nested compound) that the attribute we're representing is on
				rtti::Property Property;	// The property on the root object we're representing
			} Attribute;

			struct
			{
				rtti::Variant Array;		// The array that the array element we're representing is in
				int Index;					// The index in the array
			} ArrayElement;
		};
	};

	/**
	 * An RTTIPath represents a path to a specific property/value through a RTTI hierarchy. It's useful to be able to store a 'reference' to a property in a data structure, without actually having to have a reference.
	 * By itself, RTTIPath just stores a path and can't be used for any operations. In order to do that, the RTITPath must be 'resolved' against a root object. 
	 * The resolve of an RTTIPath results in a ResolvedRTTIPath, which can then be used to get/set the value of the property represented by the RTTIPath.
	 *
	 * As a simple example, consider the following RTTI types (assume they're registered in RTTI):
	 *
	 *		struct DataStruct
	 *		{
	 *			float				mFloatProperty = 0.0f;
	 *			rtti::RTTIObject*	mPointerProperty = nullptr;
	 *		};
	 *
	 *		class SomeRTTIClass : public rtti::RTTIObject
	 *		{
	 *			RTTI_ENABLE(rtti::RTTIObject)
	 *
	 *		public:
	 *			DataStruct						mNestedCompound;
	 *			std::vector<int>				mArrayOfInts;
	 *			std::vector<DataStruct>			mArrayOfCompounds;
	 *			std::vector<rtti::RTTIObject*>	mArrayOfPointers;
	 *		};
	 *
	 * Now, let's suppose we want to store a reference to the 'PointerProperty' of the 'DataStruct' embedded in the first element of the 'ArrayOfCompounds' property in 'SomeRTTIClass'.
	 * We could do this as follows:
	 *
	 *		RTTIPath path;
	 *		path.PushAttribute("ArrayOfCompounds");		// Push name of the RTTI property on SomeRTTIClass
	 *		path.PushArrayElement(0);					// Push index into the array 'ArrayOfCompounds' on SomeRTTIClass
	 *		path.PushAttribute("PointerProperty");		// Push name of the RTTI property on the compound contained in ArrayOfCompounds, namely 'DataStruct'
	 * 
	 * So we now have a 'path' to the pointer property, which we can store wherever we want.
	 *
	 * At some later point in time, let's suppose we now want to get or set the value that this RTTIPath points to. In order to do this, we first need to resolve the path:
	 *
	 *		SomeRTTIClass* object = ... // Assume we have a pointer to an instance of SomeRTTIClass that we can resolve against
	 *		ResolvedRTTIPath resolved_path = path.resolve(object);
	 *		if (!resolved_path.isValid())
	 *			return; // Failed to resolve the path; either it's incorrect or does not match with the provided instance of SomeRTTIClass
	 *
	 * Once the path has been successfully resolved, we can use it to get/set the value of the property:
	 *
	 *		// Retrieve the value of the pointer
	 *		rtti::RTTIObject* pointer_value = resolved_path.getValue().convert<rtti::RTTIObject*>();
	 *
	 *		// Set the value of the pointer
	 *		rtti::RTTIObject* some_other_object = ... // Get the pointer we want to set
	 *		resolved_path.setValue(some_other_object);
	 *
	 * After the setValue, object->mArrayOfCompounds[0].mPointerProperty now points to 'some_other_object'
	*/
	class RTTIPath
	{
	public:
		/**
		 * Push an attribute on the path
		 *
		 * @param attributeName The name of the attribute to push
		 */
		inline void pushAttribute(const std::string& attributeName)
		{ 
			assert(mLength < RTTIPATH_MAX_LENGTH); 
			mElements[mLength++] = RTTIPathElement(attributeName); 
		}

		/**
		 * Push an array element on the path
		 *
		 * @param index The index of the array element to push
		 */
		inline void pushArrayElement(int index)
		{ 
			assert(mLength < RTTIPATH_MAX_LENGTH); 
			mElements[mLength++] = RTTIPathElement(index); 
		}

		/**
		 * Remove last element from the path
		 */
		inline void popBack()
		{ 
			assert(mLength > 0); 
			mElements[--mLength] = RTTIPathElement(); 
		}

		/**
		 * Equality comparison
		 */
		bool operator==(const RTTIPath& lhs) const
		{
			if (mLength != lhs.mLength)
				return false;

			for (int index = 0; index < mLength; ++index)
				if (mElements[index] != lhs.mElements[index])
					return false;

			return true;
		}

		/**
		 * Inequality comparison
		 */
		bool operator!=(const RTTIPath& lhs) const
		{
			return !(*this == lhs);
		}

		/**
		 * Convert this RTTIPath to a string representation of the format "Attribute:ArrayIndex:Attribute"
		 *
		 * @return The string representation of this RTTIPath
		 */
		const std::string toString() const;

		/**
		 * Convert a string representation of an RTTIPath in the format "Attribute:ArrayIndex:Attribute" to an actual RTTIPath
		 *
		 * @return The RTTIPath
		 */
		static const RTTIPath fromString(const std::string& path);

		/**
		 * Resolve an RTTIPath against an Object
		 *
		 * @param object The object to resolve against
		 * @param path The resolved RTTI path
		 * @return Whether the resolve succeeded or not
		 */
		bool resolve(rtti::RTTIObject* object, ResolvedRTTIPath& resolvedPath) const;

	private:
		static const int	RTTIPATH_MAX_LENGTH = 16;			// Maximum number of elements on an RTTIPath
		RTTIPathElement		mElements[RTTIPATH_MAX_LENGTH];		// The elements on the path
		int					mLength = 0;						// Current length of the path
	};

	/**
	 * ResolvedRTTIPath is the 'resolved' version of an RTTIPath and can be used to get/set the value of the property being pointed to
	 * See RTTIPath for further documentation
	 */
	class ResolvedRTTIPath
	{
	public:
		/**
		 * Get the value of the property pointed to by this path
		 *
		 * @return The value of the property
		 */
		const rtti::Variant getValue() const;

		/**
		 * Set the value of the property pointed to by this path
		 *
		 * @param value The value to set
		 * @return Whether the value was successfully set or not
		 */
		bool setValue(const rtti::Variant& value);

		/**
		 * Get the type of the value pointed to by this path
		 *
		 * @return The type of the value
		 */
		const rtti::TypeInfo getType() const;

		/**
		 * Check whether this resolved path is empty
		 *
		 * @return Whether the path is empty or not
		 */
		bool isEmpty() const 
		{ 
			return mLength == 0; 
		}

		/**
		 * Check whether this path is valid
		 *
		 * @return Whether the path is valid or not
		 */
		bool isValid() const { return !isEmpty(); }

	private:
		friend class RTTIPath;

		/**
		 * Push the root object/attribute on the path. Note that this is different from a regular attribute in that no copy is made of the attribute value (it keeps a pointer to the root object)
		 */
		inline void	pushRoot(const rtti::Instance& instance, const rtti::Property& property)
		{ 
			assert(mLength < RTTIPATH_MAX_LENGTH); 
			mElements[mLength++] = ResolvedRTTIPathElement(instance, property); 
		}

		/**
		 * Push object/attribute on the path
		 */
		inline void	pushAttribute(const rtti::Variant& variant, const rtti::Property& property)
		{ 
			assert(mLength < RTTIPATH_MAX_LENGTH); 
			mElements[mLength++] = ResolvedRTTIPathElement(variant, property); 
		}

		/**
		 * Push array element on the path
		 */
		inline void	pushArrayElement(const rtti::Variant& array, int index)
		{ 
			assert(mLength < RTTIPATH_MAX_LENGTH); 
			mElements[mLength++] = ResolvedRTTIPathElement(array, index); 
		}

	private:
		static const int		RTTIPATH_MAX_LENGTH = 16;			// Maximum number of elements on an RTTIPath
		ResolvedRTTIPathElement	mElements[RTTIPATH_MAX_LENGTH];		// The elements on the path
		int						mLength = 0;						// Current length of the path
	};
}
