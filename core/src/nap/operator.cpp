
#include <algorithm>
#include <nap/configure.h>
#include <nap/core.h>
#include <nap/operator.h>
#include <nap/patch.h>

using namespace std;

RTTI_DEFINE(nap::Operator)

namespace nap
{

	nap::Patch* Operator::getPatch()
	{
		return static_cast<Patch*>(getParentObject());
	}

    
    
}
