#pragma once

#include "object.h"
#include "objectpath.h"
#include "signalslot.h"
#include <rtti/rtti.h>

namespace nap
{

	//! A link to a object. Contains a pointer and can be converted to a path.
	class Link : public Object
	{
		RTTI_ENABLE_DERIVED_FROM(Object)
	public:
		Link(Object& parent);
		Link() : Object() {}

		void clear();

		Object* getTarget() const;

		template <typename T>
		T* getTarget() const
		{
			return dynamic_cast<T*>(getTarget());
		}

		void setTarget(Object& target);
		void setTarget(const std::string& path);

		const ObjectPath& getPath();

		RTTI::TypeInfo getType() { return mTargetType; }

		bool isLinked() const;

		//! Returns whether the link is still valid
		bool isValid() const { return mTarget != nullptr; }

		void setTargetType(RTTI::TypeInfo info);
		RTTI::TypeInfo getTargetType() const { return mTargetType; }


	protected:
		RTTI::TypeInfo mTargetType = RTTI::TypeInfo::empty();

	private:
		void resolve() const;

		//! Pointer managed by the link object
		mutable Object* mTarget = nullptr;
		ObjectPath mObjectPath;

		//! Slot called by the object on removal. Sets the link to invalid, store a path
		Slot<Object&> mTargetRemoved = {[&](Object& obj) {
			mObjectPath = obj; // Store a path
			mTarget = nullptr;
		}};
	};

	template <typename T>
	class TypedLink : public Link
	{
	public:
		TypedLink(T& target) : Link(target) { mTargetType = RTTI_OF(T); }

		T* getTypedTarget() { return static_cast<T*>(getTarget()); }
	};
}

RTTI_DECLARE(nap::Link)
