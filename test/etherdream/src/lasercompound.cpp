#include "lasercompound.h"

// nap::lasercompound run time class definition 
RTTI_BEGIN_CLASS(nap::LaserCompound)
	RTTI_PROPERTY("LineMesh",		&nap::LaserCompound::mLineMesh,		nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Tracemesh",		&nap::LaserCompound::mTraceMesh,	nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("NormalMesh",		&nap::LaserCompound::mNormalsMesh,	nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Dac",			&nap::LaserCompound::mDac,			nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("Target",			&nap::LaserCompound::mTarget,		nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("LaserID",		&nap::LaserCompound::mLaserID,		nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	LaserCompound::~LaserCompound()			{ }


	bool LaserCompound::init(utility::ErrorState& errorState)
	{
		return true;
	}
}