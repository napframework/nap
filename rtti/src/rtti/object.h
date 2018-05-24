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
			 * Override this method to initialize the object after de-serialization
			 * @param errorState should contain the error message when initialization fails
			 * @return if initialization succeeded or failed
			 */
			virtual bool init(utility::ErrorState& errorState)	{ return true; }

			/**
			 * @return if this is an object that holds a valid identifier attribute
			 */
			static bool isIDProperty(rtti::Instance& object, const rtti::Property& property);

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

			std::string mID;		///< Property: 'mID' name of the object. Used as an identifier by the system
		};
	}
}
