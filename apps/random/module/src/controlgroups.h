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

			ControlGroup(const std::string& name, float brightness, std::vector<int> controlGroupIndexes) :
				mName(name),
				mBrightness(brightness),
				mControlGroupIndexes(controlGroupIndexes) {}

			std::string			mName;
			float				mBrightness;
			std::vector<int>	mControlGroupIndexes;
		};

		/**
		* Creates and inits control group
		*/
		virtual bool init(utility::ErrorState& errorState);

		std::vector<ControlGroup>	mControlGroups;

	private:
	};
}
