#include <rtti/rttiwriter.h>
#include <nap/object.h>
#include "nap/errorstate.h"

namespace nap
{
	bool serializeObjectRecursive(const RTTI::Instance object, ObjectList& objectsToSerialize, RTTIWriter& writer, ErrorState& errorState);


	/**
	 * Helper function to add the specified object to the list of objects that need to be serialized
	 */
	void addObjectToSerialize(ObjectList& objectsToSerialize, Object* object)
	{
		// If the object is already in the list, ignore it
		if (std::find(objectsToSerialize.begin(), objectsToSerialize.end(), object) != objectsToSerialize.end())
			return;

		objectsToSerialize.push_back(object);
	}


	/**
	 * Helper function to serialize an array
	 */
	bool serializeArray(const RTTI::VariantArray& array, ObjectList& objectsToSerialize, RTTIWriter& writer, ErrorState& errorState)
	{
		// Write the start of the array
		if (!errorState.check(writer.startArray(array.get_size()), "Failed write start of array"))
			return false;

		// Write the elements
		for (int i = 0; i < array.get_size(); ++i)
		{
			RTTI::Variant var = array.get_value_as_ref(i);

			// If the value is an array, recurse
			if (var.is_array())
			{
				if (!serializeArray(var.create_array_view(), objectsToSerialize, writer, errorState))
					return false;
			}
			else
			{
				// Get the actual value of the element
				RTTI::Variant wrapped_var = var.extract_wrapped_value();
				RTTI::TypeInfo value_type = wrapped_var.get_type();

				// Write pointer value
				if (value_type.is_pointer())
				{
					// Pointers must point to Objects
					if (!errorState.check(value_type.is_derived_from<nap::Object>(), "Encountered pointer to non-Object"))
						return false;

					// Get the object being pointed to
					Object* pointee = wrapped_var.convert<Object*>();
					if (pointee != nullptr)
					{
						// Objects we point to must also be serialized, so add it to the set of objects to be serialized
						addObjectToSerialize(objectsToSerialize, pointee);

						// Check that the ID of the pointer is not empty (we can't point to objects without an ID)
						const std::string& pointee_id = pointee->mID;
						if (!errorState.check(!pointee_id.empty(), "Encountered pointer to Object with invalid ID"))
							return false;

						// Write the pointer
						if (!errorState.check(writer.writePointer(pointee_id), "Failed to write pointer"))
							return false;
					}
					else
					{
						// Write null pointer
						if (!errorState.check(writer.writePointer(std::string()), "Failed to write pointer"))
							return false;
					}
				}
				else if (RTTI::isPrimitive(value_type))
				{
					// Write primitive type (float, int, string, etc)
					if (!errorState.check(writer.writePrimitive(value_type, wrapped_var), "Failed to write primitive"))
						return false;
				}
				else
				{
					// Write compound
					if (!errorState.check(writer.startCompound(value_type), "Failed to start nested compound"))
						return false;

					// Recurse into the compound
					if (!serializeObjectRecursive(wrapped_var, objectsToSerialize, writer, errorState))
						return false;

					// Finish the compound
					if (!errorState.check(writer.finishCompound(), "Failed to finish nested compound"))
						return false;
				}
			}
		}

		// Finish the array
		if (!errorState.check(writer.finishArray(), "Failed to finish array"))
			return false;

		return true;
	}


