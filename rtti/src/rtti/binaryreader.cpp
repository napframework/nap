/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "binaryreader.h"
#include "rttibinaryversion.h"
#include "factory.h"
#include "object.h"

// External Includes
#include <utility/errorstate.h>
#include <fstream>
#include "rttiutilities.h"

namespace nap
{
	namespace rtti
	{
		struct PropertyMetaData
		{
			bool mIsRequired;
			bool mIsFileLink;
		};

		using PropertyMetaDataList = std::vector<PropertyMetaData>;

		const PropertyMetaDataList getAllPropertyMetaData(const rtti::TypeInfo& type)
		{
			PropertyMetaDataList result;

			// Go through all properties of the object
			const auto& range = type.get_properties();
			result.resize(range.size());

			int index = 0;
			for (const rtti::Property& property : range)
			{
				PropertyMetaData& metadata = result[index++];

				// Determine meta-data for the property
				metadata.mIsRequired = rtti::hasFlag(property, nap::rtti::EPropertyMetaData::Required);
				metadata.mIsFileLink = rtti::hasFlag(property, nap::rtti::EPropertyMetaData::FileLink);
			}

			return result;
		}

		static bool deserializePropertiesRecursive(rtti::Object* object, rtti::Instance compound, const PropertyMetaDataList& compoundPropertyMetaData, utility::MemoryStream& stream, rtti::Path& rttiPath, 
			UnresolvedPointerList& unresolvedPointers, std::vector<FileLink>& linkedFiles,  utility::ErrorState& errorState);

		/**
		 * Helper function to check if the specified stream starts with the RTTI binary version header
		 */
		bool readAndCheckRTTIBinaryVersion(utility::MemoryStream& stream)
		{
			// Determine length of header
			int len = strlen(gRTTIBinaryVersion);
			assert(len < 64);

			// Read header from stream
			char header[64] = { 0 };
			stream.read(header, len);

			// Check binary version
			return strcmp(gRTTIBinaryVersion, header) == 0;
		}


		/**
		 * Helper function to check if the specified stream starts with the RTTI binary version header
		 */
		bool readAndCheckTypeVersions(utility::MemoryStream& stream)
		{
			// Check type versions
			size_t num_types = stream.read<size_t>();
			for (int index = 0; index < num_types; ++index)
			{
				// Read typename
				std::string type_name;
				stream.readString(type_name);

				// Read saved version
				uint64_t version = stream.read<uint64_t>();

				// Find type; if it's not found, the type has been removed and we can't deserialize this file
				rtti::TypeInfo type = rtti::TypeInfo::get_by_name(type_name);
				if (!type.is_valid())
					return false;

				// Check if version on disk matches current version
				if (version != getRTTIVersion(type))
					return false;
			}

			return true;
		}


		/**
		 * Helper function to read a basic JSON type to a C++ type
		 */
		static rtti::Variant deserializePrimitive(const rtti::TypeInfo& type, utility::MemoryStream& stream)
		{
			if (type.is_arithmetic())
			{
				if (type == rtti::TypeInfo::get<bool>())
					return stream.read<bool>();
				else if (type == rtti::TypeInfo::get<char>())
					return stream.read<uint8_t>();
				else if (type == rtti::TypeInfo::get<int8_t>())
					return stream.read<int8_t>();
				else if (type == rtti::TypeInfo::get<int16_t>())
					return stream.read<int16_t>();
				else if (type == rtti::TypeInfo::get<int32_t>())
					return stream.read<int32_t>();
				else if (type == rtti::TypeInfo::get<int64_t>())
					return stream.read<int64_t>();
				else if (type == rtti::TypeInfo::get<uint8_t>())
					return stream.read<uint8_t>();
				else if (type == rtti::TypeInfo::get<uint16_t>())
					return stream.read<uint16_t>();
				else if (type == rtti::TypeInfo::get<uint32_t>())
					return stream.read<uint32_t>();
				else if (type == rtti::TypeInfo::get<uint64_t>())
					return stream.read<uint64_t>();
				else if (type == rtti::TypeInfo::get<float>())
					return stream.read<float>();
				else if (type == rtti::TypeInfo::get<double>())
					return stream.read<double>();
			}
			else if (type.is_enumeration())
			{
				return stream.read<uint64_t>();
			}
			else if (type == rtti::TypeInfo::get<std::string>())
			{
				std::string str;
				stream.readString(str);
				return str;
			}

			// Unknown type
			assert(false);
			return rtti::Variant();
		}


