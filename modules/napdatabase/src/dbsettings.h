#pragma once

// External Includes
#include <nap/resource.h>
#include <nap/resourceptr.h>

namespace nap
{
	// Forward Declare
	class DBSetting;

	/**
	 * Represents a database settings object.
	 * Every database has global settings associated with it, such as a user etc.
	 */
	class NAPAPI DBSettings : public Resource
	{
		RTTI_ENABLE(Resource)
	public:
		virtual ~DBSettings()		{ }

		/**
		 * Ensures that every setting contains a valid key / value pair.
		 * Every DBSetting needs to have a key, the value can be empty.
		 * @return if initialization succeeded.
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		std::vector<DBSetting> mSettings;	///< Property: 'Settings' all available DB settings
	};


	//////////////////////////////////////////////////////////////////////////
	// DBSetting
	//////////////////////////////////////////////////////////////////////////

	/**
	 * Single database setting.
	 * Every setting represents a column in the database.
	 * Note that settings are for simplicity only strings for now.
	 */
	class NAPAPI DBSetting
	{
		RTTI_ENABLE()
	public:
		virtual ~DBSetting()		{ }

		/**
		 * Default constructor
		 */
		DBSetting()					{ }

		/**
		 * Every setting must contain a valid key / value
		 */
		DBSetting(std::string key, std::string value);

		std::string mKey;			///< Property: 'Key'
		std::string mValue;			///< Property: 'Value'
	};
}
