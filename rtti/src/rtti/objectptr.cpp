#include "rtti/objectptr.h"

namespace nap
{
	namespace rtti
	{
		ObjectPtrManager& ObjectPtrManager::get()
		{
			static ObjectPtrManager manager;
			return manager;
		}
	}
}