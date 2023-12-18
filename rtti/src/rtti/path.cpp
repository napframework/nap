/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "path.h"
#include "object.h"

// External Includes
#include <utility/stringutils.h>

namespace nap
{
	namespace rtti
	{
		const std::string Path::toString() const
		{
			std::string result;
			for (int index = 0; index < mLength; ++index)
			{
				const PathElement& element = mElements[index];
				switch (element.mType)
				{
					case PathElement::Type::ATTRIBUTE:
					{
						result += result.empty() ?
							element.Attribute.Name :
							utility::stringFormat("/%s", element.Attribute.Name);
						break;
					}
					case PathElement::Type::ARRAY_ELEMENT:
					{
						result += result.empty() ?
							utility::stringFormat("%d", element.ArrayElement.Index) :
							utility::stringFormat("/%d", element.ArrayElement.Index);
						break;
					}
					default:
					{
						assert(false);
						break;
					}
				}
			}

			return result;
		}


		const Path Path::fromString(const std::string& path)
		{
			// Split string on path separator
			std::list<std::string> parts;
			utility::tokenize(path, parts, "/", true);

			Path result;
			for (const std::string& part : parts)
			{
				// Try to extract array index
				int array_index = 0;
				if (sscanf(part.c_str(), "%d", &array_index) == 1)
				{
					result.pushArrayElement(array_index);
				}
				else
				{
					// Failed to extract array index; must be an attribute
					result.pushAttribute(part);
				}
			}
			return result;
		}


		bool Path::resolve(const rtti::Object* object, ResolvedPath& resolvedPath) const
		{
			// Can't resolve an empty path
			if (mLength == 0)
				return false;

			for (int index = 0; index < mLength; ++index)
			{
				const PathElement& element = mElements[index];

				// Handle attribute
				if (element.mType == PathElement::Type::ATTRIBUTE)
				{
					// If this is the first element on the path, we need to push a 'root' element.
					// The root element is identical to the attribute element, with the difference that the root
					// element does not make a copy of the object that the property is on (the attribute element does)
					if (index == 0)
					{
						// See if the object contains a property with this name. If not, it means the path is invalid
						rtti::Property property = object->get_type().get_property(element.Attribute.Name);
						if (!property.is_valid())
							return false;

						// Push root
						resolvedPath.pushRoot(object, property);
					}
					else
					{
						// Retrieve the current value of the resolved path.
						// If there is none, the path is invalid
						// (we're trying to push a nested attribute on an empty path)
						const rtti::Variant& current_context = resolvedPath.getValue();
						if (!current_context.is_valid())
							return false;

						// See if the object that's currently on the resolved path has a property with this name.
						// If not, it means the path is invalid
						rtti::Property property = current_context.get_type().get_property(element.Attribute.Name);
						if (!property.is_valid())
							return false;

						// Push attribute
						resolvedPath.pushAttribute(current_context, property);
					}
				}
				else if (element.mType == PathElement::Type::ARRAY_ELEMENT)
				{
					// Retrieve the current value of the resolved path. If there is none,
					// the path is invalid (we're trying to push a nested attribute on an empty path)
					const rtti::Variant& current_context = resolvedPath.getValue();
					if (!current_context.is_valid())
						return false;

					// Since we're pushing an array element, the previous element must be an array.
					// If not, the path is invalid
					if (!current_context.is_array())
						return false;

					// Push array element
					resolvedPath.pushArrayElement(current_context, element.ArrayElement.Index);
				}
			}

			return true;
		}


		/**
		 * Note that while this function gets the value of the property currently on the path,
		 * it does so by returning a *copy*. See setValue for more information
		 */
		const rtti::Variant ResolvedPath::getValue() const
		{
			// If empty, we can't get the value
			if (isEmpty())
				return rtti::Variant();

			const ResolvedRTTIPathElement& last_element = mElements[mLength - 1];

			// If this is a root element, get the value of the property on the root instance
			if (last_element.mType == ResolvedRTTIPathElement::Type::ROOT)
			{
				return last_element.Root.Property.get_value(last_element.Root.Instance);
			}
			else if (last_element.mType == ResolvedRTTIPathElement::Type::ATTRIBUTE)
			{
				// If this is an attribute element, get the value of the property on the attribute object
				return last_element.Attribute.Property.get_value(last_element.Attribute.Variant);
			}
			else if (last_element.mType == ResolvedRTTIPathElement::Type::ARRAY_ELEMENT)
			{
				// If this is an array element, get the value of the array at the desired index
				rtti::VariantArray array = last_element.ArrayElement.Array.create_array_view();
				return array.get_value(last_element.ArrayElement.Index);
			}

			// Unknown type
			assert(false);
			return rtti::Variant();
		}