	/**
	 * Helper function to serialize a property
	 */
	bool serializeProperty(const RTTI::Property& property, const RTTI::Variant& value, ObjectList& objectsToSerialize, RTTIWriter& writer, ErrorState& errorState)
	{
		auto value_type = value.get_type();
		auto wrapped_type = value_type.is_wrapper() ? value_type.get_wrapped_type() : value_type;
		bool is_wrapper = wrapped_type != value_type;

		// Write property name
		if (!errorState.check(writer.writeProperty(property.get_name().data()), "Failed to write property name"))
			return false;

		// If this is an array, recurse
		if (value_type.is_array())
		{
			if (!serializeArray(value.create_array_view(), objectsToSerialize, writer, errorState))
				return false;

			return true;
		}
		else if (value_type.is_pointer())
		{
			// Pointers must be derived from Object
			if (!errorState.check(wrapped_type.is_derived_from<nap::Object>(), "Encountered pointer to non-Object"))
				return false;

			Object* pointee = value.convert<Object*>();
			if (pointee != nullptr)
			{
				// Objects we point to must also be serialized, so add it to the set of objects to be serialized
				addObjectToSerialize(objectsToSerialize, pointee);

				// Check that the ID of the pointer is not empty (we can't point to objects without an ID)
				const std::string& pointee_id = pointee->mID;
				if (!errorState.check(!pointee_id.empty(), "Encountered pointer to Object with invalid ID"))
					return false;

				// Write the pointer				
				if (!errorState.check(writer.writePointer(pointee_id), "Failed to write pointer"))
					return false;
				
				return true;
			}
			else
			{
				// Write null pointer
				if (!errorState.check(writer.writePointer(std::string()), "Failed to write null pointer"))
					return false;

				return true;
			}
		}
		else if (RTTI::isPrimitive(wrapped_type))
		{
			// Write primitive type (float, string, etc)
			if (!errorState.check(writer.writePrimitive(wrapped_type, is_wrapper ? value.extract_wrapped_value() : value), "Failed to write primitive"))
				return false;
			
			return true;
		}
		else
		{
			// Write nested compound
			auto child_props = is_wrapper ? wrapped_type.get_properties() : value_type.get_properties();
			if (!child_props.empty())
			{
				// Start compound
				if (!errorState.check(writer.startCompound(wrapped_type), "Failed to start nested compound"))
					return false;

				// Recurse into compound
				if (!serializeObjectRecursive(value, objectsToSerialize, writer, errorState))
					return false;

				// Finish compound
				if (!errorState.check(writer.finishCompound(), "Failed to finsih nested compound"))
					return false;

				return true;
			}
		}

		// Unknown type
		errorState.fail("Encountered unknown property type");
		return false;
	}


	/**
	 * Helper function to serialize a RTTI object (not just nap::Object)
	 */
	bool serializeObjectRecursive(const RTTI::Instance object, ObjectList& objectsToSerialize, RTTIWriter& writer, ErrorState& errorState)
	{
		// Determine the actual type of the object
		RTTI::Instance actual_object = object.get_type().get_raw_type().is_wrapper() ? object.get_wrapped_instance() : object;

		// Write all properties
		for (const RTTI::Property& prop : actual_object.get_derived_type().get_properties())
		{
			// Get the value of the property
			RTTI::Variant prop_value = prop.get_value(actual_object);
			assert(prop_value.is_valid());

			// Serialize the property
			if (!serializeProperty(prop, prop_value, objectsToSerialize, writer, errorState))
				return false;
		}

		return true;
	}


	bool serializeObjects(const ObjectList& rootObjects, RTTIWriter& writer, ErrorState& errorState)
	{
		// Copy the list of objects to write to an internal list that we can modify
		ObjectList objects_to_write = rootObjects;

		// Signal writer that we're starting
		if (!errorState.check(writer.start(), "Failed to start writing"))
			return false;

		// Go through the array of objects to write. Note that we keep querying the length of the array because objects can be added during traversal
		for (int index = 0; index < objects_to_write.size(); ++index)
		{
			Object* object = objects_to_write[index];

			if (!errorState.check(!object->mID.empty(), "Encountered object without ID. This is not allowed"))
				return false;

			// Write start of object
			if (!errorState.check(writer.startRootObject(object->get_type()), "Failed to start writing root object"))
				return false;

			// Recurse into object
			if (!serializeObjectRecursive(object, objects_to_write, writer, errorState))
				return false;

			// Finish object
			if (!errorState.check(writer.finishRootObject(), "Failed to finish writing root object"))
				return false;
		}

		// Signal writer that we're done
		if (!errorState.check(writer.finish(), "Failed to finish writing"))
			return false;

		return true;
	}
}