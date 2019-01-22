#pragma once

#include "utility/dllexport.h"
#include "utility/errorstate.h"
#include "sqlite3.h"
#include "databasetable.h"

namespace nap
{
	class NAPAPI Database final
	{
	public:
		Database();
		~Database();

		Database(const Database& rhs) = delete;
		Database& operator=(const Database& rhs) = delete;
		
		bool init(const std::string& path, utility::ErrorState& errorState);
		DatabaseTable* createTable(const std::string& tableID, const rtti::TypeInfo& objectType, utility::ErrorState& errorState);

	private:
		sqlite3& GetDatabase() { return *mDatabase; }

	private:
		friend class DatabaseTable;
		using DatabaseTableMap = std::unordered_map<std::string, std::unique_ptr<DatabaseTable>>;

		sqlite3*			mDatabase = nullptr;		
		DatabaseTableMap	mTables;
	};
}
