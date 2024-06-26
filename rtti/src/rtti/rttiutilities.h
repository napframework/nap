/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Local Includes
#include "path.h"
#include "factory.h"
#include "unresolvedpointer.h"

// External Includes
#include <rtti/rtti.h>
#include <vector>
#include <unordered_set>
#include <utility/dllexport.h>

namespace nap
{
	namespace rtti
	{
		class Object;

		/**
		 * Represents a link from an object
		 */
		struct NAPAPI ObjectLink
		{
			const rtti::Object*	mSource;		// The object the link originates from
			Path				mSourcePath;	// The RTTIPath to the pointer property linking to the object
			rtti::Object*		mTarget;		// The object being linked to (i.e. target of the pointer)
		};

        using ObjectList = std::vector<Object*>;
        using ObjectSet = std::unordered_set<Object*>;

        /**
		 * Copies rtti attributes from one object to another.
		 * @param srcObject: the object to copy attributes from
		 * @param dstObject: the target object
		 */
		void NAPAPI copyObject(const rtti::Object& srcObject, rtti::Object& dstObject);

		/**
		 * Creates a new object with the same attributes as it's source.
		 * @param object the object to copy rtti attributes from.
		 * @param factory RTTI object factory.
		 * @return the cloned object
		 */
		template<typename T>
		std::unique_ptr<T> cloneObject(const T& object, rtti::Factory& factory)
		{
			T* copy = static_cast<T*>(factory.create(object.get_type()));
			copyObject(object, *copy);
			return std::unique_ptr<T>(copy);
		}

		/**
		 * Tests whether the attributes of two objects have the same values.
		 * @param objectA: first object to compare attributes from.
		 * @param objectB: second object to compare attributes from.
		 * @param unresolvedPointers: list of unresolved pointers that should be used for pointer comparisons in case of unresolved pointers.
		 */
		bool NAPAPI areObjectsEqual(const rtti::Object& objectA, const rtti::Object& objectB, const rtti::UnresolvedPointerList& unresolvedPointers = UnresolvedPointerList());

		/**
		 * Searches through object's rtti attributes for attribute that have the 'file link' tag.
		 * @param object: object to find file links from.
		 * @param fileLinks: output array containing the filenames.
		 */
		void NAPAPI findFileLinks(const rtti::Object& object, std::vector<std::string>& fileLinks);

		/**
		 * Searches through object's rtti attributes for pointer attributes.
		 * @param object: object to find file links from.
		 * @param objectLinks: output array containing the object links
		 */
		void NAPAPI findObjectLinks(const rtti::Object& object, std::vector<ObjectLink>& objectLinks);

		/**
		 * Helper to find the index of the unresolved pointer with the specified object and path combination
		 *
		 * @param unresolvedPointers The list of UnresolvedPointers to search in
		 * @param object The object that the UnresolvedPointer originates in
		 * @param path The path to the attribute on the object that the UnresolvedPointer originates in
		 *
		 * @return The index of the UnresolvedPointer in the specified list. -1 if not found.
		 */
		int NAPAPI findUnresolvedPointer(const UnresolvedPointerList& unresolvedPointers, const Object* object, const rtti::Path& path);

		/**
		 * Recursively traverses pointers of the given object and puts them in pointees. Basically traverses the object graph beneath object.
		 * @param object The root object to start traversing pointers from.
		 * @param pointees The resulting array of objects.
		 */
		void NAPAPI getPointeesRecursive(const rtti::Object& object, std::vector<rtti::Object*>& pointees);

		/**
		 * Calculate the version number of the specified type
		 *
		 * @param type The type to calculate the version number for
		 * @return The version number
		 */
		uint64_t NAPAPI getRTTIVersion(const rtti::TypeInfo& type);

		/**
		 * Recursively get all types derived from the specified type (including the base type itself)
		 * @param baseType The type to use as base
		 * @param types The resulting array of types
		 */
		void NAPAPI getDerivedTypesRecursive(const rtti::TypeInfo& baseType, std::vector<rtti::TypeInfo>& types);

		/**
		 * Checks if a description is provided for the given property.
		 * @return if a description is provided for the given property.
		 */
		bool NAPAPI hasDescription(const rtti::Property& property);

		/**
		 * Returns the description of a property.
		 * @return property description, nullptr when not defined.
		 */
		const NAPAPI char* getDescription(const rtti::Property& property);

		/**
		 * Checks if a description is provided for the given type, including base types.
		 * @return if a description is provided for the given type.
		 */
		bool NAPAPI hasDescription(const rtti::TypeInfo& type);

		/**
		 * Returns the description of a type, including base types.
		 * @return type description, nullptr when not defined.
		 */
		const NAPAPI char* getDescription(const rtti::TypeInfo& type);

	} //< End Namespace RTTI

}
