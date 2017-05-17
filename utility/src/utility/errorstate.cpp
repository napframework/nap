#include "utility/errorstate.h"
#include "utility/stringutils.h"

namespace nap
{
	namespace utility
	{
		const std::string ErrorState::toString() const
		{
			// No errors
			if (mErrorList.empty())
				return std::string();

			// Single error
			if (mErrorList.size() == 1)
				return mErrorList[0];

			// Multiple errors; build a 'stack' of error messages. Note: because error messages are added 'inside-out', 
			// the front of the list is the bottom of the stack and the back is the top.
			std::string result;
			result = stringFormat("%s:", mErrorList[mErrorList.size() - 1].c_str());
			for (int index = mErrorList.size() - 2; index >= 0; --index)
				result += stringFormat("\n\t%s", mErrorList[index].c_str());

			return result;
		}
	}
}