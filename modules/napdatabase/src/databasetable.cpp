#include "database.h"
#include "sqlite3.h"
#include "rtti/path.h"
#include "rtti/factory.h"

namespace nap
{
	using VisitRTTIPropertyTypesCallback = std::function<void(const rtti::Property&, const rtti::Path&)>;

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


	static bool sBindColumnValue(const rtti::TypeInfo& type, const rtti::Variant& value, sqlite3_stmt& statement, int index)
	{
		if (type.is_arithmetic())
		{
			if (type == rtti::TypeInfo::get<bool>())
				return sqlite3_bind_int(&statement, index, value.to_bool() ? 1 : 0) == SQLITE_OK;
			else if (type == rtti::TypeInfo::get<char>())
				return sqlite3_bind_int(&statement, index, value.to_int8()) == SQLITE_OK;
			else if (type == rtti::TypeInfo::get<int8_t>())
				return sqlite3_bind_int(&statement, index, value.to_int8()) == SQLITE_OK;
			else if (type == rtti::TypeInfo::get<int16_t>())
				return sqlite3_bind_int(&statement, index, value.to_int16()) == SQLITE_OK;
			else if (type == rtti::TypeInfo::get<int32_t>())
				return sqlite3_bind_int(&statement, index, value.to_int32()) == SQLITE_OK;
			else if (type == rtti::TypeInfo::get<int64_t>())
				return sqlite3_bind_int64(&statement, index, value.to_int64()) == SQLITE_OK;
			else if (type == rtti::TypeInfo::get<uint8_t>())
				return sqlite3_bind_int(&statement, index, value.to_uint8()) == SQLITE_OK;
			else if (type == rtti::TypeInfo::get<uint16_t>())
				return sqlite3_bind_int(&statement, index, value.to_uint16()) == SQLITE_OK;
			else if (type == rtti::TypeInfo::get<uint32_t>())
				return sqlite3_bind_int(&statement, index, value.to_uint32()) == SQLITE_OK;
			else if (type == rtti::TypeInfo::get<uint64_t>())
				return sqlite3_bind_int64(&statement, index, value.to_uint64()) == SQLITE_OK;
			else if (type == rtti::TypeInfo::get<float>())
				return sqlite3_bind_double(&statement, index, value.to_float()) == SQLITE_OK;
			else if (type == rtti::TypeInfo::get<double>())
				return sqlite3_bind_double(&statement, index, value.to_double()) == SQLITE_OK;
		}
		else if (type.is_enumeration() || type == rtti::TypeInfo::get<std::string>())
		{
			// Try to convert the enum to string first
			bool conversion_succeeded = false;
			std::string result = value.to_string(&conversion_succeeded);
			assert(conversion_succeeded);
			return sqlite3_bind_text(&statement, index, result.c_str(), result.size(), SQLITE_TRANSIENT) == SQLITE_OK;
		}

		assert(false);
		return "";
	}

	static bool sSetColumnValue(rtti::Object& object, const rtti::Path& path, sqlite3_stmt& statement, int columnIndex)
	{
		rtti::ResolvedPath resolvedPath;
		bool was_resolved = path.resolve(&object, resolvedPath);
		assert(was_resolved);
		rtti::TypeInfo type = resolvedPath.getType();

		if (type.is_arithmetic())
		{
			if (type == rtti::TypeInfo::get<bool>())
				return resolvedPath.setValue(sqlite3_column_int(&statement, columnIndex) != 0);
			else if (type == rtti::TypeInfo::get<char>())
				return resolvedPath.setValue(static_cast<char>(sqlite3_column_int(&statement, columnIndex)));
			else if (type == rtti::TypeInfo::get<int8_t>())
				return resolvedPath.setValue(static_cast<int8_t>(sqlite3_column_int(&statement, columnIndex)));
			else if (type == rtti::TypeInfo::get<int16_t>())
				return resolvedPath.setValue(static_cast<int16_t>(sqlite3_column_int(&statement, columnIndex)));
			else if (type == rtti::TypeInfo::get<int32_t>())
				return resolvedPath.setValue(static_cast<int32_t>(sqlite3_column_int(&statement, columnIndex)));
			else if (type == rtti::TypeInfo::get<int64_t>())
				return resolvedPath.setValue(static_cast<int64_t>(sqlite3_column_int64(&statement, columnIndex)));
			else if (type == rtti::TypeInfo::get<uint8_t>())
				return resolvedPath.setValue(static_cast<uint8_t>(sqlite3_column_int(&statement, columnIndex)));
			else if (type == rtti::TypeInfo::get<uint16_t>())
				return resolvedPath.setValue(static_cast<uint16_t>(sqlite3_column_int(&statement, columnIndex)));
			else if (type == rtti::TypeInfo::get<uint32_t>())
				return resolvedPath.setValue(static_cast<uint32_t>(sqlite3_column_int(&statement, columnIndex)));
			else if (type == rtti::TypeInfo::get<uint64_t>())
				return resolvedPath.setValue(static_cast<uint64_t>(sqlite3_column_int64(&statement, columnIndex)));
			else if (type == rtti::TypeInfo::get<float>())
				return resolvedPath.setValue(static_cast<float>(sqlite3_column_double(&statement, columnIndex)));
			else if (type == rtti::TypeInfo::get<double>())
				return resolvedPath.setValue(static_cast<double>(sqlite3_column_double(&statement, columnIndex)));
		}
		else if (type.is_enumeration() || type == rtti::TypeInfo::get<std::string>())
		{
			return resolvedPath.setValue(std::string((char*)sqlite3_column_text(&statement, columnIndex)));
		}

		assert(false);
		return false;
	}

