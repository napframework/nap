#include "binarywriter.h"
#include "rttibinaryversion.h"
#include "rttiutilities.h"

namespace nap
{
	bool BinaryWriter::start()
	{
		write(gRTTIBinaryVersion, strlen(gRTTIBinaryVersion));
		return true;
	}

	bool BinaryWriter::startRootObject(const RTTI::TypeInfo& type)
	{
		writeString(type.get_name().data(), type.get_name().length());
		write(RTTI::getRTTIVersion(type));
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


	bool BinaryWriter::writePrimitive(const RTTI::TypeInfo& type, const RTTI::Variant& value)
	{
		if (type.is_arithmetic())
		{
			if (type == RTTI::TypeInfo::get<bool>())
				write(value.to_bool());
			else if (type == RTTI::TypeInfo::get<char>())
				write(value.to_uint8());
			else if (type == RTTI::TypeInfo::get<int8_t>())
				write(value.to_int8());
			else if (type == RTTI::TypeInfo::get<int16_t>())
				write(value.to_int16());
			else if (type == RTTI::TypeInfo::get<int32_t>())
				write(value.to_int32());
			else if (type == RTTI::TypeInfo::get<int64_t>())
				write(value.to_int64());
			else if (type == RTTI::TypeInfo::get<uint8_t>())
				write(value.to_uint8());
			else if (type == RTTI::TypeInfo::get<uint16_t>())
				write(value.to_uint16());
			else if (type == RTTI::TypeInfo::get<uint32_t>())
				write(value.to_uint32());
			else if (type == RTTI::TypeInfo::get<uint64_t>())
				write(value.to_uint64());
			else if (type == RTTI::TypeInfo::get<float>())
				write(value.to_float());
			else if (type == RTTI::TypeInfo::get<double>())
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
		else if (type == RTTI::TypeInfo::get<std::string>())
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