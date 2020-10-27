/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "binarywriter.h"
#include "rttibinaryversion.h"
#include "rttiutilities.h"

namespace nap
{
	namespace rtti
	{
		void BinaryWriter::writeString(const std::string& string)
		{
			writeString(string.data(), string.length());
		}

		void BinaryWriter::writeString(const char* string, size_t length)
		{
			write(length);
			write(string, length);
		}

		size_t BinaryWriter::getPosition() const
		{
			return mBuffer.empty() ? 0 : mWritePointer - mBuffer.data();
		}

		void BinaryWriter::seek(size_t position)
		{
			assert(position <= mBuffer.size());
			mWritePointer = mBuffer.data() + position;
		}

		bool BinaryWriter::start(const ObjectList& rootObjects)
		{
			// Gather all used types
			std::unordered_set<rtti::TypeInfo> types;
			for (Object* object : rootObjects)
				types.insert(object->get_type());

			// Write binary version
			write(gRTTIBinaryVersion, strlen(gRTTIBinaryVersion));

			// Write dummy size, which we'll update later
			size_t version_size_position = getPosition();
			write<size_t>(0);

			size_t type_version_start = getPosition();

			// Now write type info for all types
			write(types.size());
			for (const rtti::TypeInfo& type : types)
			{
				writeString(type.get_name().data(), type.get_name().length());
				write(getRTTIVersion(type));
			}

			size_t cur_position = getPosition();			

			// Seek back to header start and write the actual size
			seek(version_size_position);
			write<size_t>(cur_position - type_version_start);
			
			// Seek back to actual write position so we can continue writing other data
			seek(cur_position);

			return true;
		}


		bool BinaryWriter::startRootObject(const rtti::TypeInfo& type)
		{
			writeString(type.get_name().data(), type.get_name().length());
			write(rtti::getRTTIVersion(type));
			return true;
		}


		bool BinaryWriter::startArray(int length)
		{
			write(length);
			return true;
		}


		bool BinaryWriter::writePointer(const std::string& pointeeID)
		{
			writeString(pointeeID);
			return true;
		}


		bool BinaryWriter::writePrimitive(const rtti::TypeInfo& type, const rtti::Variant& value)
		{
			if (type.is_arithmetic())
			{
				if (type == rtti::TypeInfo::get<bool>())
					write(value.to_bool());
				else if (type == rtti::TypeInfo::get<char>())
					write(value.to_uint8());
				else if (type == rtti::TypeInfo::get<int8_t>())
					write(value.to_int8());
				else if (type == rtti::TypeInfo::get<int16_t>())
					write(value.to_int16());
				else if (type == rtti::TypeInfo::get<int32_t>())
					write(value.to_int32());
				else if (type == rtti::TypeInfo::get<int64_t>())
					write(value.to_int64());
				else if (type == rtti::TypeInfo::get<uint8_t>())
					write(value.to_uint8());
				else if (type == rtti::TypeInfo::get<uint16_t>())
					write(value.to_uint16());
				else if (type == rtti::TypeInfo::get<uint32_t>())
					write(value.to_uint32());
				else if (type == rtti::TypeInfo::get<uint64_t>())
					write(value.to_uint64());
				else if (type == rtti::TypeInfo::get<float>())
					write(value.to_float());
				else if (type == rtti::TypeInfo::get<double>())
					write(value.to_double());
			}
			else if (type.is_enumeration())
			{
				// Try to convert the enum to uint64
				bool conversion_succeeded = false;
				uint64_t value_int = value.to_uint64(&conversion_succeeded);
				if (conversion_succeeded)
					write(value_int);
				else
					return false;
			}
			else if (type == rtti::TypeInfo::get<std::string>())
			{
				writeString(value.to_string());
			}

			return true;
		}


		void BinaryWriter::ensureHasRoom(uint32_t size)
		{
			uint32_t current_position = mBuffer.empty() ? 0 : mWritePointer - mBuffer.data();
			uint32_t available_room = mBuffer.size() - current_position;
			if (size > available_room)
			{
				mBuffer.resize(mBuffer.size() + (size - available_room));
				mWritePointer = mBuffer.data() + current_position;
			}
		}


		void BinaryWriter::write(const void* data, uint32_t numBytes)
		{
			ensureHasRoom(numBytes);
			std::memcpy(mWritePointer, data, numBytes);
			mWritePointer += numBytes;
		}
	}
}