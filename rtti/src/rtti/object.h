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
		static const char* sIDPropertyName = "mID";

		/**
		 * The base class for all top-level objects that need to support serialization / de-serialization.
		 * You only need to derive from this if your object should be serialized to the root of the document or needs to be
		 * able to be pointed to from other objects. If you're making, for example, a compound (i.e a plain struct) 
		 * there is no need to derive from this class.
		 */
		class NAPAPI Object
		{
			RTTI_ENABLE()
		public:
			// Construction / Destruction
			Object();
			virtual ~Object();

			/**
			 * Override this method to initialize the object after de-serialization.
			 * When called it is safe to assume that all dependencies have been resolved up to this point.
			 * @param errorState should contain the error message when initialization fails
			 * @return if initialization succeeded or failed
			 */
			virtual bool init(utility::ErrorState& errorState)	{ return true; }

			/**
			 * This function is called on destruction and is only invoked after a previous call to init().
			 * It is called in reverse init order. It is therefore safe to assume that all your pointers are still valid.
			 * In addition, this function is also called during real-time editing of JSON files if there was an error: 
			 * any objects that were successfully initialized before the failure will have onDestroy called on them in the correct order.
			 */
			virtual void onDestroy() {}

			/**
			 * @return if this is an object that holds a valid identifier attribute
			 */
			static bool isIDProperty(rtti::Instance& object, const rtti::Property& property);

			/**
			 * Enables the use of ObjectPtrs for this Object. This is normally only used within the de-serialization process for you,
			 * but in case you're not de-serializing, you need to set the use of ObjectPtrs to false when you are creating objects
			 * manually on threads other than the main threads. Disabling it will make sure that no global access to the ObjectPtrManager
			 * is performed.
			 */
			void setEnableObjectPtrs(bool enable) { mEnableObjectPtrs = enable; }

			/**
			 * Copy is not allowed
			 */
			Object(Object&) = delete;
			Object& operator=(const Object&) = delete;

			/**
			 * Move is not allowed
			 */
			Object(Object&&) = delete;
			Object& operator=(Object&&) = delete;

			std::string mID;							///< Property: 'mID' name of the object. Used as an identifier by the system
			bool		mEnableObjectPtrs = true;		///< Property: 'mEnableObjectPtrs' Enables/disables the ability to use ObjectPtrs to point to this Object
		};
	}
}