		/**
		 * Helper function to recursively read an array (can be an array of basic types, nested compound, or any other type) from JSON
		 */
		static bool deserializeArrayRecursively(rtti::Object* rootObject, rtti::VariantArray& array, utility::MemoryStream& stream, rtti::Path& rttiPath,
			UnresolvedPointerList& unresolvedPointers, std::vector<FileLink>& linkedFiles, utility::ErrorState& errorState)
		{
			uint32_t length;
			stream.read(length);

			// Pre-size the array to avoid too many dynamic allocs
			array.set_size(length);

			// Determine the rank of the array (i.e. how many dimensions it has)
			const rtti::TypeInfo array_type = array.get_rank_type(array.get_rank());
			const rtti::TypeInfo wrapped_type = array_type.is_wrapper() ? array_type.get_wrapped_type() : array_type;

			// Read values from JSON array
			const PropertyMetaDataList& wrapped_var_metadata = getAllPropertyMetaData(wrapped_type);
			for (std::size_t index = 0; index < length; ++index)
			{
				// Add array element to rtti path
				rttiPath.pushArrayElement(index);

				if (wrapped_type.is_array())
				{
					// Array-of-arrays; read array recursively
					rtti::VariantArray sub_array = array.get_value_as_ref(index).create_array_view();
					if (!deserializeArrayRecursively(rootObject, sub_array, stream, rttiPath, unresolvedPointers, linkedFiles, errorState))
						return false;
				}
				else if (wrapped_type.is_associative_container())
				{
					// Maps not supported (yet)
					errorState.fail("Encountered currently unsupported associative property");
					return false;
				}
				else if (wrapped_type.is_pointer())
				{
					// Pointer types must point to objects derived from rtti::RTTIObject
					if (!errorState.check(wrapped_type.get_raw_type().is_derived_from<rtti::Object>(), "Encountered pointer to non-Object. This is not supported"))
						return false;

					// Determine the target of the pointer
					std::string target;
					stream.readString(target);

					// Add to list of unresolved pointers
					if (!target.empty())
						unresolvedPointers.push_back(UnresolvedPointer(rootObject, rttiPath, target));
				}
				else if (rtti::isPrimitive(wrapped_type))
				{
					// Array of basic types; read basic type
					rtti::Variant extracted_value = deserializePrimitive(wrapped_type, stream);
					if (extracted_value.convert(wrapped_type))
						array.set_value(index, extracted_value);
				}
				else
				{
					// Array-of-compounds; read object recursively
					rtti::Variant var_tmp = array.get_value_as_ref(index);
					rtti::Variant wrapped_var = var_tmp.extract_wrapped_value();
					if (!deserializePropertiesRecursive(rootObject, wrapped_var, wrapped_var_metadata, stream, rttiPath, unresolvedPointers, linkedFiles, errorState))
						return false;

					array.set_value(index, wrapped_var);
				}

				// Remove array element from rtti path again
				rttiPath.popBack();
			}

			return true;
		}


