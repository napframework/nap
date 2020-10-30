/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

#include <rtti/object.h>
#include <utility/dllexport.h>

namespace nap
{
	/**
	 * A resource is a small, stand-alone building block that can be authored in JSON.
	 * Resources are used to load an image from disk, define a three-dimensional shape, create a render window, etc. 
	 * Derive from this class to create your own resource and implement the init() 
	 * call to initialize the object after de-serialization. Override the onDestroy() method
	 * to perform additional clean-up steps. 
	 */
	class NAPAPI Resource : public rtti::Object
	{
		RTTI_ENABLE(rtti::Object)
	public:
		Resource();

		/**
		 * Override this method to initialize the resource after de-serialization.
		 * When called it is safe to assume that all dependencies have been resolved up to this point.
		 * @param errorState should contain the error message when initialization fails
		 * @return if initialization succeeded or failed
		 */
		using rtti::Object::init;

		/**
		 * This function is called on destruction and is only invoked after a successful call to init().
		 * If initialization fails onDestroy will not be invoked. Objects are destroyed in reverse init order.
		 * It is safe to assume that when onDestroy is called all your pointers are valid.
		 * This function is also called when editing JSON files. If during the real-time edit stage an error occurs,
		 * every object that initialized successfully will be destroyed in the correct order.
		 */
		using rtti::Object::onDestroy;
	};
}
