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
		 *  Base class of all top-level objects that support serialization / de-serialization.
		 *
		 * Derive from this object if your object:
		 *     - needs to be serialized to the root of the document.
		 *     - needs to be able to be pointed to from other objects.
		 *
		 * If you're making, for example, a compound (a plain struct)
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
			 * This function is called on destruction and is only invoked after a successful call to init(). 
			 * If initialization fails onDestroy will not be invoked. Objects are destroyed in reverse init order. 
			 * It is safe to assume that when onDestroy is called all your pointers are valid. 
			 * This function is also called when editing JSON files. If during the real-time edit stage an error occurs,
			 * every object that initialized successfully will be destroyed in the correct order.
			 */
			virtual void onDestroy() {}

			/**
			 * @return if this is an object that holds a valid identifier attribute
			 */
			static bool isIDProperty(rtti::Instance& object, const rtti::Property& property);

			/**
			 * Enables the use of ObjectPtrs for this Object. This is normally automatically set during the de-serialization process for you.
			 * But in rare cases that does not involve a de-serialization step, for example when creating an object on the stack, 
			 * it is recommended to set the use of ObjectPtrs to false. Disabling the use of object pointers ensures that no global access to the 
			 * ObjectPtrManager is performed and therefore a potential race-condition is introduced. 
			 * This is a temporary work-around that will be removed in a future release of NAP.
			 * @param enable if ObjectPtrs should be used for this object. 
			 */
			void setEnableObjectPtrs(bool enable);

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
