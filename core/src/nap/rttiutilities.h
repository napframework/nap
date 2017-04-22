#pragma once

// RTTI Includes
#include <rtti/rtti.h>

namespace nap
{
	class Object;

	/**
	* Copies rtti attributes from one object to another.
	* @param srcObject: the object to copy attributes from
	* @param dstObject: the target object
	*/
	void rttiCopyObject(const Object& srcObject, Object& dstObject);

	/**
	* Creates a new object with the same attributes as it's source.
	* @param object: the object to copy rtti attributes from.
	*/
	template<typename T>
	T* rttiCloneObject(T& object)
	{
		RTTI::TypeInfo type = object.get_type();
		T* copy = type.create<T>();
		rttiCopyObject(object, *copy);

		return copy;
	}

	/**
	* Tests whether the attributes of two objects have the same values.
	* @param objectA: first object to compare attributes from.
	* @param objectB: second object to compare attributes from.
	*/
	bool rttiAreObjectsEqual(const Object& objectA, const Object& objectB);

	/**
	* Searches through object's rtti attributes for attribute that have the 'file link' tag.
	* @param object: object to find file links from.
	* @param fileLinks: output array containing the filenames.
	*/
	void rttiFindFileLinks(const Object& object, std::vector<std::string>& fileLinks);

	/**
	* Searches through object's rtti attributes for pointer attributes.
	* @param object: object to find file links from.
	* @param fileLinks: output array containing the Objects being pointed to.
	*/
	void rttiFindObjectLinks(const Object& object, std::vector<Object*>& objectLinks);

} //< End Namespace nap

