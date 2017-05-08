#include <rtti/rttiutilities.h>
#include <nap/object.h>

namespace RTTI
{
	template<class FUNC>
	void VisitRTTIPropertiesRecursive(const Variant& variant, RTTIPath& path, FUNC& visitFunc)
	{
		// Extract wrapped type
		auto value_type = variant.get_type();
		auto actual_type = value_type.is_wrapper() ? value_type.get_wrapped_type() : value_type;

		// If this is an array, recurse into the array for each element
		if (actual_type.is_array())
		{
			VariantArray array = variant.create_array_view();

			// Recursively visit each array element
			for (int index = 0; index < array.get_size(); ++index)
			{
				path.PushArrayElement(index);

				RTTI::Variant array_value = array.get_value_as_ref(index);
				VisitRTTIPropertiesRecursive(array_value, path, visitFunc);

				path.PopBack();
			}
		}
		else
		{
			// Recursively visit each property of the type
			for (const RTTI::Property& property : actual_type.get_properties())
			{
				path.PushAttribute(property.get_name().data());

				RTTI::Variant value = property.get_value(variant);
				visitFunc(variant, property, value, path);

				VisitRTTIPropertiesRecursive(value, path, visitFunc);

				path.PopBack();
			}
		}
	}

	template<class FUNC>
	void VisitRTTIProperties(const Instance& instance, RTTIPath& path, FUNC& visitFunc)
	{
		// Recursively visit each property of the type
		for (const RTTI::Property& property : instance.get_derived_type().get_properties())
		{
			path.PushAttribute(property.get_name().data());

			RTTI::Variant value = property.get_value(instance);
			visitFunc(instance, property, value, path);

			VisitRTTIPropertiesRecursive(value, path, visitFunc);

			path.PopBack();
		}
	}

	struct ObjectLinkVisitor
	{
	public:
		ObjectLinkVisitor(const nap::Object& sourceObject, std::vector<ObjectLink>& objectLinks) :
			mSourceObject(sourceObject),
			mObjectLinks(objectLinks)
		{
		}

		void operator()(const Instance& instance, const Property& property, const Variant& value, const RTTIPath& path)
		{
			if (!property.get_type().is_pointer())
				return;

			assert (value.get_type().is_derived_from<nap::Object>());

			mObjectLinks.push_back({ &mSourceObject, path, value.convert<nap::Object*>() });
		}

	private:
		const nap::Object&				mSourceObject;
		std::vector<ObjectLink>&	mObjectLinks;
	};

	struct FileLinkVisitor
	{
	public:
		FileLinkVisitor(std::vector<std::string>& fileLinks) :
			mFileLinks(fileLinks)
		{
		}

		void operator()(const Instance& instance, const Property& property, const Variant& value, const RTTIPath& path)
		{
			if (!property.get_metadata(RTTI::EPropertyMetaData::FileLink).is_valid())
				return;

			assert (value.get_type().is_derived_from<std::string>());

			mFileLinks.push_back(value.convert<std::string>());
		}

	private:
		std::vector<std::string>&	mFileLinks;
	};

	/**
	 * Helper function to recursively check whether two variants (i.e. values) are equal
	 * Correctly deals with arrays and nested compounds, but note: does not follow pointers
	 */
	bool areVariantsEqualRecursive(const RTTI::Variant& variantA, const RTTI::Variant& variantB, EPointerComparisonMode pointerComparisonMode)
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

				if (!areVariantsEqualRecursive(array_value_a, array_value_b, pointerComparisonMode))
					return false;
			}
		}
		else
		{
			// Special case handling for pointers so we can compare by ID or by actual pointer value
			if (value_type.is_pointer())
			{
				// If we don't want to compare by ID, just check the pointers directly
				if (pointerComparisonMode == EPointerComparisonMode::BY_POINTER)
				{
					return is_wrapper ? (variantA.extract_wrapped_value() == variantB.extract_wrapped_value()) : (variantA == variantB);
				}
				else if (pointerComparisonMode == EPointerComparisonMode::BY_ID)
				{
					// Extract the pointer
					RTTI::Variant value_a = is_wrapper ? variantA.extract_wrapped_value() : variantA;
					RTTI::Variant value_b = is_wrapper ? variantB.extract_wrapped_value() : variantB;

					// Can only compare pointers that are of type Object
					assert(value_a.get_type().is_derived_from<nap::Object>() && value_b.get_type().is_derived_from<nap::Object>());

					// Extract the objects
					nap::Object* object_a = value_a.convert<nap::Object*>();
					nap::Object* object_b = value_b.convert<nap::Object*>();

					// If both are null, they're equal
					if (object_a == nullptr && object_b == nullptr)
						return true;

					// If only one is null, they can't be equal
					if (object_a == nullptr || object_b == nullptr)
						return false;

					// Check whether the IDs match
					return object_a->mID == object_b->mID;
				}
				else
				{
					assert(false);
				}
			}

			// If the type of this variant is a primitive type or non-primitive type with no RTTI properties,
			// we perform a normal comparison
			auto child_properties = actual_type.get_properties();
			if (RTTI::isPrimitive(value_type) || child_properties.empty())
				return is_wrapper ? (variantA.extract_wrapped_value() == variantB.extract_wrapped_value()) : (variantA == variantB);

			// Recursively compare each property of the compound
			for (const RTTI::Property& property : child_properties)
			{
				RTTI::Variant value_a = property.get_value(variantA);
				RTTI::Variant value_b = property.get_value(variantB);
				if (!areVariantsEqualRecursive(value_a, value_b, pointerComparisonMode))
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
	bool areObjectsEqual(const nap::Object& objectA, const nap::Object& objectB, EPointerComparisonMode pointerComparisonMode)
	{
		RTTI::TypeInfo typeA = objectA.get_type();
		assert(typeA == objectB.get_type());

		for (const RTTI::Property& property : typeA.get_properties())
		{
			RTTI::Variant valueA = property.get_value(objectA);
			RTTI::Variant valueB = property.get_value(objectB);
			if (!areVariantsEqualRecursive(valueA, valueB, pointerComparisonMode))
				return false;
		}

		return true;
	}


	/**
	* Searches through object's rtti attributes for attribute that have the 'file link' tag.
	*/
	void findFileLinks(const nap::Object& object, std::vector<std::string>& fileLinks)
	{
		fileLinks.clear();

		RTTIPath path;
		FileLinkVisitor visitor(fileLinks);
		VisitRTTIProperties(object, path, visitor);
	}
	
	
	/**
	* Searches through object's rtti attributes for pointer attributes.
	*/
	void findObjectLinks(const nap::Object& object, std::vector<ObjectLink>& objectLinks)
	{
		objectLinks.clear();

		RTTIPath path;
		ObjectLinkVisitor visitor(object, objectLinks);
		VisitRTTIProperties(object, path, visitor);
	}
}
