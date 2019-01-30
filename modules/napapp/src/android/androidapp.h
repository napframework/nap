#pragma once

#include "app.h"

// External includes
#include <rtti/typeinfo.h>
#include <inputevent.h>
#include <utility/errorstate.h>
#include <nap/core.h>
#include <nap/windowevent.h>

#include <jni.h>

namespace nap
{
    class NAPAPI AndroidApp : public App
    {
       RTTI_ENABLE(App)
    public:
        AndroidApp(Core& core);

        /**
        * Populate Android variables. JVM and Context object into the app, and the AssetManager and shared libs directory into core
        * @param jniEnv The JNI environment
        * @param androidContextObject The Android Context object. Typically the parent activity or service.
        */
        void populateAndroidVars(JNIEnv *jniEnv, jobject androidContextObject);

        /**
         * Get the JNI environment from our Java VM
         * @return The java native environment
         */
        JNIEnv* getEnv();

		/**
		 * Get the context back in Android space, typically the activity or service
		 * @return The context back in Android space
		 */
		jobject getContext();

		/**
		 * @return the android asset manager, which provides access to an application's raw assets
		 */
		AAssetManager* getAssetManager();

    protected:
        std::string getNativeLibDir();

        /**
         * @return The internal storage directory for the app
         */
        std::string getAppInternalFilesDir();

        JavaVM*	mAndroidJvm; 						// The JNI environment
        jobject	mAndroidGlobalObject;				// Our context object back in Android space, typically the activity or service
    };
}
