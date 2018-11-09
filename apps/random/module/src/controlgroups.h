#pragma once

#include <nap/resource.h>

namespace nap
{
	class NAPAPI ControlGroups : public Resource
	{
		RTTI_ENABLE(Resource)
	public:
		struct ControlGroup
		{
			ControlGroup() = default;

			std::string			mName;						///< Property: 'Name' the name of the group
			float				mBrightness;				///< Property: 'Brightness' the final brightness of the group
			std::vector<int>	mControlGroupIndexes;		///< Property: 'ControlGroupIndexes' artnet group indices associated with this group
		};

		/**
		 * Creates and inits control group
		 */
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * @param index the index of the group you want to fetch
		 * @return a group associated with a specific index
		 */
		const ControlGroup& getGroup(int index) const;

		/**
		 *@return total number of control groups managed by this container
		 */
		int getCount() const								{ return mControlGroups.size(); }

		/**
		 * Finds a group with a specific control group index
		 * @return the group associated with a s specific control group index, nullptr if not found
		 */
		const ControlGroup* findGroup(int controlIndex) const;

		std::vector<ControlGroup>	mControlGroups;			////< Property: 'List of control groups'
	};
}
