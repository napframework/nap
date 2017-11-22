#include "xformsmoothcomponent.h"

// External Includes
#include "entity.h"
#include <transformcomponent.h>

// nap::xformsmoothcomponent run time class definition 
RTTI_BEGIN_CLASS(nap::XformSmoothComponent)
	RTTI_PROPERTY("TransformSmoothTime",	&nap::XformSmoothComponent::mBlendSpeed,		nap::rtti::EPropertyMetaData::Required)
	RTTI_PROPERTY("ScaleSmoothTime",		&nap::XformSmoothComponent::mScaleBlendSpeed,	nap::rtti::EPropertyMetaData::Required)
RTTI_END_CLASS

// nap::xformsmoothcomponentInstance run time class definition 
RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::XformSmoothComponentInstance)
	RTTI_CONSTRUCTOR(nap::EntityInstance&, nap::Component&)
RTTI_END_CLASS

//////////////////////////////////////////////////////////////////////////


namespace nap
{
	void XformSmoothComponent::getDependentComponents(std::vector<rtti::TypeInfo>& components) const
	{
		components.emplace_back(RTTI_OF(TransformComponent));
	}


	bool XformSmoothComponentInstance::init(utility::ErrorState& errorState)
	{
		mTransform = getEntityInstance()->findComponent<TransformComponentInstance>();
		if (!errorState.check(mTransform != nullptr, "Missing transform component"))
			return false;

		// Set start value for smoother
		mXformSmoother.setValue(mTransform->getTranslate());
		setTarget(mTransform->getTranslate());

		mScaleSmoother.setValue(mTransform->getUniformScale());
		setTargetScale(mTransform->getUniformScale());

		// Now copy settings
		mXformSmoother.mSmoothTime = getComponent<XformSmoothComponent>()->mBlendSpeed;
		mScaleSmoother.mSmoothTime = getComponent<XformSmoothComponent>()->mScaleBlendSpeed;

		return true;
	}


	void XformSmoothComponentInstance::update(double deltaTime)
	{
		mXformSmoother.update(mTargetValue, deltaTime);
		mTransform->setTranslate(mXformSmoother.getValue());

		mScaleSmoother.update(mTargetScale, deltaTime);
		mTransform->setUniformScale(mScaleSmoother.getValue());
	}

}