	//////////////////////////////////////////////////////////////////////////

	DatabasePropertyPath::DatabasePropertyPath(const rtti::Path& rttiPath) :
		mRTTIPath(rttiPath)
	{
	}

	std::unique_ptr<DatabasePropertyPath> DatabasePropertyPath::sCreate(const rtti::TypeInfo& rootType, const rtti::Path& rttiPath, utility::ErrorState& errorState)
	{
		// Can't resolve an empty path
		if (rttiPath.getLength() == 0)
			return nullptr;

		rtti::TypeInfo current_type = rootType;
		for (int index = 0; index < rttiPath.getLength(); ++index)
		{
			const rtti::PathElement& element = rttiPath.getElement(index);

			// Arrays are not supported
			if (!errorState.check(element.mType != rtti::PathElement::Type::ARRAY_ELEMENT, "Array properties are not supported"))
				return nullptr;

			// Handle attribute
			assert(element.mType == rtti::PathElement::Type::ATTRIBUTE);
			rtti::Property property = current_type.get_property(element.Attribute.Name);
			if (!errorState.check(property.is_valid(), "Failed to find property %s on type %s", element.Attribute.Name, current_type.get_name().data()))
				return nullptr;

			current_type = property.get_type().is_wrapper() ? property.get_type().get_wrapped_type() : property.get_type();
		}

		if (!errorState.check(current_type.get_properties().empty(), "Path ends in a property with sub properties"))
			return nullptr;

		return std::unique_ptr<DatabasePropertyPath>(new DatabasePropertyPath(rttiPath));
	}

	std::string DatabasePropertyPath::toString() const
	{
		std::string column_name = mRTTIPath.toString();
		std::replace(column_name.begin(), column_name.end(), '/', '$');
		return column_name;
	}

	//////////////////////////////////////////////////////////////////////////


	DatabaseTable::DatabaseTable(Database& database, const std::string& tableID, const rtti::TypeInfo& objectType) :
		mTableID(tableID),
		mObjectType(objectType),
		mDatabase(&database)
	{
		std::replace(mTableID.begin(), mTableID.end(), ':', '_');
	}

	DatabaseTable::~DatabaseTable()
	{
		sqlite3_finalize(mInsertStatement);
	}

	bool DatabaseTable::init(utility::ErrorState& errorState)
	{
		sqlite3& database = mDatabase->GetDatabase();
	
		rtti::Path current_path;
 		bool result = sVisitRTTIPropertyTypes(mObjectType, current_path, [this](const rtti::Property& property, const rtti::Path& path)
		{
			utility::ErrorState errorState;
			std::unique_ptr<DatabasePropertyPath> database_path = DatabasePropertyPath::sCreate(mObjectType, path, errorState);
			assert(database_path != nullptr);

			mColumns.push_back({ std::move(database_path), sGetTypeString(property.get_type()) });
			
		}, errorState);

		if (!result)
			return false;

		std::string createTableSql = utility::stringFormat("CREATE TABLE IF NOT EXISTS %s (", mTableID.c_str());
		std::string insertColumnsSql;
		std::string insertValuesSql;
		for (int index = 0; index < mColumns.size(); ++index)
		{
			Column& column = mColumns[index];

			std::string column_name = column.mPath->toString();

			const char* separator = index < mColumns.size() - 1 ? ", " : "";
			createTableSql += utility::stringFormat("'%s' %s%s", column_name.c_str(), column.mSqlType.c_str(), separator);

			insertColumnsSql += utility::stringFormat("'%s'%s", column_name.c_str(), separator);
			insertValuesSql += utility::stringFormat("?%s", separator);
		}
		createTableSql += ");";

		char* errorMessage = nullptr;
		if (!errorState.check(sqlite3_exec(&database, createTableSql.c_str(), nullptr, nullptr, &errorMessage) == SQLITE_OK, "Failed to create table with ID %s: %s", mTableID.c_str(), errorMessage == nullptr ? "" : errorMessage))
		{
			sqlite3_free(errorMessage);
			return false;
		}

		std::string insertRowSql = utility::stringFormat("INSERT INTO %s (%s) VALUES (%s)", mTableID.c_str(), insertColumnsSql.c_str(), insertValuesSql.c_str());
		if (!errorState.check(sqlite3_prepare_v2(&mDatabase->GetDatabase(), insertRowSql.c_str(), insertRowSql.size(), &mInsertStatement, nullptr) == SQLITE_OK, "Failed to create insert query %s", insertRowSql.c_str()))
			return false;

		return true;
	}


