// Local Includes
#include "cameracomponent.h"

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::CameraComponentInstance)
RTTI_END_CLASS

namespace nap
{
	CameraComponentInstance::CameraComponentInstance(EntityInstance& entity) :
		ComponentInstance(entity)
	{
	}
}