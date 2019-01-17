#pragma once

#include "utility/dllexport.h"
#include "utility/errorstate.h"
#include "sqlite3.h"
#include "rtti/typeinfo.h"
#include "rtti/object.h"

namespace nap
{
	class Database;

	class NAPAPI DatabaseTable
	{
	public:
		DatabaseTable(Database& database, const std::string& tableID, const rtti::TypeInfo& objectType);
		
		bool init(utility::ErrorState& errorState);
		bool add(const rtti::Object& object, utility::ErrorState& errorState);

		std::vector<rtti::Object*> getLast(int count)
		{
			return std::vector<rtti::Object*>();
		}

	private:
		rtti::TypeInfo	mObjectType;
		Database*		mDatabase;
		std::string		mTableID;
	};
}
