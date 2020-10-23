/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "database.h"
#include "sqlite3.h"

#include <nap/logger.h>

namespace nap
{
	Database::Database(rtti::Factory& factory) :
		mFactory(&factory)
	{
	}


	Database::~Database()
	{
		// Need to destroy the tables first to ensure the database isn't 'busy' (the tables hold on to sqlite3_stmt objects which are destroyed in their destructors)
		mTables.clear();

		switch (sqlite3_close(mDatabase))
		{
		case SQLITE_OK:
			nap::Logger::info("Successfully closed database");
			break;
		case SQLITE_BUSY:
			nap::Logger::error("Unable to close database: BUSY");
			break;
		default:
			nap::Logger::error("Unable to close database: Unknown Error");
			break;
		}
	}


	bool Database::init(const std::string& path, utility::ErrorState& errorState)
	{
		if (!errorState.check(sqlite3_open(path.c_str(), &mDatabase) == SQLITE_OK, "Failed to open database"))
		{
			sqlite3_close(mDatabase);
			return false;
		}

		char* errorMessage = nullptr;
		if (!errorState.check(sqlite3_exec(mDatabase, "PRAGMA journal_mode = WAL", nullptr, nullptr, &errorMessage) == SQLITE_OK, "Failed to set journal mode: %s", errorMessage != nullptr ? errorMessage : "Unknown error"))
		{
			sqlite3_free(errorMessage);
			return false;
		}

		if (!errorState.check(sqlite3_exec(mDatabase, "PRAGMA temp_store = 2", nullptr, nullptr, &errorMessage) == SQLITE_OK, "Failed to set temp_store: %s", errorMessage != nullptr ? errorMessage : "Unknown error"))
		{
			sqlite3_free(errorMessage);
			return false;
		}

		if (!errorState.check(sqlite3_exec(mDatabase, "PRAGMA synchronous = OFF", nullptr, nullptr, &errorMessage) == SQLITE_OK, "Failed to set synchronous mode: %s", errorMessage != nullptr ? errorMessage : "Unknown error"))
		{
			sqlite3_free(errorMessage);
			return false;
		}

		if (!errorState.check(sqlite3_exec(mDatabase, "PRAGMA locking_mode = EXCLUSIVE", nullptr, nullptr, &errorMessage) == SQLITE_OK, "Failed to set locking mode: %s", errorMessage != nullptr ? errorMessage : "Unknown error"))
		{
			sqlite3_free(errorMessage);
			return false;
		}

		return true;
	}


	DatabaseTable* Database::getOrCreateTable(const std::string& tableID, const rtti::TypeInfo& objectType, const DatabaseTable::DatabasePropertyPathList& propertiesToIgnore, utility::ErrorState& errorState)
	{
 		DatabaseTableMap::iterator pos = mTables.find(tableID);
 		if (pos != mTables.end())
 			return pos->second.get();

 		std::unique_ptr<DatabaseTable> table = std::make_unique<DatabaseTable>(*mDatabase, *mFactory, tableID, objectType);
 		if (!table->init(propertiesToIgnore, errorState))
 			return nullptr;

		auto inserted = mTables.emplace(std::make_pair(tableID, std::move(table)));
		return inserted.first->second.get();
	}
}