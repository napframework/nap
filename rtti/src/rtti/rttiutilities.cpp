#include <rtti/rttiutilities.h>
#include <rtti/rttiobject.h>

namespace nap
{
	namespace rtti
	{
		/**
		 * Helper function to recursively visit the properties of an object (without following pointers).
		 *
		 * A functor-like parameter can be provided that will be invoked for each property visited. The signature of the parameter should be as follows:
		 *		void visitFunction(const rtti::Instance& instance, const rtti::Property& property, const rtti::Variant& value, const rtti::RTTIPath& path)
		 *
		 * The parameters of the visit function are as follows:
		 *  - instance: the instance (object) we're visiting
		 *  - property: the property on the object we're visiting
		 *  - value: the value of the property we're visiting
		 *  - path: full RTTIPath to the property we're visiting
		 *
		 */
		template<class FUNC>
		void VisitRTTIPropertiesRecursive(const rtti::Property& property, const Variant& variant, RTTIPath& path, FUNC& visitFunc)
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
					path.pushArrayElement(index);

					rtti::Variant array_value = array.get_value(index);

					// Invoke visit func
					visitFunc(variant, property, array_value, path);

					// Recurse
					if (!array_value.get_type().get_wrapped_type().is_pointer()) // Don't recurse into properties of pointers
						VisitRTTIPropertiesRecursive(property, array_value, path, visitFunc);

