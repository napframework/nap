#include "database.h"
#include "sqlite3.h"

namespace nap
{
	Database::Database(rtti::Factory& factory) :
		mFactory(&factory)
	{
	}

	Database::~Database()
	{
		sqlite3_close(mDatabase);
	}

	bool Database::init(const std::string& path, utility::ErrorState& errorState)
	{
		if (!errorState.check(sqlite3_open(path.c_str(), &mDatabase) == SQLITE_OK, "Failed to open database"))
		{
			sqlite3_close(mDatabase);
			return false;
		}

		char* errorMessage = nullptr;
		if (!errorState.check(sqlite3_exec(mDatabase, "PRAGMA journal_mode = MEMORY", nullptr, nullptr, &errorMessage) == SQLITE_OK, "Failed to set journal mode: %s", errorMessage != nullptr ? errorMessage : "Unknown error"))
		{
			sqlite3_free(errorMessage);
			return false;
		}

		if (!errorState.check(sqlite3_exec(mDatabase, "PRAGMA temp_store = 2", nullptr, nullptr, &errorMessage) == SQLITE_OK, "Failed to set journal mode: %s", errorMessage != nullptr ? errorMessage : "Unknown error"))
		{
			sqlite3_free(errorMessage);
			return false;
		}

		if (!errorState.check(sqlite3_exec(mDatabase, "PRAGMA synchronous = 0", nullptr, nullptr, &errorMessage) == SQLITE_OK, "Failed to set journal mode: %s", errorMessage != nullptr ? errorMessage : "Unknown error"))
		{
			sqlite3_free(errorMessage);
			return false;
		}

		if (!errorState.check(sqlite3_exec(mDatabase, "PRAGMA locking_mode = EXCLUSIVE", nullptr, nullptr, &errorMessage) == SQLITE_OK, "Failed to set journal mode: %s", errorMessage != nullptr ? errorMessage : "Unknown error"))
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

 		std::unique_ptr<DatabaseTable> table = std::make_unique<DatabaseTable>(*this, *mFactory, tableID, objectType);
 		if (!table->init(propertiesToIgnore, errorState))
 			return nullptr;

		auto inserted = mTables.emplace(std::make_pair(tableID, std::move(table)));
		return inserted.first->second.get();
	}
}