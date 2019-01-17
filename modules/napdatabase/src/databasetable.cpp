#include "database.h"
#include "sqlite3.h"
#include "rtti/path.h"

namespace nap
{
	using VisitRTTIPropertyTypesCallback = std::function<void(const rtti::Property&, const rtti::Path&)>;
	using VisitRTTIPropertyValuesCallback = std::function<void(const rtti::Property&, const rtti::Path&, const rtti::Variant&)>;

	static bool sVisitRTTIPropertyTypes(const rtti::TypeInfo& type, rtti::Path& path, const VisitRTTIPropertyTypesCallback& callback, utility::ErrorState& errorState)
	{
		// Recursively visit each property of the type
		for (const rtti::Property& property : type.get_properties())
		{
			path.pushAttribute(property.get_name().data());

			rtti::TypeInfo actual_type = property.get_type().is_wrapper() ? property.get_type().get_wrapped_type() : property.get_type();

			if (!errorState.check(!actual_type.is_array() && !actual_type.is_pointer(), "Serializing objects with pointers or arrays to a database is not supported"))
				return false;

			if (actual_type.is_arithmetic() || actual_type.is_enumeration() || actual_type == rtti::TypeInfo::get<std::string>())
			{
				callback(property, path);
			}
			else
			{
				if (!sVisitRTTIPropertyTypes(actual_type, path, callback, errorState))
					return false;
			}

			path.popBack();
		}

		return true;
	}


	static bool sVisitRTTIPropertyValues(const rtti::Variant& variant, const rtti::TypeInfo& type, rtti::Path& path, const VisitRTTIPropertyValuesCallback& callback, utility::ErrorState& errorState)
	{
		// Recursively visit each property of the type
		for (const rtti::Property& property : type.get_properties())
		{
			path.pushAttribute(property.get_name().data());

			rtti::TypeInfo actual_type = property.get_type().is_wrapper() ? property.get_type().get_wrapped_type() : property.get_type();

			if (!errorState.check(!actual_type.is_array() && !actual_type.is_pointer(), "Serializing objects with pointers or arrays to a database is not supported"))
				return false;

			rtti::Variant value = property.get_value(variant);
			if (actual_type.is_arithmetic() || actual_type.is_enumeration() || actual_type == rtti::TypeInfo::get<std::string>())
			{
				callback(property, path, value);
			}
			else
			{
				if (!sVisitRTTIPropertyValues(value, actual_type, path, callback, errorState))
					return false;
			}

			path.popBack();
		}

		return true;
	}


	static std::string sGetTypeString(const rtti::TypeInfo& type)
	{
		if (type.is_arithmetic())
		{
			if (type == rtti::TypeInfo::get<bool>() ||
				type == rtti::TypeInfo::get<char>() ||
				type == rtti::TypeInfo::get<int8_t>() ||
				type == rtti::TypeInfo::get<int16_t>() ||
				type == rtti::TypeInfo::get<int32_t>() ||
				type == rtti::TypeInfo::get<int64_t>() ||
				type == rtti::TypeInfo::get<uint8_t>() ||
				type == rtti::TypeInfo::get<uint16_t>() ||
				type == rtti::TypeInfo::get<uint32_t>() ||
				type == rtti::TypeInfo::get<uint64_t>())
			{
				return "INTEGER";
			}
			else if (type == rtti::TypeInfo::get<float>() || type == rtti::TypeInfo::get<double>())
			{
				return "REAL";
			}
		}
		else if (type.is_enumeration() ||
			type == rtti::TypeInfo::get<std::string>())
		{
			return "TEXT";
		}

		assert(false);
		return "";
	}


