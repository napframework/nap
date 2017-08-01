// Local Includes
#include "cameracomponent.h"

RTTI_BEGIN_BASE_CLASS(nap::CameraComponentInstance)
RTTI_END_CLASS

namespace nap
{
	CameraComponentInstance::CameraComponentInstance(EntityInstance& entity) :
		ComponentInstance(entity)
	{
	}
}