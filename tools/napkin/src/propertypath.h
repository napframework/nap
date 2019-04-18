#pragma once

#include <rtti/path.h>
#include <QMetaType>
#include <entity.h>
#include <scene.h>

namespace napkin
{
	class PropertyPath;
	class Document;

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

	struct NameIndex
	{
		NameIndex(const std::string& nameIndex);

		std::string toString() const;
		operator std::string() const { return toString(); }
		std::string mID;
		int mIndex = -1;
	};

	using PPath = std::vector<NameIndex>;

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
		 * Create a PropertyPath to an object
		 * @param obj The object to create the path to.
		 */
		PropertyPath(nap::rtti::Object& obj);

		PropertyPath(const std::string& abspath);

		PropertyPath(const std::string& abspath, const std::string& proppath);

		PropertyPath(const PPath& abspath);
		PropertyPath(const PPath& absPath, const PPath& propPath);

		/**
		 * Create a PropertyPath using an Object and a nap::rtti::Path
		 * @param obj The object this property is on
		 * @param path The path to the property
		 */
		PropertyPath(nap::rtti::Object& obj, const nap::rtti::Path& path);

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
		nap::rtti::Object* getObject() const;

		/**
		 * @return path The path to the property
		 */
		nap::rtti::Path getPath() const;

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
		bool isInstanceProperty() const;

		/**
		 * @return True if this path represents an instance and the value has been overridden
		 */
		bool isOverridden() const;

		/**
		 * @return True if this path has any children with an override
		 */
		bool hasOverriddenChildren() const;

		/**
		 * @return true when the path points to a property, false when it points to an Object
		 */
		bool hasProperty() const;

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
		 * @param other The property to compare to
		 * @return false if the both property paths point to the same property
		 */
		bool operator!=(const PropertyPath& other) const { return !(*this == other); }

		/**
		 * Iterate over the children of this property and call PropertyVisitor for each child.
		 * @param visitor The function to be called on each iteration, return false from this function to stop iteration
		 * @param flags Provide traversal flags
		 */
		void iterateChildren(PropertyVisitor visitor, int flags = IterFlag::FollowEmbeddedPointers) const;

		/**
		 * Get this property's children if it has any.
		 * @param flags Provide true to also get the children's children and so on
		 * @return All children of this property
		 */
		std::vector<PropertyPath> getChildren(int flags = IterFlag::FollowEmbeddedPointers) const;

		/**
		 * Iterate over this object's root properties.
		 * @param visitor The function to be called on each iteration, return false from this function to stop iteration
		 * @param flags Provide traversal flags
		 */
		void iterateProperties(PropertyVisitor visitor, int flags = 0) const;
		std::vector<PropertyPath> getProperties(int flags = 0) const;
		std::string getComponentInstancePath() const;
		nap::RootEntity* getRootEntity() const;

		/**
		 * If this path represents a child entity, get the ID-DISAMBIGUATING index of this entity under its parent.
		 * TODO: This should go as soon as we can make instance properties editor-suited
		 * @return the (not a real) "index" or -1 if it failed.
		 */
		int getInstanceChildEntityIndex() const;

		/**
		 * Get the actual index of this child Entity under its parent Entity
		 * @return the actual index or -1 if not found
		 */
		int getRealChildEntityIndex() const;

	private:
		void iterateArrayElements(PropertyVisitor visitor, int flags) const;
		void iterateChildrenProperties(PropertyVisitor visitor, int flags) const;
		void iteratePointerProperties(PropertyVisitor visitor, int flags) const;
		Document* document() const;

		nap::ComponentInstanceProperties* instanceProps() const;
		nap::ComponentInstanceProperties& getOrCreateInstanceProps();

		/**
		 * This PropertyPath is most likely pointing to a Component, retrieve it here.
		 * @return The component this PropertyPath is pointing to.
		 */
		nap::Component* component() const;
		nap::TargetAttribute* targetAttribute() const;
		nap::TargetAttribute& getOrCreateTargetAttribute();

		std::string objectPathStr() const;
		std::string propPathStr() const;

		PPath mObjectPath;
		PPath mPropertyPath;
	};
}

Q_DECLARE_METATYPE(napkin::PropertyPath)