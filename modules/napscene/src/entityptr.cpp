#include "entityptr.h"

RTTI_BEGIN_CLASS(nap::EntityPtr)
	RTTI_FUNCTION("toString",			&nap::EntityPtr::toString)
	RTTI_FUNCTION("translateTargetID",	&nap::EntityPtr::translateTargetID)
	RTTI_FUNCTION("assign",				&nap::EntityPtr::assign)
RTTI_END_CLASS

namespace nap
{
	std::string EntityPtr::translateTargetID(const std::string& targetID)
	{
		size_t pos = targetID.find_last_of('/');
		if (pos == std::string::npos)
			return targetID;

		return targetID.substr(pos + 1);
	}
}
