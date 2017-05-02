#include "rttiutilities.h"
#include "Object.h"

namespace RTTI
{
	/**
	 * Helper function to recursively check whether two variants (i.e. values) are equal
	 * Correctly deals with arrays and nested compounds, but note: does not follow pointers
	 */
	bool areVariantsEqualRecursive(const RTTI::Variant& variantA, const RTTI::Variant& variantB)
	{
		// Extract wrapped type
		auto value_type = variantA.get_type();
		auto actual_type = value_type.is_wrapper() ? value_type.get_wrapped_type() : value_type;
		bool is_wrapper = actual_type != value_type;
		
		// Types must match
		assert(value_type == variantB.get_type());

		// If this is an array, compare the array element-wise
		if (value_type.is_array())
		{
			// Get the arrays
			RTTI::VariantArray array_a = variantA.create_array_view();
			RTTI::VariantArray array_b = variantB.create_array_view();

			// If the sizes don't match, the arrays can't be equal
			if (array_a.get_size() != array_b.get_size())
				return false;

			// Recursively compare each array element
			for (int index = 0; index < array_a.get_size(); ++index)
			{
				RTTI::Variant array_value_a = array_a.get_value_as_ref(index);
				RTTI::Variant array_value_b = array_a.get_value_as_ref(index);

				if (!areVariantsEqualRecursive(array_value_a, array_value_b))
					return false;
			}
		}
		else
		{
			// If the type of this variant is a primitive type or non-primitive type with no RTTI properties,
			// we perform a normal comparison
			auto child_properties = actual_type.get_properties();
			if (value_type.is_arithmetic() || value_type.is_pointer() || child_properties.empty())
				return is_wrapper ? (variantA.extract_wrapped_value() == variantB.extract_wrapped_value()) : (variantA == variantB);

			// Recursively compare each property of the compound
			for (const RTTI::Property& property : child_properties)
			{
				RTTI::Variant value_a = property.get_value(variantA);
				RTTI::Variant value_b = property.get_value(variantB);
				if (!areVariantsEqualRecursive(value_a, value_b))
					return false;
			}
		}

		return true;
	}


	/**
	* Copies rtti attributes from one object to another.
	*/
	void copyObject(const nap::Object& srcObject, nap::Object& dstObject)
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
	bool areObjectsEqual(const nap::Object& objectA, const nap::Object& objectB)
	{
		RTTI::TypeInfo typeA = objectA.get_type();
		assert(typeA == objectB.get_type());

		for (const RTTI::Property& property : typeA.get_properties())
		{
			RTTI::Variant valueA = property.get_value(objectA);
			RTTI::Variant valueB = property.get_value(objectB);
			if (!areVariantsEqualRecursive(valueA, valueB))
				return false;
		}

		return true;
	}


	/**
	* Searches through object's rtti attributes for attribute that have the 'file link' tag.
	*/
	void findFileLinks(const nap::Object& object, std::vector<std::string>& fileLinks)
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
	void findObjectLinks(const nap::Object& object, std::vector<nap::Object*>& objectLinks)
	{
		RTTI::TypeInfo type = object.get_type();

		objectLinks.clear();

		for (const RTTI::Property& property : type.get_properties())
		{
			if (property.get_type().is_pointer())
			{
				nap::Object* target_object = property.get_value(object).convert<nap::Object*>();
				objectLinks.push_back(target_object);
			}
		}
	}
}