		/**
		 * There are some important subtleties to the resolving of an RTTIPath.
		 * The RTTI system we use does not allow you to retrieve a direct pointer to a property of an object.
		 * This means that whenever you get a value,
		 * you are getting a *copy* of the value actually stored in the property.
		 * The implication of this is that you can never directly set the property of an object in a nested hierarchy.
		 * For example, consider this path (see RTTIPath documentation):
		 *
		 *		RTTIPath path;
		 *
		 *		// Push name of the RTTI property on SomeRTTIClass
		 *		path.PushAttribute("ArrayOfCompounds");
		 *
		 *		// Push index into the array 'ArrayOfCompounds' on SomeRTTIClass
		 *		path.PushArrayElement(0);
		 *
		 *		// Push name of the RTTI property on the compound contained in ArrayOfCompounds, namely 'DataStruct'
		 *		path.PushAttribute("PointerProperty");
		 *
		 * If we want to set the value of the pointer property, we need to do a couple of things:
		 * - First, we set the value of the pointer property on a *copy* of the object (DataStruct)
		 *   that is actually stored in the array
		 * - Then, we copy the object (DataStruct) to a *copy* of the array that is actually stored in
		 *   the ArrayOfCompounds property
		 * - Finally, we copy the entire array back to the actual array on the root element
		 *
		 * In essence, setting a value is a recursive function where the value to set is recursively copied up
		 * the path from the bottom
		 */
		bool ResolvedPath::setValue(const rtti::Variant& value)
		{
			// Empty path, can't set value
			if (isEmpty())
				return false;

			rtti::TypeInfo value_type = getType();
			value_type = value_type.is_wrapper() ? value_type.get_wrapped_type() : value_type;

			// We keep track of the value we want to set on the current element of the path.
			// We start with the value the user provided and backtrack from there.
			// We explicitly check and set a nullptr -> nullptr variant conversion not working or available(?).
			rtti::Variant value_to_set = value;
			for (int index = mLength - 1; index >= 0; --index)
			{
				// If this is the root element, directly set the value on the object
				const ResolvedRTTIPathElement& element = mElements[index];
				if (element.mType == ResolvedRTTIPathElement::Type::ROOT)
				{
					if (value_type.is_pointer() && value_to_set == nullptr)
					{
						if (!element.Root.Property.set_value(element.Root.Instance, nullptr))
							return false;
					}
					else
					{
						if (!element.Root.Property.set_value(element.Root.Instance, value_to_set))
							return false;
					}
				}
				// Attribute element: set the value on the *copy* of the object (the variant)
				else if (element.mType == ResolvedRTTIPathElement::Type::ATTRIBUTE)
				{
					if (value_type.is_pointer() && value_to_set == nullptr)
					{
						if (!element.Attribute.Property.set_value(element.Attribute.Variant, nullptr))
							return false;
					}
					else
					{
						// Attribute element: set the value on the *copy* of the object (the variant)
						if (!element.Attribute.Property.set_value(element.Attribute.Variant, value_to_set))
							return false;
					}

					// Now that we've updated our copy, we need to copy our copy to the original object.
					// So we update value_to_set for the next iteration
					value_to_set = element.Attribute.Variant;
				}
				else if (element.mType == ResolvedRTTIPathElement::Type::ARRAY_ELEMENT)
				{
					// Array element: set the array index value on a *copy* of the array
					rtti::VariantArray array = element.ArrayElement.Array.create_array_view();
					if (!array.set_value(element.ArrayElement.Index, value_to_set))
						return false;

					// Now that we've updated our copy of the array,
					// we need to copy our array copy to the original object.
					// So we update value_to_set for the next iteration
					value_to_set = element.ArrayElement.Array;
				}
			}

			return true;
		}


		const rtti::TypeInfo ResolvedPath::getType() const
		{
			return getValue().get_type();
		}

		
		const rtti::Property& ResolvedPath::getProperty() const
		{
			const rtti::Property* property = nullptr;
			for (int index = mLength - 1; index >= 0; --index)
			{
				if (mElements[index].mType == ResolvedRTTIPathElement::Type::ATTRIBUTE)
				{
					property = &mElements[index].Attribute.Property;
				}
				else if (mElements[index].mType == ResolvedRTTIPathElement::Type::ROOT)
				{
					property = &mElements[index].Root.Property;
				}

				if (property != nullptr)
					break;
			}

			assert(property != nullptr);
			return *property;
		}
	}
}
