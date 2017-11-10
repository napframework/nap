#include "componentptr.h"

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::ComponentPtrBase)
	RTTI_FUNCTION("toString",			&nap::ComponentPtrBase::toString)
	RTTI_FUNCTION("translateTargetID",	&nap::ComponentPtrBase::translateTargetID)
	RTTI_FUNCTION("assign",				&nap::ComponentPtrBase::assign)
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