					path.popBack();
				}
			}
			else
			{
				// Recursively visit each property of the type
				for (const rtti::Property& nested_property : actual_type.get_properties())
				{
					path.pushAttribute(nested_property.get_name().data());

					rtti::Variant value = nested_property.get_value(variant);

					// Invoke visit func
					visitFunc(variant, nested_property, value, path);

					// Recurse
					if (!actual_type.is_pointer()) // Don't recurse into properties of pointers
						VisitRTTIPropertiesRecursive(nested_property, value, path, visitFunc);

					path.popBack();
				}
			}
		}

		/**
		 * Helper function to recursively visit the properties of an object (without following pointers).
		 *
		 * A functor-like parameter can be provided that will be invoked for each property visited. The signature of the parameter should be as follows:
		 *		void visitFunction(const rtti::Instance& instance, const rtti::Property& property, const rtti::Variant& value, const rtti::RTTIPath& path)
		 *
		 * The parameters of the visit function are as follows:
		 *  - instance: the instance (object) we're visiting
		 *  - property: the property on the object we're visiting
		 *  - value: the value of the property we're visiting
		 *  - path: full RTTIPath to the property we're visiting
		 *
		 */
		template<class FUNC>
		void VisitRTTIProperties(const Instance& instance, RTTIPath& path, FUNC& visitFunc)
		{
			// Recursively visit each property of the type
			for (const rtti::Property& property : instance.get_derived_type().get_properties())
			{
				path.pushAttribute(property.get_name().data());

				rtti::Variant value = property.get_value(instance);

				// Invoke visit func
				visitFunc(instance, property, value, path);

				rtti::TypeInfo actual_type = property.get_type().is_wrapper() ? property.get_type().get_wrapped_type() : property.get_type();

				// Recurse
				if (!actual_type.is_pointer()) // Don't recurse into properties of pointers
					VisitRTTIPropertiesRecursive(property, value, path, visitFunc);

				path.popBack();
			}
		}


		/**
		 * ObjectLinkVisitor is a functor-like object that can be used as an argument to VisitRTTIProperties and collects all links to other rtti::RTTIObjects
		 */
		struct ObjectLinkVisitor
		{
		public:
			ObjectLinkVisitor(const rtti::RTTIObject& sourceObject, std::vector<ObjectLink>& objectLinks) :
				mSourceObject(sourceObject),
				mObjectLinks(objectLinks)
			{
			}

			void operator()(const Instance& instance, const Property& property, const Variant& value, const RTTIPath& path)
			{
				auto actual_type = value.get_type().is_wrapper() ? value.get_type().get_wrapped_type() : value.get_type();
				if (!actual_type.is_pointer())
					return;

				Variant actual_value = value.get_type().is_wrapper() ? value.extract_wrapped_value() : value;

				assert(actual_value.get_type().is_derived_from<rtti::RTTIObject>());
				mObjectLinks.push_back({ &mSourceObject, path, actual_value.convert<rtti::RTTIObject*>() });
			}

		private:
			const rtti::RTTIObject&		mSourceObject;	// The object we're visiting (i.e. the source of any link found)
			std::vector<ObjectLink>&	mObjectLinks;	// Array of all links found
		};


		/**
		 * FileLinkVisitor is a functor-like object that can be used as an argument to VisitRTTIProperties and collects all file links
		 */
		struct FileLinkVisitor
		{
		public:
			FileLinkVisitor(std::vector<std::string>& fileLinks) :
				mFileLinks(fileLinks)
			{
			}

			void operator()(const Instance& instance, const Property& property, const Variant& value, const RTTIPath& path)
			{
				if (!rtti::hasFlag(property, rtti::EPropertyMetaData::FileLink))
					return;

				Variant actual_value = value.get_type().is_wrapper() ? value.extract_wrapped_value() : value;

				assert(actual_value.get_type().is_derived_from<std::string>());
				mFileLinks.push_back(actual_value.convert<std::string>());
			}

		private:
			std::vector<std::string>&	mFileLinks;	// Array of file links found
		};


		/**
		 * Helper function to recursively check whether two variants (i.e. values) are equal
		 * Correctly deals with arrays and nested compounds, but note: does not follow pointers
		 */
		bool areVariantsEqualRecursive(const rtti::Variant& variantA, const rtti::Variant& variantB, EPointerComparisonMode pointerComparisonMode)
		{
			// Extract wrapped type
			auto value_type = variantA.get_type();
			auto actual_type = value_type.is_wrapper() ? value_type.get_wrapped_type() : value_type;
			bool is_wrapper = actual_type != value_type;

			// Types must match
			assert(value_type == variantB.get_type());

			// If this is an array, compare the array element-wise
			if (actual_type.is_array())
			{
				// Get the arrays
				rtti::VariantArray array_a = variantA.create_array_view();
				rtti::VariantArray array_b = variantB.create_array_view();

				// If the sizes don't match, the arrays can't be equal
				if (array_a.get_size() != array_b.get_size())
					return false;

				// Recursively compare each array element
				for (int index = 0; index < array_a.get_size(); ++index)
				{
					rtti::Variant array_value_a = array_a.get_value(index);
					rtti::Variant array_value_b = array_b.get_value(index);

					if (!areVariantsEqualRecursive(array_value_a, array_value_b, pointerComparisonMode))
						return false;
				}
			}
			else
			{
				// Special case handling for pointers so we can compare by ID or by actual pointer value
				if (actual_type.is_pointer())
				{
					// If we don't want to compare by ID, just check the pointers directly
					if (pointerComparisonMode == EPointerComparisonMode::BY_POINTER)
					{
						return is_wrapper ? (variantA.extract_wrapped_value() == variantB.extract_wrapped_value()) : (variantA == variantB);
					}
					else if (pointerComparisonMode == EPointerComparisonMode::BY_ID)
					{
						// Extract the pointer
						rtti::Variant value_a = is_wrapper ? variantA.extract_wrapped_value() : variantA;
						rtti::Variant value_b = is_wrapper ? variantB.extract_wrapped_value() : variantB;

						// Can only compare pointers that are of type Object
						assert(value_a.get_type().is_derived_from<rtti::RTTIObject>() && value_b.get_type().is_derived_from<rtti::RTTIObject>());

						// Extract the objects
						rtti::RTTIObject* object_a = value_a.convert<rtti::RTTIObject*>();
						rtti::RTTIObject* object_b = value_b.convert<rtti::RTTIObject*>();

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
				if (rtti::isPrimitive(actual_type) || child_properties.empty())
					return is_wrapper ? (variantA.extract_wrapped_value() == variantB.extract_wrapped_value()) : (variantA == variantB);

				// Recursively compare each property of the compound
				for (const rtti::Property& property : child_properties)
				{
					rtti::Variant value_a = property.get_value(variantA);
					rtti::Variant value_b = property.get_value(variantB);
					if (!areVariantsEqualRecursive(value_a, value_b, pointerComparisonMode))
						return false;
				}
			}

			return true;
		}


		/**
		* Copies rtti attributes from one object to another.
		*/
		void copyObject(const rtti::RTTIObject& srcObject, rtti::RTTIObject& dstObject)
		{
			rtti::TypeInfo type = srcObject.get_type();
			assert(type == dstObject.get_type());

			for (const rtti::Property& property : type.get_properties())
			{
				rtti::Variant new_value = property.get_value(srcObject);
				bool success = property.set_value(dstObject, new_value);
				assert(success);
			}
		}


		/**
		* Tests whether the attributes of two objects have the same values.
		* @param objectA: first object to compare attributes from.
		* @param objectB: second object to compare attributes from.
		*/
		bool areObjectsEqual(const rtti::RTTIObject& objectA, const rtti::RTTIObject& objectB, EPointerComparisonMode pointerComparisonMode)
		{
			rtti::TypeInfo typeA = objectA.get_type();
			assert(typeA == objectB.get_type());

			for (const rtti::Property& property : typeA.get_properties())
			{
				rtti::Variant valueA = property.get_value(objectA);
				rtti::Variant valueB = property.get_value(objectB);
				if (!areVariantsEqualRecursive(valueA, valueB, pointerComparisonMode))
					return false;
			}

			return true;
		}


		/**
		* Searches through object's rtti attributes for attribute that have the 'file link' tag.
		*/
		void findFileLinks(const rtti::RTTIObject& object, std::vector<std::string>& fileLinks)
		{
			fileLinks.clear();

			RTTIPath path;
			FileLinkVisitor visitor(fileLinks);
			VisitRTTIProperties(object, path, visitor);
		}


		/**
		* Searches through object's rtti attributes for pointer attributes.
		*/
		void findObjectLinks(const rtti::RTTIObject& object, std::vector<ObjectLink>& objectLinks)
		{
			objectLinks.clear();

			RTTIPath path;
			ObjectLinkVisitor visitor(object, objectLinks);
			VisitRTTIProperties(object, path, visitor);
		}


		/**
		 * Helper function to recursively build a type version string for a given RTTI type
		 */
		void appendTypeInfoToVersionStringRecursive(const rtti::TypeInfo& type, std::string& versionString)
		{
			// Append name of type
			versionString.append(type.get_name().data(), type.get_name().size());

			// Append properties
			for (const rtti::Property& property : type.get_properties())
			{
				// Append property name + type
				versionString.append(property.get_name().data(), property.get_name().size());
				versionString.append(property.get_type().get_name().data(), property.get_type().get_name().size());

				rtti::TypeInfo type = property.get_type();

				// TODO: array/map support
	// 			if (type.is_array())
	// 			{
	// 				if (type.can_create_instance())
	// 				{
	// 					rtti::Variant array_inst = type.create();
	// 					rtti::VariantArray array_view = array_inst.create_array_view();
	// 					type = array_view.get_rank_type(array_view.get_rank());
	// 				}
	// 			}				

				// Don't recurse into primitives, pointers or types without further properties
				if (rtti::isPrimitive(type) || type.is_pointer() || type.get_properties().empty())
					continue;

				// Recurse
				appendTypeInfoToVersionStringRecursive(type, versionString);
			}
		}


		/**
		 * Calculate the version number of the specified type
		 */
		std::size_t getRTTIVersion(const rtti::TypeInfo& type)
		{
			// Build the version string first
			std::string version_string;
			appendTypeInfoToVersionStringRecursive(type, version_string);

			// Hash
			std::size_t hash = std::hash<std::string>()(version_string);
			return hash;
		}
	}
}
