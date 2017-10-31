#include "componentptr.h"

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::ComponentPtrBase)
RTTI_END_CLASS

namespace nap
{
	std::string ComponentPtrBase::translateTargetID(const std::string& targetID)
	{
		size_t pos = targetID.find_last_of('/');
		if (pos == std::string::npos)
			return targetID;

		return targetID.substr(pos + 1);
	}
}