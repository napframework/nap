#include "link.h"

namespace nap
{

	Link::Link(Object& parent) : Object() { setParent(parent); }

    
	const ObjectPath& Link::getPath()
	{
		if (mTarget) mObjectPath = mTarget;
		return mObjectPath;
	}

    
	void Link::setTarget(Object& target)
	{
		if (mTarget) {
			mTarget->removed.disconnect(mTargetRemoved);
		}
		mTarget = &target;
		mTarget->removed.connect(mTargetRemoved);
	}


	void Link::setTargetType(RTTI::TypeInfo info) { mTargetType = info; }


	void Link::resolve() const
	{
		mTarget = mObjectPath.resolve(*getParentObject()->getRootObject());
	}


	Object* Link::getTarget() const
	{
		if (!isLinked()) return nullptr;

		if (mTarget) return mTarget;

		resolve();

		return mTarget;
	}


	bool Link::isLinked() const
	{
		if (mTarget) return true;
		if (!mObjectPath.isEmpty()) return true;
		return false;
	}


	void Link::clear()
	{
		mTarget = nullptr;
		mObjectPath.clear();
	}

    
    void Link::setTarget(const std::string& path) {
        mObjectPath = path;
    }
}

RTTI_DEFINE(nap::Link)