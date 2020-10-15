/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// External Includes
#include <nap/coreextension.h>
#include <nap/logger.h>
#include <android/asset_manager.h>
#include <jni.h>

namespace nap
{
	/**
	 * Extension required to run Android specific applications. 
	 * Provides access to the android environment and context as given by the JNI.
	 * This extension is used during initialization of the engine to source android specific parameters.
	 * When creating an Android application, make sure to construct Core with an instance of this class.
	 * Failure to do so results in a non functioning Android application.
	 */
	class NAPAPI AndroidExtension : public CoreExtension
	{
		RTTI_ENABLE(CoreExtension)
	public:
		// No default constructor
		AndroidExtension() = delete;

		/**
		 * Constructor
		 * @param jniEnv the java native environment
		 * @param androidContextObject the android context (activity or service)
		 */
		AndroidExtension(JNIEnv *jniEnv, jobject androidContextObject);

		/**
		 * @return the android asset manager
		 */
		AAssetManager* getAssetManager() const;

		/**
		 * @return the android library directory	
		 */
		const std::string& getNativeLibDir() const;

		/**
		 * @return the android internal storage directory	
		 */
		const std::string& getInternalFilesDir() const;

		/**
		 * Get the JNI environment from our Java VM
		 * @return The Java native environment
		 */
		JNIEnv* getEnv() const;

		/**
		 * Get the context back in Android space, typically the activity or service
		 * @return The context back in Android space
		 */
		jobject getContext() const;

	private:
		JavaVM*	mAndroidJvm = nullptr; 				///< The JNI environment
		jobject	mAndroidGlobalObject = nullptr;		///< Our context object back in Android space, typically the activity or service
		JNIEnv* mAndroidEnv = nullptr;				///< The android environment
		
		std::string mAppInternalFilesDir;			///< The application file directory.
		std::string mAndroidNativeLibDir;			///< The location of the shared libraries for the Android app on disk.

		// The AssetManager is used to load assets bundled included with the Android APK. Bundled assets with the app
        // aren't files on disk and need to be accessed via this mechanism. Currently used by the ResourceManager to 
        // load the project structure JSON and the ProjectInfoManager for the.. project info but may likely be useful 
        // elsewhere later.
        AAssetManager* mAndroidAssetManager;

		/**
		 * Extract the java native environment.
		 * @param virtualMachine the java virtual environment.
		 * @return java environment, nullptr if extraction fails	
		 */
		static JNIEnv* extractEnv(JavaVM* virtualMachine);

		/**
		 * Extracts the asset manager	
		 */
		AAssetManager* extractAssetManager(JNIEnv* env);

		/**
		 * Extracts the android native library	
		 */
		std::string extractNativeLibDir(JNIEnv* env);

		/**
		 * Extracts the internal application directory from the app	
		 */
		std::string extractAppInternalFilesDir(JNIEnv* env);
	};
}