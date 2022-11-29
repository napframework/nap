/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "utility/dllexport.h"
#include "utility/errorstate.h"
#include "rtti/typeinfo.h"
#include "rtti/object.h"
#include "rtti/path.h"
#include "sqlite3.h"

namespace nap
{
	namespace rtti
	{
		class Factory;
	}

	/**
	 * Identification of an path to an RTTI property that can be serialized to a DatabaseTable.
	 * Upon creation, it is verified that this is a property that can be serialized to a table, otherwise the creation will fail.
	 * This object is then further used to identify columns in the database table.
	 */
	class NAPAPI DatabasePropertyPath final
	{
	public:
		/**
		 * Attempts to create a path to a property that is targeting a column in a database. Array properties are not supported. Also,
		 * the client always needs to target leaf properties as these are the fields that are serialized to the database, so make sure 
		 * to expand any embedded classes and structures down to their leaf properties, otherwise the function will fail.
		 * @param rootType:	The type of the root object that holds the property.
		 * @param rttiPath:	The path to the property.
		 * @param errorState : if the function returns nullptr, contains error information.
		 * @return On success, it returns an object representing the path to the property. This can be used in the DatabaseTable 
		 *  interface to identify properties/columns. On failure, nullptr is returned. 
		 */
		static std::unique_ptr<DatabasePropertyPath> sCreate(const rtti::TypeInfo& rootType, const rtti::Path& rttiPath, utility::ErrorState& errorState);
		
		/**
		 * @return Returns the path to the property.
		 */
		const rtti::Path& getRTTIPath() const { return mRTTIPath; }

	private:
		DatabasePropertyPath(const rtti::Path& rttiPath);

	private:
		rtti::Path	mRTTIPath;		///< RTTI path to the property
	};

	/**
	 * Table in a SQlite database that for serializing RTTI object to and from the database. RTTI objects can have embedded classes/structs in them,
	 * but not arrays or pointers. So it is suitable for serializing vectors of relatively simple classes and structs to and from a database. To avoid
	 * serializing all properties, use the ignore list to prevent certain properties from being serialized.
	 * The table can be queried on an RTTI property. To speed up queries, an index can be created on a property.
	 */
	class NAPAPI DatabaseTable final
	{
	public:
		using DatabasePropertyPathList = std::vector<DatabasePropertyPath>;

		/**
		 * Constructor
		 * @param database: The internal sql database object managed by Database
		 * @param factory: A factory for creating objects when they are returned from the database.
		 * @param tableID: A unique ID representing this table
		 * @param objectType: The object type that will be serialized to this table.
		 */
		DatabaseTable(sqlite3& database, rtti::Factory& factory, const std::string& tableID, const rtti::TypeInfo& objectType);

		/**
		 * Destructor
		 */
		~DatabaseTable();

		DatabaseTable(const DatabaseTable& rhs) = delete;
		DatabaseTable& operator=(const DatabaseTable& rhs) = delete;

		/**
		 * Attempts to get or create the table in the database, prepares for serialization. Columns are created
		 * for each property that should be serialized. The name of the table and the column is derived from the
		 * type of the object and its properties. Because not all characters are supported, illegal characters 
		 * are replaced with characters that are valid.
		 * @param propertiesToIgnore: A list of properties that should not be serialized to the database.
		 * @param errorState: if the function returns false, contains error information.
		 */
		bool init(const DatabasePropertyPathList& propertiesToIgnore, utility::ErrorState& errorState);

		/**
		 * Creates an index on a table for speeding up queries.
		 * @param propertyPath: The property to create an index for.
		 * @param errorState: if the function returns false, contains error information.
		 */
		bool getOrCreateIndex(const DatabasePropertyPath& propertyPath, utility::ErrorState& errorState);

		/**
		 * Adds an object to the database. Object should be of the type that this table was bound to in the constructor.
		 * @param object: Object to serialize to the table.
		 * @param errorState: if the function returns false, contains error information.
		 */
		bool add(const rtti::Object& object, utility::ErrorState& errorState);

		/**
		 * Query the table for objects. The query will deserialize the objects through the factory that was passed onto the 
		 * constructor and fill it with the values from the database. The whereClause is a condition that can be filled in 
		 * to select the rows from the table that you want to deserialize. The text represents what comes after the WHERE
		 * statement in SQL. If it is empty, all objects are retrieved. Because the column names in the table are generated
		 * you should use the DatabasePropertyPath::toString function to retrieve the correct column name. Example:
		 *
		 *		query(utility::stringFormat("%s >= 0.2 AND %s > 0.5", property.toString()), object, errorState);
		 *
		 * @param whereClause: The part of the SQL query that comes after the WHERE statement. Keep empty to deserialize all.
		 * @param objects The deserialized objects
		 * @param errorState if the function returns false, contains error information.
		 */
		bool query(const std::string& whereClause, std::vector<std::unique_ptr<rtti::Object>>& objects, utility::ErrorState& errorState);

		/**
		 * Clears all rows from the table.
		 * @param errorState: if the function returns false, contains error information.
		 */
		bool clear(utility::ErrorState& errorState);

		/**
		 * Get the column name for the specified database path
		 *
		 * @param path The path to get the column name for
		 * @param errorState: if the function returns an empty string, contains error information.
		 * @return The column name for the specified path, or empty if the path could not be found
		 */
		std::string getColumnName(const DatabasePropertyPath& path, utility::ErrorState& errorState) const;

	private:
		/**
		 * Generate a column name for the specified RTTI path. The generated name is unique with respect to other columns in the table.
		 *
		 * @param path The path to generate the column name for
		 * @return A unique column name
		 */
		std::string generateUniqueColumnName(const rtti::Path& path) const;

	private:
		/**
		 * Metadata for a column in the database
		 */
		struct Column
		{
			std::unique_ptr<DatabasePropertyPath>	mPath;			///< Path to the property
			std::string								mName;			///< Name of the column in the database
			std::string								mSqlType;		///< SQL typename for a column (INTEGER/REAL/TEXT)
		};

		using ColumnList = std::vector<Column>;

		sqlite3*		mDatabase = nullptr;			///< Sqlite object
		sqlite3_stmt*   mInsertStatement = nullptr;		///< Already prepared statement for quick insertion of objects
		rtti::Factory*  mFactory = nullptr;				///< Factory used when deserializing object from the database
		rtti::TypeInfo	mObjectType;					///< Type of object that is used to serialize/deserialize this table
		std::string		mTableID;						///< Unique ID / name of the table
		ColumnList		mColumns;						///< List of all properties/columns that are to be serialized/deserialized
	};
}
