#pragma once

#include <rtti/path.h>
#include <QMetaType>

namespace napkin
{
	/**
	 * A path to a property, including its object.
	 * This class carries both the object and the property path.
	 */
	class PropertyPath
	{
	public:
		/**
		 * Create an invalid path.
		 */
		PropertyPath() = default;

		/**
		 * Copy constructor
		 * @param other
		 */
		PropertyPath(const PropertyPath& other);

		/**
		 * @param obj The object this property is on
		 * @param path The path to the property
		 */
		PropertyPath(nap::rtti::Object& obj, const nap::rtti::Path& path);

		/**
		 * @param obj The object this property is on
		 * @param path The path to the property
		 */
		PropertyPath(nap::rtti::Object& obj, const std::string& path);

		/**
		 * @return The value of this property
		 */
		rttr::variant getValue() const;

		/**
		 * Set the value of this property
		 */
		void setValue(rttr::variant value);

		/**
		 * @return The property this path points to
		 */
		rttr::property getProperty() const;

		/**
		 * @return The type of the property
		 */
		rttr::type getType() const;

		/**
		 * @return A child path of this propertypath
		 */
		PropertyPath getChild(const std::string& name) const;

		/**
		 * @return obj The object this property is on
		 */
		nap::rtti::Object& getObject() const { return *mObject; }

		/**
		 * @return path The path to the property
		 */
		const nap::rtti::Path& getPath() const { return mPath; }

		/**
		 * Resolve a property path
		 */
		nap::rtti::ResolvedPath resolve() const;

		/**
		 * If this path refers to an array property, get the element type
		 * If this path does not refer to an array property, return type::empty()
		 */
		rttr::type getArrayElementType() const;

		/**
		 * If this path refers to an array property, return the length of the array.
		 */
		size_t getArrayLength()const;

		/**
		 * Get the path to an element of the array (if this represents an array)
		 * @return A path to an element of the array or an invalid path if it cannot be found.
		 */
		PropertyPath getArrayElement(size_t index) const;

		/**
		 * @return Wrapped type
		 */
		rttr::type getWrappedType() const;

		/**
		 * @return A string representation of this path
		 */
		std::string toString() const;

		/**
		 * @return If the path is a valid one
		 */
		bool isValid() const;

		/**
		 * @return true if the property pointed to is representing a pointer
		 */
		bool isPointer() const;

		/**
		 * @return true if the property pointed to is representing an embedded pointer
		 */
		bool isEmbeddedPointer() const;

		/**
		 * @return true if the property pointed to represents a non-embedded pointer
		 */
		bool isNonEmbeddedPointer() const;

		/**
		 * @return true if the property pointed to is representing an enum
		 */
		bool isEnum() const;

		/**
		 * @return true if the property pointed to is an array
		 */
		bool isArray() const;

		/**
		 * If this path refers to a pointer, get the Object it's pointing to.
		 * @return The object this property is pointing to or nullptr if this path does not represent a pointer.
		 */
		nap::rtti::Object* getPointee() const;

		/**
		 * If this path refers to a pointer, set the Object it's pointing to
		 * @param pointee The Object this property will be pointing to.
		 */
		void setPointee(nap::rtti::Object* pointee);

		/**
		 * @param other The property to compare to
		 * @return true if the both property paths point to the same property
		 */
		bool operator==(const PropertyPath& other) const;


	private:
		nap::rtti::Object* mObject = nullptr;
		nap::rtti::Path mPath;
	};
}

Q_DECLARE_METATYPE(napkin::PropertyPath)