#pragma once

// Local Includes
#include "object.h"
#include "objectpath.h"
#include "signalslot.h"

namespace nap
{
	/**
	 * Link to an external object that can be resolved from an object path
	 * Use this object to connect two objects in the system tree
	 * When constructing a link using a path the object is automatically resolved on query
	 * If the object isn't available a nullptr will be returned
	 */
	class Link : public Object
	{
		RTTI_ENABLE(Object)
	public:
		// Constructor
		Link(Object& parent);
		Link(Object& parent, const RTTI::TypeInfo&  type);
		Link() : Object() {}

		/**
		 * Clears the link, breaking the connection to the target object if one exists
		 */
		void clear();

		/**
		 * Returns the object this link points to, nullptr if link is invalid or not set
		 */
		Object* getTarget();

		/**
		 * Returns the object this link points to as object of type T
		 * Note that this performs an RTTI cast, if the requested type does not
		 * is not derived from the target type the result will be a null ptr
		 */
		template <typename T>
		T* getTarget() 										{ return rtti_cast<T>(getTarget()); }

		/**
		 * Set the link to point @target
		 * @param target the object to link to
		 */
		void setTarget(Object& target);

		/**
		 * Set the link to point at the object described by path
		 * @param path the path to the object to link to
		 */
		void setTarget(const std::string& path);

		/**
		 * @return the path of the linked object
		 */
		const ObjectPath& getPath();

		/**
		 * @return object type of the link
		 * only objects derived from the target type are eligible as link targets
		 */
		RTTI::TypeInfo getType()								{ return mTargetType; }

		/**
		 * @return true if the link points to an object or a link path is specified
		 */
		bool isLinked() const;

		/**
		 * @return if the link points to an object
		 */
		bool isResolved() const									{ return mTarget != nullptr; }

		/**
		 * specifies the object target type to link to
		 * @param info the type information of the object this link points to
		 */
		void setTargetType(RTTI::TypeInfo info)					{ mTargetType = info; }

		/**
		 * @return the type of the object this link is allowed to point to
		 */
		RTTI::TypeInfo getTargetType() const					{ return mTargetType; }

		/**
		 * emitted when the target object of the link changes
		 */
		mutable Signal<const Link&> targetChanged;

	protected:
		// the link target type
		RTTI::TypeInfo mTargetType = RTTI::TypeInfo::empty();

	private:
		/**
		 * Tries to resolve the link based on the current object path
		 * On success the target will be set, otherwise the target will be nullptr
		 */
		void resolve();

		// Pointer to the object this link points to
		mutable Object* mTarget = nullptr;

		// Path to the object this link points to
		ObjectPath mObjectPath;

		//! Slot called by the object on removal. Sets the link to invalid, store a path
		Slot<Object&> mTargetRemoved = {[&](Object& obj) {
			mObjectPath = obj; // Store a path
			mTarget = nullptr;
		}};
	};


	// TODO: DEPRICATE
	template <typename T>
	class TypedLink : public Link
	{
	public:
		TypedLink(Object& parent) : Link(parent) { mTargetType = RTTI_OF(T); }

		T* getTypedTarget() { return static_cast<T*>(getTarget()); }
	};
}
