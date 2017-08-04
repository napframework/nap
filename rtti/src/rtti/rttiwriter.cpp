#include <rtti/rttiwriter.h>
#include <rtti/rttiobject.h>
#include <cassert>

namespace nap
{
	namespace rtti
	{
		bool serializeObjectRecursive(const rtti::Instance object, ObjectList& objectsToSerialize, bool isEmbeddedObject, RTTIWriter& writer, utility::ErrorState& errorState);
		bool serializeValue(const rtti::Property& property, const rtti::Variant& variant, ObjectList& objectsToSerialize, RTTIWriter& writer, utility::ErrorState& errorState);


		/**
		 * Helper function to add the specified object to the list of objects that need to be serialized
		 */
		void addObjectToSerialize(ObjectList& objectsToSerialize, RTTIObject* object)
		{
			// If the object is already in the list, ignore it
			if (std::find(objectsToSerialize.begin(), objectsToSerialize.end(), object) != objectsToSerialize.end())
				return;

			objectsToSerialize.push_back(object);
		}


		/**
		 * Helper function to serialize an array
		 */
		bool serializeArray(const rtti::Property& property, const rtti::VariantArray& array, ObjectList& objectsToSerialize, RTTIWriter& writer, utility::ErrorState& errorState)
		{
			// Write the start of the array
			if (!errorState.check(writer.startArray(array.get_size()), "Failed write start of array"))
				return false;

			// Write the elements
			for (int i = 0; i < array.get_size(); ++i)
			{
				rtti::Variant var = array.get_value(i);

				// Write each value
				if (!serializeValue(property, var, objectsToSerialize, writer, errorState))
					return false;
			}

			// Finish the array
			if (!errorState.check(writer.finishArray(), "Failed to finish array"))
				return false;

			return true;
		}


		/**
		 * Helper function to serialize a property
		 */
		bool serializeProperty(const rtti::Property& property, const rtti::Variant& value, ObjectList& objectsToSerialize, RTTIWriter& writer, utility::ErrorState& errorState)
		{
			// Write property name
			if (!errorState.check(writer.writeProperty(property.get_name().data()), "Failed to write property name"))
				return false;

			if (!serializeValue(property, value, objectsToSerialize, writer, errorState))
				return false;

			return true;
		}


		/**
		 * Helper function to serialize a value
		*/
		bool serializeValue(const rtti::Property& property, const rtti::Variant& value, ObjectList& objectsToSerialize, RTTIWriter& writer, utility::ErrorState& errorState)
		{
			auto value_type = value.get_type();
			auto wrapped_type = value_type.is_wrapper() ? value_type.get_wrapped_type() : value_type;
			bool is_wrapper = wrapped_type != value_type;

			// If this is an array, recurse
			if (wrapped_type.is_array())
			{
				if (!serializeArray(property, value.create_array_view(), objectsToSerialize, writer, errorState))
					return false;

				return true;
			}
			else if (wrapped_type.is_associative_container())
			{
				errorState.fail("Associative containers are not supported");
				return false;
			}
			else if (wrapped_type.is_pointer())
			{
				// Pointers must be derived from Object
				if (!errorState.check(wrapped_type.is_derived_from<rtti::RTTIObject>(), "Encountered pointer to non-Object"))
					return false;

				RTTIObject* pointee = is_wrapper ? value.extract_wrapped_value().get_value<RTTIObject*>() : value.get_value<RTTIObject*>();
				if (pointee != nullptr)
				{
					bool is_embedded_pointer = rtti::hasFlag(property, nap::rtti::EPropertyMetaData::Embedded);
					if (is_embedded_pointer && writer.supportsEmbeddedPointers())
					{
						// Write start of nested object
						if (!errorState.check(writer.startCompound(pointee->get_type()), "Failed to start writing root object"))
							return false;

						// Write start of object
						if (!errorState.check(writer.startRootObject(pointee->get_type()), "Failed to start writing root object"))
							return false;

						// Recurse into object
						if (!serializeObjectRecursive(pointee, objectsToSerialize, true, writer, errorState))
							return false;

						// Finish object
						if (!errorState.check(writer.finishRootObject(), "Failed to finish writing root object"))
							return false;

						// Finish object
						if (!errorState.check(writer.finishCompound(), "Failed to finish writing root object"))
							return false;
					}
					else
					{
						// Check that the ID of the pointer is not empty (we can't point to objects without an ID)
						const std::string& pointee_id = pointee->mID;
						if (!errorState.check(!pointee_id.empty(), "Encountered pointer to Object with invalid ID"))
							return false;

						// Objects we point to must also be serialized, so add it to the set of objects to be serialized
						addObjectToSerialize(objectsToSerialize, pointee);

						// Write the pointer				
						if (!errorState.check(writer.writePointer(pointee_id), "Failed to write pointer"))
							return false;
					}

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
			else if (rtti::isPrimitive(wrapped_type))
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
					if (!serializeObjectRecursive(value, objectsToSerialize, false, writer, errorState))
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
		 * Helper function to serialize a RTTI object (not just rtti::RTTIObject)
		 */
		bool serializeObjectRecursive(const rtti::Instance object, ObjectList& objectsToSerialize, bool isEmbeddedObject, RTTIWriter& writer, utility::ErrorState& errorState)
		{
			// Determine the actual type of the object
			rtti::Instance actual_object = object.get_type().get_raw_type().is_wrapper() ? object.get_wrapped_instance() : object;

			// Write all properties
			for (const rtti::Property& property : actual_object.get_derived_type().get_properties())
			{
				// Don't write the ID for embedded objects (if the writer supports it)
				bool is_id = RTTIObject::isIDProperty(actual_object, property);
				if (is_id && isEmbeddedObject && writer.supportsEmbeddedPointers())
					continue;

				// Get the value of the property
				rtti::Variant prop_value = property.get_value(actual_object);
				assert(prop_value.is_valid());

				// Serialize the property
				if (!serializeProperty(property, prop_value, objectsToSerialize, writer, errorState))
					return false;
			}

			return true;
		}


		bool serializeObjects(const ObjectList& rootObjects, RTTIWriter& writer, utility::ErrorState& errorState)
		{
			// Copy the list of objects to write to an internal list that we can modify
			ObjectList objects_to_write = rootObjects;

			// Signal writer that we're starting
			if (!errorState.check(writer.start(), "Failed to start writing"))
				return false;

			// Go through the array of objects to write. Note that we keep querying the length of the array because objects can be added during traversal
			for (int index = 0; index < objects_to_write.size(); ++index)
			{
				RTTIObject* object = objects_to_write[index];

				if (!errorState.check(!object->mID.empty(), "Encountered object without ID. This is not allowed"))
					return false;

				// Write start of object
				if (!errorState.check(writer.startRootObject(object->get_type()), "Failed to start writing root object"))
					return false;

				// Recurse into object
				if (!serializeObjectRecursive(object, objects_to_write, false, writer, errorState))
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
}