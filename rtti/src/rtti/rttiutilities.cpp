/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "rttiutilities.h"
#include "object.h"

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
		void VisitRTTIPropertiesRecursive(const rtti::Property& property, const Variant& variant, Path& path, FUNC& visitFunc)
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

					auto nested_property_type = nested_property.get_type().is_wrapper() ? nested_property.get_type().get_wrapped_type() : nested_property.get_type();

					// Invoke visit func
					visitFunc(variant, nested_property, value, path);					

					// Recurse
					if (!nested_property_type.is_pointer()) // Don't recurse into properties of pointers
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
		void VisitRTTIProperties(const Instance& instance, Path& path, FUNC& visitFunc)
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
			ObjectLinkVisitor(const rtti::Object& sourceObject, std::vector<ObjectLink>& objectLinks) :
				mSourceObject(sourceObject),
				mObjectLinks(objectLinks)
			{
			}

			void operator()(const Instance& instance, const Property& property, const Variant& value, const Path& path)
			{
				auto actual_type = value.get_type().is_wrapper() ? value.get_type().get_wrapped_type() : value.get_type();
				if (!actual_type.is_pointer())
					return;

				Variant actual_value = value.get_type().is_wrapper() ? value.extract_wrapped_value() : value;
				assert(actual_value.get_type().is_derived_from<rtti::Object>());

				// rttr::Variant::convert<T> fails and returns garbage if the current value of the pointer is a nullptr.
				// The non-templated convert function instead returns a bool to indicate succss/failure, so we use that one here
				Object* target = nullptr;
				actual_value.convert(target);

				mObjectLinks.push_back({ &mSourceObject, path, target });
			}

		private:
			const rtti::Object&		mSourceObject;	// The object we're visiting (i.e. the source of any link found)
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

			void operator()(const Instance& instance, const Property& property, const Variant& value, const Path& path)
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
		bool areVariantsEqualRecursive(const Object* unresolvedPointerRootObject, const rtti::Variant& variantA, const rtti::Variant& variantB, Path& currentRTTIPath, const rtti::UnresolvedPointerList& unresolvedPointers)
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
					currentRTTIPath.pushArrayElement(index);

					rtti::Variant array_value_a = array_a.get_value(index);
					rtti::Variant array_value_b = array_b.get_value(index);

					if (!areVariantsEqualRecursive(unresolvedPointerRootObject, array_value_a, array_value_b, currentRTTIPath, unresolvedPointers))
						return false;

					currentRTTIPath.popBack();
				}
			}
			else
			{
				// Special case handling for pointers so we can compare by ID or by actual pointer value
				if (actual_type.is_pointer())
				{
					std::string target_a_id;
					std::string target_b_id;

					// Extract the pointer
					rtti::Variant value_a = is_wrapper ? variantA.extract_wrapped_value() : variantA;
					rtti::Variant value_b = is_wrapper ? variantB.extract_wrapped_value() : variantB;

					// Can only compare pointers that are of type Object
					assert(value_a.get_type().is_derived_from<rtti::Object>() && value_b.get_type().is_derived_from<rtti::Object>());

					// Extract the objects
					rtti::Object* object_a = value_a.get_value<rtti::Object*>();
					if (object_a == nullptr)
					{
						int unresolved_pointer_index = findUnresolvedPointer(unresolvedPointers, unresolvedPointerRootObject, currentRTTIPath);
						if (unresolved_pointer_index != -1)
							target_a_id = unresolvedPointers[unresolved_pointer_index].getResourceTargetID();
					}
					else
					{
						target_a_id = object_a->mID;
					}

					rtti::Object* object_b = value_b.get_value<rtti::Object*>();
					if (object_b != nullptr)
						target_b_id = object_b->mID;

					// Check whether the IDs match
					return target_a_id == target_b_id;
				}

				// If the type of this variant is a primitive type or non-primitive type with no RTTI properties,
				// we perform a normal comparison
				auto child_properties = actual_type.get_properties();
				if (rtti::isPrimitive(actual_type) || child_properties.empty())
					return is_wrapper ? (variantA.extract_wrapped_value() == variantB.extract_wrapped_value()) : (variantA == variantB);

				// Recursively compare each property of the compound
				for (const rtti::Property& property : child_properties)
				{
					currentRTTIPath.pushAttribute(property.get_name().data());

					rtti::Variant value_a = property.get_value(variantA);
					rtti::Variant value_b = property.get_value(variantB);
					if (!areVariantsEqualRecursive(unresolvedPointerRootObject, value_a, value_b, currentRTTIPath, unresolvedPointers))
						return false;

					currentRTTIPath.popBack();
				}
			}

			return true;
		}


		/**
		* Copies rtti attributes from one object to another.
		*/
		void copyObject(const rtti::Object& srcObject, rtti::Object& dstObject)
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
		 * Tests whether the attributes of two objects have the same values. This is a special version that takes the unresolved pointer list
		 * and compares any unresolved pointers against IDs present in that list.
		 * @param objectA: first object to compare attributes from.
		 * @param objectB: second object to compare attributes from.
		 * @param unresolvedPointers: list of unresolved pointers as returned from readJSONFile.
		 */
		bool areObjectsEqual(const rtti::Object& objectA, const rtti::Object& objectB, const rtti::UnresolvedPointerList& unresolvedPointers)
		{
			rtti::TypeInfo typeA = objectA.get_type();
			assert(typeA == objectB.get_type());

			Path path;
			for (const rtti::Property& property : typeA.get_properties())
			{
				path.pushAttribute(property.get_name().data());

				rtti::Variant valueA = property.get_value(objectA);
				rtti::Variant valueB = property.get_value(objectB);
				if (!areVariantsEqualRecursive(&objectA, valueA, valueB, path, unresolvedPointers))
					return false;

				path.popBack();
			}

			return true;
		}


		/**
		* Searches through object's rtti attributes for attribute that have the 'file link' tag.
		*/
		void findFileLinks(const rtti::Object& object, std::vector<std::string>& fileLinks)
		{
			fileLinks.clear();

			Path path;
			FileLinkVisitor visitor(fileLinks);
			VisitRTTIProperties(object, path, visitor);
		}


		/**
		* Searches through object's rtti attributes for pointer attributes.
		*/
		void findObjectLinks(const rtti::Object& object, std::vector<ObjectLink>& objectLinks)
		{
			objectLinks.clear();

			Path path;
			ObjectLinkVisitor visitor(object, objectLinks);
			VisitRTTIProperties(object, path, visitor);
		}


		void getPointeesRecursive(const rtti::Object& object, std::vector<rtti::Object*>& pointees)
		{
			std::unordered_set<const rtti::Object*> objects_to_visit_set;
			std::vector<const rtti::Object*> objects_to_visit;

			objects_to_visit_set.insert(&object);
			objects_to_visit.push_back(&object);

			std::vector<ObjectLink> object_links;
			for (int index = 0; index < objects_to_visit.size(); ++index)
			{
				findObjectLinks(*objects_to_visit[index], object_links);

				for (const ObjectLink& link : object_links)
				{
					// Check if we already processed this pointer. We don't return nullptrs.
					if (link.mTarget != nullptr && objects_to_visit_set.find(link.mTarget) == objects_to_visit_set.end())
					{
						objects_to_visit_set.insert(link.mTarget);
						objects_to_visit.push_back(link.mTarget);
						pointees.push_back(link.mTarget);
					}
				}
			}
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
		uint64_t getRTTIVersion(const rtti::TypeInfo& type)
		{
			// Build the version string first
			std::string version_string;
			appendTypeInfoToVersionStringRecursive(type, version_string);

			// Hash
			std::size_t hash = std::hash<std::string>()(version_string);
			return (uint64_t)hash;
		}

		
		/**
		 * Helper to find the index of the unresolved pointer with the specified object and path combination
		 */
		int findUnresolvedPointer(const UnresolvedPointerList& unresolvedPointers, const Object* object, const rtti::Path& path)
		{
			for (int index = 0; index < unresolvedPointers.size(); ++index)
			{
				const UnresolvedPointer& unresolved_pointer = unresolvedPointers[index];
				if (unresolved_pointer.mObject == object && unresolved_pointer.mRTTIPath == path)
					return index;
			}

			return -1;
		}


		void getDerivedTypesRecursive(const rtti::TypeInfo& baseType, std::vector<rtti::TypeInfo>& types)
		{
			// Don't add same type more than once (eg. in case of multiple inheritance)
			if (std::find(types.begin(), types.end(), baseType) == types.end())
				types.push_back(baseType);

			for (const rtti::TypeInfo& derived_type : baseType.get_derived_classes())
				getDerivedTypesRecursive(derived_type, types);
		}


		bool hasDescription(const rtti::Property& property)
		{
			return property.get_metadata("description").is_valid();
		}


		const char* getDescription(const rtti::Property& property)
		{
			const rtti::Variant& meta_data = property.get_metadata("description");
			return meta_data.is_valid() ? meta_data.convert<const char*>() : nullptr;
		}


		bool hasDescription(const rtti::TypeInfo& type)
		{
			return getDescription(type) != nullptr;
		}


		const char* getDescription(const rtti::TypeInfo& type)
		{
			// Find description for given type
			auto range = type.get_methods(rttr::filter_item::static_item | rttr::filter_item::public_access);
			const rttr::method* description_method = nullptr;
			for (const auto& method : range)
			{
				if (method.get_name() == nap::rtti::method::description)
					description_method = &method;
			}

			// Invoke description when found
			if(description_method != nullptr)
				return description_method->invoke(rttr::instance()).convert<const char*>();

			// Not available -> search base types.
			// The `get_methods()` call 'should' include base types but it doesn't -> rttr bug?
			// Appears that base types are only included when the function is defined for every type in the chain!
			auto base_types = type.get_base_classes();
			return base_types.empty() ? nullptr : getDescription(*base_types.crbegin());
		}
	}
}