		/**
		 * Helper function to recursively read an object (can be a rtti::RTTIObject, nested compound or any other type) from JSON
		 */
		static bool deserializePropertiesRecursive(rtti::Object* object, rtti::Instance compound, const PropertyMetaDataList& compoundPropertyMetaData, utility::MemoryStream& stream, 
			rtti::Path& rttiPath, UnresolvedPointerList& unresolvedPointers, std::vector<FileLink>& linkedFiles, utility::ErrorState& errorState)
		{
			// Determine the object type. Note that we want to *most derived type* of the object.
			rtti::TypeInfo object_type = compound.get_derived_type();

			// Go through all properties of the object
			int property_index = 0;
			for (const rtti::Property& property : object_type.get_properties())
			{
				// Push attribute on path
				rttiPath.pushAttribute(property.get_name().data());

				// Determine meta-data for the property
				const PropertyMetaData& metadata = compoundPropertyMetaData[property_index++];

				const rtti::TypeInfo value_type = property.get_type();
				const rtti::TypeInfo wrapped_type = value_type.is_wrapper() ? value_type.get_wrapped_type() : value_type;

				// If this is a file link, make sure it's of the expected type (string)
				if (!errorState.check((metadata.mIsFileLink && wrapped_type.get_raw_type().is_derived_from<std::string>()) || !metadata.mIsFileLink, "Encountered a non-string file link. This is not supported"))
					return false;

				// If this is an array, read its elements recursively again (in case of nested compounds)
				if (wrapped_type.is_array())
				{
					// Get instance of the current value (this is a copy) and create an array view on it so we can fill it
					rtti::Variant value = property.get_value(compound);
					rtti::VariantArray array_view = value.create_array_view();

					// Now read the array recursively into array view
					if (!deserializeArrayRecursively(object, array_view, stream, rttiPath, unresolvedPointers, linkedFiles, errorState))
						return false;

					// Now copy the read array back into the target object
					property.set_value(compound, value);
				}
				else if (wrapped_type.is_associative_container())
				{
					// Maps not supported (yet)
					errorState.fail("Encountered currently unsupported associative property %s", property.get_type().get_name().data());
					return false;
				}
				else if (wrapped_type.is_pointer())
				{
					// Pointer types must point to objects derived from rtti::RTTIObject
					if (!errorState.check(wrapped_type.get_raw_type().is_derived_from<rtti::Object>(), "Encountered pointer to non-Object. This is not supported"))
						return false;

					// Determine the target of the pointer
					std::string target;
					stream.readString(target);

					// If the target is empty (i.e. null pointer), but the property is required, throw an error
					if (!errorState.check((metadata.mIsRequired && !target.empty()) || (!metadata.mIsRequired), "Required property '%s' not found in object '%s' of type %s", property.get_name().data(), object->mID.c_str(), object_type.get_name().data()))
						return false;

					// Add to list of unresolved pointers
					if (!target.empty())
						unresolvedPointers.push_back(UnresolvedPointer(object, rttiPath, target));
				}
				else if (rtti::isPrimitive(wrapped_type))
				{
					// Basic JSON type, read value and copy to target
					rtti::Variant extracted_value = deserializePrimitive(wrapped_type, stream);
					if (extracted_value.convert(wrapped_type))
						property.set_value(compound, extracted_value);
				}
				else
				{
					// If the property is a nested compound, read it recursively
					rtti::Variant var = property.get_value(compound);
					if (!deserializePropertiesRecursive(object, var, getAllPropertyMetaData(var.get_type()), stream, rttiPath, unresolvedPointers, linkedFiles, errorState))
						return false;

					// Copy read object back into the target object
					property.set_value(compound, var);
				}

				// If this property is a file link, add it to the list of file links
				if (metadata.mIsFileLink)
				{
					FileLink file_link;
					file_link.mTargetFile = property.get_value(compound).get_value<std::string>();;
					linkedFiles.push_back(file_link);
				}

				rttiPath.popBack();
			}

			return true;
		}

