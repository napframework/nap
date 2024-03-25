/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Local Includes
#include "rtti.h"

// External Includes
#include <utility/errorstate.h>
#include <utility/dllexport.h>

namespace nap
{
	namespace rtti
	{
		constexpr const char* sIDPropertyName = "mID";

        /**
         * Base class of all top-level objects that support serialization / de-serialization.
         *
         * Derive from this object if your object:
         *     - needs to be serialized to the root of the document.
         *     - needs to be able to be pointed to from other objects.
         *
		 * If you're making, for example, a compound (i.e a plain struct)
		 * there is no need to derive from this class.
		 */
		class NAPAPI Object
		{
			RTTI_ENABLE()
		public:
			// Construction
			Object();

			// Destruction
			virtual ~Object();

			/**
			 * Override this method to initialize the object after de-serialization.
			 * When called it is safe to assume that all dependencies have been resolved up to this point.
			 * @param errorState should contain the error message when initialization fails.
			 * @return if initialization succeeded or failed.
			 */
			virtual bool init(utility::ErrorState& errorState) { return true; }

			/**
			 * This function is called on destruction and is only invoked after a successful call to init().
			 * If initialization fails onDestroy will not be invoked. Objects are destroyed in reverse init order.
			 * It is safe to assume that when onDestroy is called all your pointers are valid.
			 * This function is also called when editing JSON files. If during the real-time edit stage an error occurs,
			 * every object that initialized successfully will be destroyed in the correct order.
			 */
			virtual void onDestroy() { };

			/**
			 * @return if this is an object that holds a valid identifier attribute
			 */
			static bool isIDProperty(rtti::Instance& object, const rtti::Property& property);

			/**
			 * Copy is not allowed
			 */
			Object(Object&) = delete;

			/**
			 * Copy assignment is not allowed
			 */
			Object& operator=(const Object&) = delete;

			/**
			 * Move is not allowed
			 */
			Object(Object&&) = delete;

			/**
			 * Move assignment is not allowed
			 */
			Object& operator=(Object&&) = delete;

			std::string	mID;				///< Property: 'mID' unique name of the object. Used as an identifier by the system

		private:
			friend class ObjectPtrBase;
			template<class T> friend class ObjectPtr;

			inline void incrementObjectPtrRefCount() { mObjectPtrRefCount++; }
			inline void decrementObjectPtrRefCount() { mObjectPtrRefCount--; };

			int	mObjectPtrRefCount = 0;		///< The number of ObjectPtrs pointing to this object. Note that this refcount is not multithread-safe: it is still expected that ObjectPtrs are pointing to an Object from the same thread (but it can be any thread).
		};
	}
}
