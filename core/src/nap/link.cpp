#include "link.h"
#include "logger.h"

namespace nap
{

	Link::Link(Object& parent) : Object() 
	{ 
		setParent(parent); 
	}

    
	Link::Link(Object& parent, const RTTI::TypeInfo& type) : mTargetType(type)
	{
		setParent(parent);
	}


	// Returns the target path
	// If a target is resolved, return that path
	// Otherwise thep path that is currently set but not resolved
	const ObjectPath& Link::getPath()
	{
		if (mTarget) 
			mObjectPath = mTarget;
		return mObjectPath;
	}


	// Sets the new link target
	void Link::setTarget(Object& target)
	{
		// Don't do anything when target is the same
		if (&target == mTarget)
			return;

		// Disconnect from current target if target 
		// is associated with link
		if (mTarget)
		{
			mTarget->removed.disconnect(mTargetRemoved);
		}

		Object* current_target = mTarget;
		mTarget = &target;
		mTarget->removed.connect(mTargetRemoved);
		
		// Trigger change
		targetChanged.trigger(*this);
	}


	// Resolved the link and set the new target based on it's result
	// If the actual target changed emit a signal
	void Link::resolve() const
	{
		Object* current_target = mTarget;
		
		mTarget = mObjectPath.resolve(*getParentObject()->getRootObject());
		if (mTarget == nullptr)
		{
			nap::Logger::warn("unable to resolve target object from object path: %s", mObjectPath.toString().c_str());
		}

		// Trigger change
		if (mTarget != current_target)
		{
			targetChanged.trigger(*this);
		}
	}


	Object* Link::getTarget() const
	{
		// return if it's not linked currently (ie, no path and object)
		if (!isLinked()) 
			return nullptr;

		// if there's an object associated with this link return that one
		if (isValid()) 
			return mTarget;

		resolve();

		return mTarget;
	}


	// Returns if the link currently points to an object
	// An object is linked when either a target is specicified or a path is given
	bool Link::isLinked() const
	{
		if (mTarget != nullptr || !mObjectPath.isEmpty())
			return true;
		return false;
	}


	// Clear existing link, if an object was associated with the link
	// notify possible listeners of change
	void Link::clear()
	{
		bool had_link = isValid();
		mTarget = nullptr;
		if (had_link)
		{
			targetChanged.trigger(*this);
		}
		mObjectPath.clear();
	}

    
	// Set new target path without resolving
    void Link::setTarget(const std::string& path) 
	{
        mObjectPath = path;
    }
}

RTTI_DEFINE(nap::Link)