	static std::string sGetTypeValue(const rtti::TypeInfo& type, const rtti::Variant& value)
	{
		if (type.is_arithmetic())
		{
			if (type == rtti::TypeInfo::get<bool>())
				return utility::stringFormat("%d", value.to_bool() ? 1 : 0);
			else if (type == rtti::TypeInfo::get<char>())
				return utility::stringFormat("%u", value.to_uint8());
			else if (type == rtti::TypeInfo::get<int8_t>())
				return utility::stringFormat("%d", value.to_int8());
			else if (type == rtti::TypeInfo::get<int16_t>())
				return utility::stringFormat("%d", value.to_int16());
			else if (type == rtti::TypeInfo::get<int32_t>())
				return utility::stringFormat("%d", value.to_int32());
			else if (type == rtti::TypeInfo::get<int64_t>())
				return utility::stringFormat("%lld", value.to_int64());
			else if (type == rtti::TypeInfo::get<uint8_t>())
				return utility::stringFormat("%u", value.to_uint8());
			else if (type == rtti::TypeInfo::get<uint16_t>())
				return utility::stringFormat("%u", value.to_uint16());
			else if (type == rtti::TypeInfo::get<uint32_t>())
				return utility::stringFormat("%u", value.to_uint32());
			else if (type == rtti::TypeInfo::get<uint64_t>())
				return utility::stringFormat("%llu", value.to_uint64());
			else if (type == rtti::TypeInfo::get<float>())
				return utility::stringFormat("%f", value.to_float());
			else if (type == rtti::TypeInfo::get<double>())
				return utility::stringFormat("%f", value.to_double());
		}
		else if (type.is_enumeration() || type == rtti::TypeInfo::get<std::string>())
		{
			// Try to convert the enum to string first
			bool conversion_succeeded = false;
			std::string result = value.to_string(&conversion_succeeded);
			assert(conversion_succeeded);
			return utility::stringFormat("'%s'", result.c_str());
		}

		assert(false);
		return "";
	}

	//////////////////////////////////////////////////////////////////////////


	DatabaseTable::DatabaseTable(Database& database, const std::string& tableID, const rtti::TypeInfo& objectType) :
		mTableID(tableID),
		mObjectType(objectType),
		mDatabase(&database)
	{
		std::replace(mTableID.begin(), mTableID.end(), ':', '_');
	}


	bool DatabaseTable::init(utility::ErrorState& errorState)
	{
		sqlite3& database = mDatabase->GetDatabase();

		std::string sql = utility::stringFormat("CREATE TABLE IF NOT EXISTS %s (", mTableID.c_str());

		bool is_first = true;
		rtti::Path path;

 		bool result = sVisitRTTIPropertyTypes(mObjectType, path, [&sql, &is_first](const rtti::Property& property, const rtti::Path& path)
		{
			std::string column_name = path.toString().c_str();
			std::replace(column_name.begin(), column_name.end(), '/', '.');
			sql += utility::stringFormat("%s'%s' %s", is_first ? "" : ", ", column_name.c_str(), sGetTypeString(property.get_type()).c_str());
			is_first = false;
		}, errorState);
		
		if (!result)
			return false;

		sql += ");";

		char* errorMessage = nullptr;
		if (!errorState.check(sqlite3_exec(&database, sql.c_str(), nullptr, nullptr, &errorMessage) == SQLITE_OK, "Failed to create table with ID %s: %s", mTableID.c_str(), errorMessage == nullptr ? "" : errorMessage))
		{
			sqlite3_free(errorMessage);
			return false;
		}

		return true;
	}


	bool DatabaseTable::add(const rtti::Object& object, utility::ErrorState& errorState)
	{
		// Types need to match exactly: even if the type is derived, it could mean that additional properties were added, making it incompatible with the table
		assert(object.get_type() == mObjectType);

		bool is_first = true;
		rtti::Path path;

		std::string columns;
		std::string values;

		bool result = sVisitRTTIPropertyValues(&object, mObjectType, path, [&columns, &values, &is_first](const rtti::Property& property, const rtti::Path& path, const rtti::Variant& value)
		{
			std::string column_name = path.toString().c_str();
			std::replace(column_name.begin(), column_name.end(), '/', '.');

			columns += utility::stringFormat("%s'%s'", is_first ? "" : ", ", column_name.c_str());
			values += utility::stringFormat("%s%s", is_first ? "" : ", ", sGetTypeValue(property.get_type(), value).c_str());

			is_first = false;
		}, errorState);

		// Everything that can go wrong during visiting of the values should already be verified on table creation
		assert(result);

		std::string sql = utility::stringFormat("INSERT INTO %s (%s) VALUES (%s)", mTableID.c_str(), columns.c_str(), values.c_str());

		char* errorMessage;
		if (!errorState.check(sqlite3_exec(&mDatabase->GetDatabase(), sql.c_str(), nullptr, nullptr, &errorMessage) == SQLITE_OK, "Failed to add object to table %s: %s", mTableID.c_str(), errorMessage))
		{
			sqlite3_free(errorMessage);
			return false;
		}

		return true;
	}
}