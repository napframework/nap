#include <napoftransform.h>
#include <nap/entity.h>
#include <nap/logger.h>

namespace nap
{
	/**
	@brief Constructor, initializes global matrix to be identiy
	**/
	OFTransform::OFTransform()
	{
		mGlobalTransform.makeIdentityMatrix();
	}


	/**
	@brief Composes a matrix out of the individual transform attributes
	**/
	ofMatrix4x4 OFTransform::getLocalTransform() const
	{
		ofMatrix4x4 matrix;
		matrix.makeIdentityMatrix();
		matrix.glTranslate(mTranslate.getValue());
		matrix.glScale(mScale.getValue() * mUniformScale.getValue());
		matrix.glRotate(mRotate.getValue().x, 1.0f, 0.0f, 0.0f);
		matrix.glRotate(mRotate.getValue().y, 0.0f, 1.0f, 0.0f);
		matrix.glRotate(mRotate.getValue().z, 0.0f, 0.0f, 1.0f);
		return matrix;
	}


	// Iterate over all the children and find child transforms
	void OFTransform::fetchChildTransforms()
	{
		// Clear all child transforms
		mChildTransforms.clear();

		// Get ownder
		Entity* parent = getParent();
		if (parent == nullptr)
		{
			nap::Logger::warn("transform has no parent: %s", this->getName().c_str());
			return;
		}

		// Iterate over all entities and add to child transform
		for (auto& child : parent->getEntities())
		{
			OFTransform* child_xform = child->getComponent<OFTransform>();
			if (child_xform == nullptr)
			{
				Logger::debug("child has no transform: %s", child->getName().c_str());
				continue;
			}
			mChildTransforms.emplace_back(child_xform);

			// Now fetch all other child transforms
			child_xform->fetchChildTransforms();
		}
	}


	// Updates it's own global matrix and forwards calls to it's children
	void OFTransform::update(const ofMatrix4x4& inParentMatrix)
	{
		mGlobalTransform = getLocalTransform() * inParentMatrix;
		for (auto& child : mChildTransforms)
		{
			child->update(mGlobalTransform);
		}
	}

	//////////////////////////////////////////////////////////////////////////

	OFRotateComponent::OFRotateComponent()
	{
		mPreviousTime = ofGetElapsedTimef();
		mEnableUpdates.valueChangedSignal.connect(mUpdateCalled);
		mReset.signal.connect(mResetCalled);

	}


	/**
	@brief Update

	Updates local time axis based on the time passed
	**/
	void OFRotateComponent::onUpdate()
	{
		// Get time difference and store
		float current_time = ofGetElapsedTimef();
		float diff_time = current_time - mPreviousTime;
		mPreviousTime = current_time;

		// Store time
		mTimeX += (diff_time * mSpeed.getValue() * mX.getValue());
		mTimeY += (diff_time * mSpeed.getValue() * mY.getValue());
		mTimeZ += (diff_time * mSpeed.getValue() * mZ.getValue());

		// Get component
		OFTransform* xform = mDependency.get();
		if (xform == nullptr)
		{
			Logger::warn("Rotation component can't find transform component");
			return;
		}

		// Set rotation
		xform->mRotate.setValue(ofVec3f(mTimeX, mTimeY, mTimeZ));
	}


	/**
	@brief Resets the entire operator and disables updates
	**/
	void OFRotateComponent::onReset(const SignalAttribute& signal)
	{		
		mTimeX = 0.0f;
		mTimeY = 0.0f;
		mTimeZ = 0.0f;

		nap::OFTransform* xform = mDependency.get();
		if (xform == nullptr)
		{
			Logger::warn("Rotation component can't find transform component");
			return;
		}
		xform->mRotate.setValue(ofVec3f(0.0f, 0.0f, 0.0f));
	}


	/**
	@brief Stores the last time stamp so rotation continues where it left off
	**/
	void OFRotateComponent::onUpdateChanged(const bool& value)
	{
		mPreviousTime = ofGetElapsedTimef();
	}

}

RTTI_DEFINE(nap::OFTransform)
RTTI_DEFINE(nap::OFRotateComponent)
