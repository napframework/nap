#include "rttiutilities.h"
#include "Object.h"

namespace nap
{

	/**
	* Copies rtti attributes from one object to another.
	*/
	void rttiCopyObject(const Object& srcObject, Object& dstObject)
	{
		RTTI::TypeInfo type = srcObject.get_type();
		assert(type == dstObject.get_type());

		for (const RTTI::Property& property : type.get_properties())
		{
			RTTI::Variant new_value = property.get_value(srcObject);
			property.set_value(dstObject, new_value);
		}
	}


	/**
	* Tests whether the attributes of two objects have the same values.
	* @param objectA: first object to compare attributes from.
	* @param objectB: second object to compare attributes from.
	*/
	bool rttiAreObjectsEqual(const Object& objectA, const Object& objectB)
	{
		RTTI::TypeInfo typeA = objectA.get_type();
		assert(typeA == objectB.get_type());

		for (const RTTI::Property& property : typeA.get_properties())
		{
			RTTI::Variant valueA = property.get_value(objectA);
			RTTI::Variant valueB = property.get_value(objectB);
			if (valueA != valueB)
				return false;
		}

		return true;
	}


	/**
	* Searches through object's rtti attributes for attribute that have the 'file link' tag.
	*/
	void rttiFindFileLinks(const Object& object, std::vector<std::string>& fileLinks)
	{
		RTTI::TypeInfo type = object.get_type();

		fileLinks.clear();

		for (const RTTI::Property& property : type.get_properties())
		{
			if (property.get_metadata(RTTI::EPropertyMetaData::FileLink).is_valid())
			{
				std::string filename = property.get_value(object).convert<std::string>();
				fileLinks.push_back(filename);
			}
		}
	}


	/**
	* Searches through object's rtti attributes for pointer attributes.
	*/
	void rttiFindObjectLinks(const Object& object, std::vector<Object*>& objectLinks)
	{
		RTTI::TypeInfo type = object.get_type();

		objectLinks.clear();

		for (const RTTI::Property& property : type.get_properties())
		{
			if (property.get_type().is_pointer())
			{
				Object* target_object = property.get_value(object).convert<Object*>();
				objectLinks.push_back(target_object);
			}
		}
	}


}
