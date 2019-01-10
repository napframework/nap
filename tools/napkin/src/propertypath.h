#pragma once

#include <rtti/path.h>
#include <QMetaType>
#include <entity.h>
#include <scene.h>

namespace napkin
{
	class PropertyPath;

	using PropertyVisitor = std::function<bool(const PropertyPath& path)>;

	/**
	 * Flags used in property path iteration/recursion
	 */
	enum IterFlag : int
	{
		Resursive 					= 1 << 0,	// Recursively find children in the property path
		FollowPointers 				= 1 << 1,	// Resolve regular pointers and visit the resolved object's properties
		FollowEmbeddedPointers 		= 1 << 2,	// Resolve embedded pointers and visit the resolved object's properties
	};

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
		 * Create a PropertyPath to an object
		 * @param obj The object to create the path to.
		 */
		PropertyPath(nap::rtti::Object& obj);

		/**
		 * Create a path to an object that is to be instantiated.
		 * @param rootEntity Contains the instance property data
		 * @param obj
		 */
		PropertyPath(nap::RootEntity& rootEntity, nap::rtti::Object& obj);

		/**
		 * Create a PropertyPath using an Object and a nap::rtti::Path
		 * @param obj The object this property is on
		 * @param path The path to the property
		 */
		PropertyPath(nap::rtti::Object& obj, const nap::rtti::Path& path);

		/**
		 * Create a PropertyPath using an Object and a path as string
		 * @param obj The object this property is on
		 * @param path The path to the property
		 */
		PropertyPath(nap::rtti::Object& obj, const std::string& path);

		/**
		 * Create a PropertyPath using an Object and a property
		 * @param obj
		 * @param prop
		 */
		PropertyPath(nap::rtti::Object& obj, rttr::property prop);

		/**
		 * @return The last part of the property name (not including the path)
		 */
		const std::string getName() const;

		/**
		 * @return The value of this property
		 */
		rttr::variant getValue() const;

		/**
		 * Set the value of this property
		 */
		void setValue(rttr::variant value);

		/**
		 * Get the parent of this path
		 */
		PropertyPath getParent() const;

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
		 * @return True if this path represents an instance
		 */
		bool isInstance() const { return mRootEntity != nullptr; }

		/**
		 * @return true when the path points to a property, false when it points to an Object
		 */
		bool hasProperty() const { return mPath.length() > 0; }

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

		/**
		 * Iterate over the children of this property and call PropertyVisitor for each child.
		 * @param visitor The function to be called on each iteration, return false from this function to stop iteration
		 * @param flags Provide true to also iterate the children's children and so on
		 */
		void iterateChildren(PropertyVisitor visitor, int flags) const;

		/**
		 * Get this properties children if it has any.
		 * @param flags Provide true to also get the children's children and so on
		 * @return All children of this property
		 */
		std::vector<PropertyPath> getChildren(int flags = IterFlag::FollowEmbeddedPointers) const;

		/**
		 * Iterate over an Object's properties, top-level or recursive
		 * @param obj The object's properties to iterator over
		 * @param visitor This function will be called for every property.
		 * 				  Return false from this function to stop the iteration
		 * @param flags Whether to recurse into sub-properties or not.
		 */
		static void iterateProperties(nap::rtti::Object& obj, PropertyVisitor visitor, int flags = 0);

		/**
		 * Get all properties of an object.
		 * @param obj The object to retrieve the properties from
		 * @param flags Also get the property children recursively?
		 * @return All the properties on this object
		 */
		static std::vector<PropertyPath> getProperties(nap::rtti::Object& obj, int flags = 0);

	private:
		void iterateArrayElements(PropertyVisitor visitor, int flags) const;
		void iterateChildrenProperties(PropertyVisitor visitor, int flags) const;
		void iteratePointerProperties(PropertyVisitor visitor, int flags) const;

		nap::RootEntity* mRootEntity = nullptr; // contains the root entity in the scene and the instance properties
		nap::rtti::Object* mObject = nullptr; // the object on which the property exists
		nap::rtti::Path mPath; // the path to the property on the object
	};
}

Q_DECLARE_METATYPE(napkin::PropertyPath)