#pragma once

// RTTI Includes
#include <rtti/rtti.h>
#include "rttipath.h"
#include "factory.h"
#include "unresolvedpointer.h"
#include "utility/dllexport.h"

namespace nap
{
	namespace rtti
	{
		class RTTIObject;

		/**
		 * Represents a link from an object
		 */
		struct NAPAPI ObjectLink
		{
			const rtti::RTTIObject*	mSource;		// The object the link originates from
			RTTIPath				mSourcePath;	// The RTTIPath to the pointer property linking to the object
			rtti::RTTIObject*		mTarget;		// The object being linked to (i.e. target of the pointer)
		};


		/**
		* Copies rtti attributes from one object to another.
		* @param srcObject: the object to copy attributes from
		* @param dstObject: the target object
		*/
		void NAPAPI copyObject(const rtti::RTTIObject& srcObject, rtti::RTTIObject& dstObject);

		/**
		* Creates a new object with the same attributes as it's source.
		* @param object: the object to copy rtti attributes from.
		*/
		template<typename T>
		std::unique_ptr<T> NAPAPI cloneObject(T& object, rtti::Factory& factory)
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
		bool NAPAPI areObjectsEqual(const rtti::RTTIObject& objectA, const rtti::RTTIObject& objectB, const rtti::UnresolvedPointerList& unresolvedPointers = UnresolvedPointerList());

		/**
		* Searches through object's rtti attributes for attribute that have the 'file link' tag.
		* @param object: object to find file links from.
		* @param fileLinks: output array containing the filenames.
		*/
		void findFileLinks(const rtti::RTTIObject& object, std::vector<std::string>& fileLinks);

		/**
		* Searches through object's rtti attributes for pointer attributes.
		* @param object: object to find file links from.
		* @param objectLinks: output array containing the object links
		*/
		void NAPAPI findObjectLinks(const rtti::RTTIObject& object, std::vector<ObjectLink>& objectLinks);

		/**
		* Helper to find the index of the unresolved pointer with the specified object and path combination
		*
		* @param unresolvedPointers The list of UnresolvedPointers to search in
		* @param object The object that the UnresolvedPointer originates in
		* @param path The path to the attribute on the object that the UnresolvedPointer originates in
		*
		* @return The index of the UnresolvedPointer in the specified list. -1 if not found.
		*/
		int NAPAPI findUnresolvedPointer(const UnresolvedPointerList& unresolvedPointers, const RTTIObject* object, const rtti::RTTIPath& path);

		/**
		* Calculate the version number of the specified type
		*
		* @param type The type to calculate the version number for
		* @return The version number
		*/
		std::size_t NAPAPI getRTTIVersion(const rtti::TypeInfo& type);

	} //< End Namespace RTTI

}
