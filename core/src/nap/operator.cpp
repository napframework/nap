
#include <algorithm>
#include <nap/configure.h>
#include <nap/core.h>
#include <nap/operator.h>
#include <nap/patch.h>
#include <nap/entity.h>

using namespace std;

RTTI_DEFINE(nap::Operator)

namespace nap
{

	nap::Patch* Operator::getPatch()
	{
		return static_cast<Patch*>(getParentObject());
	}

    
    Entity* Operator::getEntity()
    {
        Object* parent = getParentObject();
        while (parent)
        {
            if (parent->getTypeInfo().isKindOf<Entity>())
                return static_cast<Entity*>(parent);
            else
                parent = parent->getParentObject();
        }
        return nullptr;            
    }
    
}
