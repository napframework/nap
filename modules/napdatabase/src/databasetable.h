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

	class NAPAPI DatabasePropertyPath final
	{
	public:
		static std::unique_ptr<DatabasePropertyPath> sCreate(const rtti::TypeInfo& rootType, const rtti::Path& rttiPath, utility::ErrorState& errorState);
		
		const rtti::Path& getRTTIPath() const { return mRTTIPath; }
		
		std::string toString() const;

	private:
		DatabasePropertyPath(const rtti::Path& rttiPath);

	private:
		rtti::Path	mRTTIPath;
	};

	class NAPAPI DatabaseTable final
	{
	public:
		using DatabasePropertyPathList = std::vector<DatabasePropertyPath>;

		DatabaseTable(Database& database, const std::string& tableID, const rtti::TypeInfo& objectType);
		~DatabaseTable();

		DatabaseTable(const DatabaseTable& rhs) = delete;
		DatabaseTable& operator=(const DatabaseTable& rhs) = delete;

		bool init(const DatabasePropertyPathList& propertiesToIgnore, utility::ErrorState& errorState);
		bool getOrCreateIndex(const DatabasePropertyPath& propertyPath, utility::ErrorState& errorState);
		bool add(const rtti::Object& object, utility::ErrorState& errorState);
		bool query(const std::string& whereClause, std::vector<std::unique_ptr<rtti::Object>>& objects, utility::ErrorState& errorState);
		bool clear(utility::ErrorState& errorState);

	private:
		struct Column
		{
			std::unique_ptr<DatabasePropertyPath>	mPath;
			std::string						mSqlType;
		};

		using ColumnList = std::vector<Column>;

		rtti::TypeInfo	mObjectType;
		Database*		mDatabase;
		std::string		mTableID;
		ColumnList		mColumns;

		sqlite3_stmt*   mInsertStatement = nullptr;
	};
}
