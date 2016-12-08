#include "link.h"
#include "logger.h"

namespace nap
{
	// Initialize link with basic target type of type object
	Link::Link(Object& parent) : Object() 
	{ 
		setParent(parent); 
		mTargetType = RTTI_OF(nap::Object);
	}


    // Initialize link with specific target type
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

		// Make sure new target matches type
		if (!target.getTypeInfo().isKindOf(mTargetType))
		{
			nap::Logger::warn("invalid link, object: %s not of type: %s", target.getName().c_str(), mTargetType.getName().c_str());
			return;
		}

		// Disconnect from current target if target 
		// is associated with link
		if (mTarget)
		{
			mTarget->removed.disconnect(mTargetRemoved);
		}

		// Set and connect to new target
		mTarget = &target;
		mTarget->removed.connect(mTargetRemoved);
		
		// Trigger change
		targetChanged.trigger(*this);
	}


	// Resolved the link and set the new target based on it's result
	// If the actual target changed emit a signal
	void Link::resolve()
	{
		// Get new target
		nap::Object* link_target = mObjectPath.resolve(*getParentObject()->getRootObject());
		if (link_target == nullptr)
		{
			nap::Logger::warn("unable to resolve target object from object path: %s", mObjectPath.toString().c_str());
			return;
		}

		// Make sure types match
		if(!(link_target->getTypeInfo().isKindOf(mTargetType)))
		{
			nap::Logger::warn("invalid link, object not of type: %s", mTargetType.getName().c_str());
			return;
		}

		setTarget(*link_target);
	}


	// Returns link target, nullptr if not linked or path can't be resolved
	Object* Link::getTarget()
	{
		// return if it's not linked currently (ie, no path and object)
		if (!isLinked()) 
			return nullptr;

		// if there's an object associated with this link return that one
		if (isResolved())
			return mTarget;

		// Otherwise resolve linnk
		resolve();

		return mTarget;
	}


	// Returns if the link currently points to an object
	// An object is linked when either a target is specicified or a path is given
	bool Link::isLinked() const
	{
		return (mTarget != nullptr || !mObjectPath.isEmpty());
	}


	// Clear existing link, if an object was associated with the link
	// notify possible listeners of change
	void Link::clear()
	{
		bool had_link = isResolved();
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