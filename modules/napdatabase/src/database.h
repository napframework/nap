/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include "utility/dllexport.h"
#include "utility/errorstate.h"
#include "databasetable.h"

namespace nap
{

	/**
	 * Wrapper around a SQLite database used to store RTTI objects for a specific type in tables. 
	 * The Database interface is primarily used to create tables for a specific object type.
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
		 * @param path path to the database file.
		 * @param errorState : if the function returns false, contains error information.
		 */
		bool init(const std::string& path, utility::ErrorState& errorState);

		/**
		 * Gets or creates a database table with the given ID. For each rtti property in objectType, a column is created. To filter out certain properties use inpropertiesToIgnore.
		 * @param tableID unique ID for the table.
		 * @param objectType object used for serialization to the database.
		 * @param propertiesToIgnore a list of properties that are not serialized to the database.
		 * @param errorState if the function returns nullptr, contains error information.
		 * @return the DatabaseTable object.
		 */
		DatabaseTable* getOrCreateTable(const std::string& tableID, const rtti::TypeInfo& objectType, const DatabaseTable::DatabasePropertyPathList& propertiesToIgnore, utility::ErrorState& errorState);

	private:
		using DatabaseTableMap = std::unordered_map<std::string, std::unique_ptr<DatabaseTable>>;

		rtti::Factory*		mFactory = nullptr;			///< Factory used to create objects when querying data
		sqlite3*			mDatabase = nullptr;		///< Sqlite object
		DatabaseTableMap	mTables;					///< Map from table ID to DatabaseTable
	};
}
