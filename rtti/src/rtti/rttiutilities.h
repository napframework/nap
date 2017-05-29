#pragma once

// RTTI Includes
#include <rtti/rtti.h>
#include "rttipath.h"
#include "factory.h"

namespace nap
{
	namespace rtti
	{
		class RTTIObject;

		/**
		 * Represents the comparison mode to use when comparing pointers
		 */
		enum class EPointerComparisonMode
		{
			BY_POINTER,							// Compare pointers by pointer value
			BY_ID								// Compare pointers by the ID of the rtti::RTTIObject they're pointing to
		};

		/**
		 * Represents a link from an object
		 */
		struct ObjectLink
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
		void copyObject(const rtti::RTTIObject& srcObject, rtti::RTTIObject& dstObject);

		/**
		* Creates a new object with the same attributes as it's source.
		* @param object: the object to copy rtti attributes from.
		*/
		template<typename T>
		std::unique_ptr<T> cloneObject(T& object, rtti::Factory& factory)
		{
			T* copy = static_cast<T*>(factory.create(object.get_type()));
			copyObject(object, *copy);

			return std::unique_ptr<T>(copy);
		}

		/**
		* Tests whether the attributes of two objects have the same values.
		* @param objectA: first object to compare attributes from.
		* @param objectB: second object to compare attributes from.
		*/
		bool areObjectsEqual(const rtti::RTTIObject& objectA, const rtti::RTTIObject& objectB, EPointerComparisonMode pointerComparisonMode = EPointerComparisonMode::BY_POINTER);

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
		void findObjectLinks(const rtti::RTTIObject& object, std::vector<ObjectLink>& objectLinks);

		/**
		* Calculate the version number of the specified type
		*
		* @param type The type to calculate the version number for
		* @return The version number
		*/
		std::size_t getRTTIVersion(const rtti::TypeInfo& type);

	} //< End Namespace RTTI

}
