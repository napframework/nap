// Local Includes
#include "androidapp.h"

// External Includes
#include <nap/logger.h>
#include <android/asset_manager_jni.h>

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::AndroidApp)
    RTTI_CONSTRUCTOR(nap::Core&)
RTTI_END_CLASS

namespace nap
{
    AndroidApp::AndroidApp(Core& core) : App(core)
    {
    }


    void AndroidApp::populateAndroidVars(JNIEnv *jniEnv, jobject androidContextObject)
    {
        // Get Java VM
        jniEnv->GetJavaVM(&mAndroidJvm);

        // Get global reference to our activity/service
        // TODO Cleanup, releasing
        mAndroidGlobalObject = reinterpret_cast<jobject>(jniEnv->NewGlobalRef(androidContextObject));

        // Get the AssetManager and shared lib dir and set them in core
        getCore().setAndroidInitialisationVars(getAssetManager(), getNativeLibDir());
    }


    JNIEnv* AndroidApp::getEnv()
    {
        JNIEnv* env = nullptr;
        // JNI 1.6 is supported back to very early versions of Android, pre v2.0
        int getEnvStat = mAndroidJvm->GetEnv((void **) &env, JNI_VERSION_1_6);
        if (getEnvStat == JNI_OK)
            return env;

        switch(getEnvStat) 
        {
            case JNI_EVERSION:
                Logger::error("GetEnv: version not supported");
                break;
            case JNI_EDETACHED:
                Logger::error("GetEnv: not attached");
                break;
            default:
                Logger::error("GetEnv failure: %d", getEnvStat);
        }
        return nullptr;
    }


    std::string AndroidApp::getNativeLibDir()
    {
        // Get JNI env
        JNIEnv *env = getEnv();

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
        jstring nativeLibDirJStr = (jstring) env->GetObjectField(applicationInfoObject, nativeLibDirField);

        // Convert to C++ string
        const char* nativeLibDirChr = env->GetStringUTFChars(nativeLibDirJStr, 0);

        // Temp. store, release and return
        std::string s = nativeLibDirChr;
        env->ReleaseStringUTFChars(nativeLibDirJStr, nativeLibDirChr);
        return s;
    }


    AAssetManager* AndroidApp::getAssetManager()
    {
        // Get JNI env
        JNIEnv *env = getEnv();

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
}
