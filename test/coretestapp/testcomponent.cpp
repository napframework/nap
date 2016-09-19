#include "testcomponent.h"

RTTI_DEFINE(nap::TransformComponent)
RTTI_DEFINE(nap::Receiver)

/**
@brief Overloaded constructor TransformComponent
**/
nap::TransformComponent::TransformComponent(float inX, float inY, float inZ)
{
	mX.setValue(inX);
	mY.setValue(inY);
	mZ.setValue(inZ);
}



