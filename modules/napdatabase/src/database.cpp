#include "database.h"
#include "sqlite3.h"

namespace nap
{
	Database::Database()
	{
	}

	Database::~Database()
	{
		sqlite3_close(mDatabase);
	}

	bool Database::init(utility::ErrorState& errorState)
	{
		if (!errorState.check(sqlite3_open("emography.db", &mDatabase) == SQLITE_OK, "Failed to open database"))
		{
			sqlite3_close(mDatabase);
			return false;
		}

		return true;
	}

	DatabaseTable* Database::createTable(const std::string& tableID, const rtti::TypeInfo& objectType, utility::ErrorState& errorState)
	{
 		DatabaseTableMap::iterator pos = mTables.find(tableID);
 		if (pos != mTables.end())
 			return pos->second.get();

		//std::unique_ptr<DatabaseTable> table;
 		std::unique_ptr<DatabaseTable> table = std::make_unique<DatabaseTable>(*this, tableID, objectType);
 		if (!table->init(errorState))
 			return nullptr;

		auto inserted = mTables.emplace(std::make_pair(tableID, std::move(table)));
		return inserted.first->second.get();
	}
}