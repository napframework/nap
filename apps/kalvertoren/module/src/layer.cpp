#include "layer.h"

// nap::layer run time class definition
RTTI_DEFINE_BASE(nap::Layer)

namespace nap
{
	Layer::~Layer() { }


	bool Layer::init(utility::ErrorState& errorState)
	{
		return true;
	}
}