		bool checkBinaryVersion(const std::string& path)
		{
			// Open the file
			std::ifstream file(path, std::ios::in | std::ios::binary);
			if (!file.good())
				return false;

			// Read and check binary version
			{
				// Determine length of header
				int len = strlen(gRTTIBinaryVersion);
				assert(len < 64);

				char header[64] = { 0 };
				file.read(header, len);
				if (!file.good())
					return false;

				utility::MemoryStream header_stream((uint8_t*)header, len);
				if (!readAndCheckRTTIBinaryVersion(header_stream))
					return false;
			}

			// Read and check type version table
			{
				size_t version_table_size = 0;
				file.read((char*)&version_table_size, sizeof(version_table_size));

				std::vector<uint8_t> version_table_data;
				version_table_data.resize(version_table_size);

				file.read((char*)version_table_data.data(), version_table_data.size());
				if (!file.good())
					return false;

				utility::MemoryStream version_table_stream(version_table_data.data(), version_table_data.size());
				if (!readAndCheckTypeVersions(version_table_stream))
					return false;
			}

			return true;
		}

		bool deserializeBinary(utility::MemoryStream& stream, Factory& factory, DeserializeResult& result, utility::ErrorState& errorState)
		{
			if (!errorState.check(!stream.isDone(), "Can't deserialize from empty stream"))
				return false;

			if (!errorState.check(readAndCheckRTTIBinaryVersion(stream), "Can't deserialize binary; RTTIBinaryVersion mismatch"))
				return false;
			
            // We need to read the version table size here, even if we don't use it to make sure we can correctly read the type versions next
			stream.read<size_t>();
			if (!errorState.check(readAndCheckTypeVersions(stream), "Can't deserialize binary; version of a type contained in the binary has changed"))
				return false;

			// Continue reading while there's data in the stream
			while (!stream.isDone())
			{
				// Read type of the object
				std::string object_type;
				stream.readString(object_type);

				rtti::TypeInfo type_info = rtti::TypeInfo::get_by_name(object_type);
				if (!errorState.check(type_info.is_valid(), "Unknown object type %s encountered.", object_type.c_str()))
					return false;

				// Check whether this is a type that can actually be instantiated
				if (!errorState.check(factory.canCreate(type_info), "Unable to instantiate object of type %s.", object_type.c_str()))
					return false;

				// We only support root-level objects that derive from rtti::RTTIObject (compounds, etc can be of any type)
				if (!errorState.check(type_info.is_derived_from(RTTI_OF(rtti::Object)), "Unable to instantiate object %s. Class is not derived from Object.", object_type.c_str()))
					return false;

				// Check version
				uint64_t version = stream.read<uint64_t>();
				if (!errorState.check(version == rtti::getRTTIVersion(type_info), "Type %s found that does not match the expected version (perhaps the type has changed?). Re-export the binary to fix this issue.", object_type.c_str()))
					return false;

				// Create new instance of the object
				Object* object = factory.create(type_info);
				result.mReadObjects.push_back(std::unique_ptr<Object>(object));

				// Recursively read properties, nested compounds, etc
				rtti::Path path;
				if (!deserializePropertiesRecursive(object, *object, getAllPropertyMetaData(object->get_type()), stream, path, result.mUnresolvedPointers, result.mFileLinks, errorState))
					return false;
			}

			return true;
		}

		bool readBinary(const std::string& path, Factory& factory, DeserializeResult& result, utility::ErrorState& errorState)
		{
			// Open the file
			std::ifstream file(path, std::ios::in | std::ios::binary);
			if (!errorState.check(file.good(), "Unable to open file %s", path.c_str()))
				return false;

			// Create buffer of appropriate size
			file.seekg(0, std::ios::end);
			size_t len = file.tellg();
			std::vector<uint8_t> buffer;
			buffer.resize(len);

			// Read all data
			file.seekg(0, std::ios::beg);
			file.read((char*)buffer.data(), len);
			file.close();

			utility::MemoryStream stream(buffer.data(), buffer.size());
			if (!deserializeBinary(stream, factory, result, errorState))
				return false;

			return true;
		}
	}
}
      