	bool DatabaseTable::add(const rtti::Object& object, utility::ErrorState& errorState)
	{
		// Types need to match exactly: even if the type is derived, it could mean that additional properties were added, making it incompatible with the table
		assert(object.get_type() == mObjectType);

		std::string columns;
		std::string values;
		for (int index = 0; index < mColumns.size(); ++index)
		{
			Column& column = mColumns[index];

			rtti::ResolvedPath resolvedPath;
			bool resolved = column.mPath->getRTTIPath().resolve(&object, resolvedPath);
			assert(resolved);

			if (!errorState.check(sBindColumnValue(resolvedPath.getType(), resolvedPath.getValue(), *mInsertStatement, index+1), "Failed to set value for column %d", index))
				return false;
		}

		if (!errorState.check(sqlite3_step(mInsertStatement) == SQLITE_DONE, "Failed to execute insert statement"))
			return false;

		if (!errorState.check(sqlite3_reset(mInsertStatement) == SQLITE_OK, "Failed to reset insert statement"))
			return false;

		return true;
	}

	bool DatabaseTable::getLast(int count, std::vector<std::unique_ptr<rtti::Object>>& objects, utility::ErrorState& errorState)
	{
 		std::string sql = utility::stringFormat("SELECT * FROM %s LIMIT %d OFFSET (SELECT COUNT(*) FROM %s)-%d", mTableID.c_str(), count, mTableID.c_str(), count);
 		sqlite3_stmt* statement = nullptr;
 		if (!errorState.check(sqlite3_prepare_v2(&mDatabase->GetDatabase(), sql.c_str(), sql.size(), &statement, nullptr) == SQLITE_OK, "Failed to create query %s", sql.c_str()))
 			return false;

		rtti::Factory factory;

		while (sqlite3_step(statement) == SQLITE_ROW)
		{
			std::unique_ptr<rtti::Object> object(factory.create(mObjectType));
			for (int column_index = 0; column_index < mColumns.size(); ++column_index)
			{
				const Column& column = mColumns[column_index];
				sSetColumnValue(*object, column.mPath->getRTTIPath(), *statement, column_index);
			}
			objects.emplace_back(std::move(object));
		}

		sqlite3_finalize(statement);

		return true;
	}

	bool DatabaseTable::query(const std::string& whereClause, std::vector<std::unique_ptr<rtti::Object>>& objects, utility::ErrorState& errorState)
	{
		std::string sql = utility::stringFormat("SELECT * FROM %s WHERE %s", mTableID.c_str(), whereClause.c_str());
		sqlite3_stmt* statement = nullptr;
		if (!errorState.check(sqlite3_prepare_v2(&mDatabase->GetDatabase(), sql.c_str(), sql.size(), &statement, nullptr) == SQLITE_OK, "Failed to create query %s", sql.c_str()))
			return false;

		rtti::Factory factory;

		while (sqlite3_step(statement) == SQLITE_ROW)
		{
			std::unique_ptr<rtti::Object> object(factory.create(mObjectType));
			for (int column_index = 0; column_index < mColumns.size(); ++column_index)
			{
				const Column& column = mColumns[column_index];
				sSetColumnValue(*object, column.mPath->getRTTIPath(), *statement, column_index);
			}
			objects.emplace_back(std::move(object));
		}

		sqlite3_finalize(statement);

		return true;
	}

	bool DatabaseTable::createIndex(const DatabasePropertyPath& propertyPath, utility::ErrorState& errorState)
	{
		std::string column_name = propertyPath.toString();

		std::string sql = utility::stringFormat("CREATE INDEX IF NOT EXISTS \"%s_%s\" ON %s (\"%s\")", mTableID.c_str(), column_name.c_str(), mTableID.c_str(), column_name.c_str());

		char* errorMessage = nullptr;
		if (!errorState.check(sqlite3_exec(&mDatabase->GetDatabase(), sql.c_str(), nullptr, nullptr, &errorMessage) == SQLITE_OK, "Failed to create index on table with ID %s: %s", mTableID.c_str(), errorMessage == nullptr ? "" : errorMessage))
		{
			sqlite3_free(errorMessage);
			return false;
		}

		return true;
	}
}