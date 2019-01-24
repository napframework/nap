#pragma once

#include "utility/dllexport.h"
#include "utility/errorstate.h"
#include "sqlite3.h"
#include "databasetable.h"

namespace nap
{

	/**
	 * 
	 */
	class NAPAPI Database final
	{
	public:
		Database(rtti::Factory& factory);
		~Database();

		Database(const Database& rhs) = delete;
		Database& operator=(const Database& rhs) = delete;
		
		/**
		 * Attempts to open or create the database and initializes it.
		 * @param path Path to the database file.
		 * @param errorState : if the function returns false, contains error information.
		 */
		bool init(const std::string& path, utility::ErrorState& errorState);

		/**
		 * Gets or creates a database table with ID @tableID. For each rtti property in @objectType, a column is created. To filter out certain properties, use the ignore 
		 * list in * @propertiesToIgnore.
		 * @param tableID: Unique ID for the table.
		 * @param objectType: Object used for serialization to the database.
		 * @param propertiesToIgnore: A list of properties that are not serialized to the database.
		 * @param errorState : if the function returns nullptr, contains error information.
		 * @return The DatabaseTable object.
		 */
		DatabaseTable* getOrCreateTable(const std::string& tableID, const rtti::TypeInfo& objectType, const DatabaseTable::DatabasePropertyPathList& propertiesToIgnore, utility::ErrorState& errorState);

	private:
		sqlite3& GetDatabase() { return *mDatabase; }

	private:
		friend class DatabaseTable;
		using DatabaseTableMap = std::unordered_map<std::string, std::unique_ptr<DatabaseTable>>;

		rtti::Factory*		mFactory = nullptr;			///< Factory used to create objects when querying data
		sqlite3*			mDatabase = nullptr;		///< Sqlite object
		DatabaseTableMap	mTables;					///< Map from table ID to DatabaseTable
	};
}
