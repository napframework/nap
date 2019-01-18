#pragma once

#include "utility/dllexport.h"
#include "utility/errorstate.h"
#include "rtti/typeinfo.h"
#include "rtti/object.h"
#include "rtti/path.h"

struct sqlite3_stmt;

namespace nap
{
	class Database;

	class NAPAPI DatabaseTable
	{
	public:
		DatabaseTable(Database& database, const std::string& tableID, const rtti::TypeInfo& objectType);
		~DatabaseTable();
		
		bool init(utility::ErrorState& errorState);
		bool add(const rtti::Object& object, utility::ErrorState& errorState);

		bool getLast(int count, std::vector<std::unique_ptr<rtti::Object>>& objects, utility::ErrorState& errorState);
		bool query(const std::string& whereClause, std::vector<std::unique_ptr<rtti::Object>>& objects, utility::ErrorState& errorState);

	private:
		struct Column
		{
			rtti::Path	mPath;
			std::string mSqlType;
		};

		using ColumnList = std::vector<Column>;

		rtti::TypeInfo	mObjectType;
		Database*		mDatabase;
		std::string		mTableID;
		ColumnList		mColumns;

		sqlite3_stmt*   mInsertStatement = nullptr;
	};
}
