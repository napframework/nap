/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

// Local Includes
#include "androidextension.h"

// External Includes
#include <android/asset_manager_jni.h>

RTTI_DEFINE_BASE(nap::AndroidExtension)

namespace nap
{

	AndroidExtension::AndroidExtension(JNIEnv *jniEnv, jobject androidContextObject) : CoreExtension()
	{
		// Get Java VM
		jniEnv->GetJavaVM(&mAndroidJvm);

		// Get global reference to our activity/service
		mAndroidGlobalObject = reinterpret_cast<jobject>(jniEnv->NewGlobalRef(androidContextObject));

		// Extract android environment
		mAndroidEnv = extractEnv(mAndroidJvm);
		assert(mAndroidEnv != nullptr);

		// Extract asset manager
		mAndroidAssetManager = extractAssetManager(mAndroidEnv);

		// Extract native library dir
		mAndroidNativeLibDir = extractNativeLibDir(mAndroidEnv);

		// Extract application file dir
		mAppInternalFilesDir = extractAppInternalFilesDir(mAndroidEnv);
	}


	AAssetManager* AndroidExtension::getAssetManager() const
	{
		return mAndroidAssetManager;
	}


	const std::string& AndroidExtension::getNativeLibDir() const
	{
		return mAndroidNativeLibDir;
	}


	const std::string& AndroidExtension::getInternalFilesDir() const
	{
		return mAppInternalFilesDir;
	}


	JNIEnv* AndroidExtension::getEnv() const
	{
		return mAndroidEnv;
	}


	jobject AndroidExtension::getContext() const
	{
		return mAndroidGlobalObject;
	}


	JNIEnv* AndroidExtension::extractEnv(JavaVM* virtualMachine)
	{
		// Get environment from java virtual machine
		JNIEnv* env = nullptr;
		int env_stat = virtualMachine->GetEnv((void **)&env, JNI_VERSION_1_6);
		if (env_stat == JNI_OK)
			return env;

		switch (env_stat)
		{
		case JNI_EVERSION:
			Logger::error("Android environment: version not supported");
			break;
		case JNI_EDETACHED:
			Logger::error("Android environment: not attached");
			break;
		default:
			Logger::error("Android environment failure: %d", env_stat);
			break;
		}
		return nullptr;
	}


	AAssetManager* AndroidExtension::extractAssetManager(JNIEnv* env)
	{
		// Get Context Class descriptor
		jclass contextClass = env->FindClass("android/content/Context");

		// Get methodId from Context class
		jmethodID getResourcesMethodId = env->GetMethodID(contextClass, "getResources", "()Landroid/content/res/Resources;");

		// Call method on Context object which is passed in
		jobject resourcesObject = env->CallObjectMethod(mAndroidGlobalObject, getResourcesMethodId);

		// Get Resources class descriptor
		jclass resourcesClass = env->FindClass("android/content/res/Resources");

		// Get methodId from Resources class
		jmethodID getAssetsMethodId = env->GetMethodID(resourcesClass, "getAssets", "()Landroid/content/res/AssetManager;");

		// Call method on Context object which is passed in
		jobject assetManagerObject = env->CallObjectMethod(resourcesObject, getAssetsMethodId);

		return AAssetManager_fromJava(env, assetManagerObject);
	}


	std::string AndroidExtension::extractNativeLibDir(JNIEnv* env)
	{		
		// Get Context Class descriptor
		jclass contextClass = env->FindClass("android/content/Context");

		// Get methodId from Context class
		jmethodID getApplicationInfoMethodId = env->GetMethodID(contextClass, "getApplicationInfo", "()Landroid/content/pm/ApplicationInfo;");

		// Call method on Context object which is passed in
		jobject applicationInfoObject = env->CallObjectMethod(mAndroidGlobalObject, getApplicationInfoMethodId);

		// Get ApplicationInfo class descriptor
		jclass applicationInfoClass = env->FindClass("android/content/pm/ApplicationInfo");

		// Get nativeLibraryDir field on ApplicationInfo
		jfieldID nativeLibDirField = env->GetFieldID(applicationInfoClass, "nativeLibraryDir", "Ljava/lang/String;");

		// Fetch the field string from the object instance
		jstring nativeLibDirJStr = (jstring)env->GetObjectField(applicationInfoObject, nativeLibDirField);

		// Convert to C++ string
		const char* nativeLibDirChr = env->GetStringUTFChars(nativeLibDirJStr, 0);

		// Temp. store, release and return
		std::string s = nativeLibDirChr;
		env->ReleaseStringUTFChars(nativeLibDirJStr, nativeLibDirChr);
		return s;
	}


	std::string AndroidExtension::extractAppInternalFilesDir(JNIEnv* env)
	{
		// Get Context Class descriptor
		jclass contextClass = env->FindClass("android/content/Context");

		// Get methodId from Context class
		jmethodID getFilesDirMethodId = env->GetMethodID(contextClass, "getFilesDir", "()Ljava/io/File;");

		// Call method on Context object which is passed in
		jobject fileObject = env->CallObjectMethod(mAndroidGlobalObject, getFilesDirMethodId);

		// Get File class descriptor
		jclass fileClass = env->FindClass("java/io/File");

		// Get methodId from File class
		jmethodID getAbsolutePathMethodId = env->GetMethodID(fileClass, "getAbsolutePath", "()Ljava/lang/String;");

		// Call method on File object which is passed in
		jstring absolutePathJStr = (jstring)env->CallObjectMethod(fileObject, getAbsolutePathMethodId);

		// Convert to C++ string
		const char* absolutePathChr = env->GetStringUTFChars(absolutePathJStr, 0);

		// Temp. store, release and return
		std::string s = absolutePathChr;
		env->ReleaseStringUTFChars(absolutePathJStr, absolutePathChr);
		return s;
	}
}