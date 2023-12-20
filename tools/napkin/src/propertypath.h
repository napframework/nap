/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

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


	/**
	 * Maps a name to a specific index.
	 * Used as a part of a path to objects and properties.
	 */
	class NameIndex
	{
		friend class PropertyPath;
	public:
		/**
		 * Creates the name / index pair
		 */
		NameIndex(const std::string& nameIndex);

		/**
		 * @return name at index as string
		 */
		std::string toString() const				{ return  mIndex < 0 ? mID : nap::utility::stringFormat("%s:%d", mID.c_str(), mIndex); }

		/**
		 * @return name at index as string
		 */
		operator std::string() const				{ return toString(); }

	private:
		std::string mID;							// the name
		int mIndex = -1;							// the index of the name
	};
	using PPath = std::vector<NameIndex>;


	/**
	 * A path to a property, including its object.
	 * This class carries both the object and the property path.
	 */
	class PropertyPath
	{
		friend class Document;
	public:
		/**
		 * Creates an invalid path
		 */
		PropertyPath() = default;

		/**
		 * Create a PropertyPath to an object
		 * @param obj The object to create the path to.
		 */
		PropertyPath(nap::rtti::Object& obj, Document& doc);

		PropertyPath(const std::string& abspath, Document& doc);

		PropertyPath(const std::string& abspath, const std::string& proppath, Document& doc);

		PropertyPath(const PPath& abspath, Document& doc);
		
		PropertyPath(const PPath& absPath, const PPath& propPath, Document& doc);

		/**
		 * Create a PropertyPath using an Object and a nap::rtti::Path
		 * @param obj The object this property is on
		 * @param path The path to the property
		 */
		PropertyPath(nap::rtti::Object& obj, const nap::rtti::Path& path, Document& doc);

		/**
		 * Create a PropertyPath using an Object and a property
		 * @param obj
		 * @param prop
		 */
		PropertyPath(nap::rtti::Object& obj, rttr::property prop, Document& doc);

		/**
		 * @return The last part of the property name
		 */
		const std::string getName() const;

		/**
		 * @return The value of this property
		 */
		rttr::variant getValue() const;

		/**
		 * Set the value of this property
		 * @param value the new property value
		 * @return if the value was set
		 */
		bool setValue(rttr::variant value);

		/**
		 * If this path refers to a pointer, get the Object it's pointing to.
		 * Return nullptr if the object doesn't exist or is not a pointer.
		 * @return The object this property is pointing to, nullptr if the object doesn't exist or isn't a pointer
		 */
		nap::rtti::Object* getPointee() const;

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
		 * @return obj The object that has the property, nullptr if the path is invalid
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
		 * Returns if the array is editable.
		 * @return if the array is editable.
		 */
		bool getArrayEditable() const;

		/**
		 * @return Wrapped type
		 */
		rttr::type getWrappedType() const;

		/**
		 * @return A string representation of this path
		 */
		std::string toString() const;

		/**
		 * @return True if this path edits an instance property
		 */
		bool isInstanceProperty() const;

		/**
		 * @return True if this path represents an instance and the value has been overridden
		 */
		bool isOverridden() const;

		/**
		 * Remove overridden value
		 */
		void removeOverride();

		/**
		 * @return True if this path has any children with an override
		 */
		bool hasOverriddenChildren() const;

		/**
		 * @return true when the path points to a property, false when it points to an Object
		 */
		bool hasProperty() const;

		/**
		 * @return If the path is valid, this means the path can be resolved for the associated document
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
		 * @return if the property pointed to is representing a color
		 */
		bool isColor() const;

		/**
		 * @return true if the property pointed to is an array
		 */
		bool isArray() const;

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

		/**
		 * @return entity as root in the scene, along with it's instance properties
		 */
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
		int getEntityIndex() const;

		/**
		 * Replaces every occurrence of oldName with newName
		 * @param oldName old object name
		 * @param newName new object name
		 */
		void updateObjectName(const std::string& oldName, const std::string& newName);

		/**
		 * Checks if the path refers to the object with the given name
		 */
		bool referencesObject(const std::string& name);

		/**
		 * @return data model
		 */
		Document* getDocument() const { return mDocument; }

	private:
		void iterateArrayElements(PropertyVisitor visitor, int flags) const;
		void iterateChildrenProperties(PropertyVisitor visitor, int flags) const;
		void iteratePointerProperties(PropertyVisitor visitor, int flags) const;

		nap::ComponentInstanceProperties* instanceProps() const;
		nap::ComponentInstanceProperties& getOrCreateInstanceProps();
		void removeInstanceValue(const nap::TargetAttribute* targetAttr, rttr::variant& val) const;
		/**
		 * This PropertyPath is most likely pointing to a Component, retrieve it here.
		 * @return The component this PropertyPath is pointing to.
		 */
		nap::Component* component() const;
		nap::TargetAttribute* targetAttribute() const;
		nap::TargetAttribute& getOrCreateTargetAttribute();

		std::string objectPathStr() const;
		std::string propPathStr() const;
		rttr::variant patchValue(const rttr::variant& value) const;

		Document* mDocument = nullptr;
		PPath mObjectPath;									//< Objects pointing to the property
		PPath mPropertyPath;								//< Path to property
		mutable nap::rtti::Object* mObject = nullptr;		//< Resolved object that holds the property
		mutable nap::RootEntity* mRootEntity = nullptr;		//< Root entity in the scene, can be null
		mutable bool mRootQueried = false;					//< If the root has been queried
	};
}

Q_DECLARE_METATYPE(napkin::PropertyPath)
