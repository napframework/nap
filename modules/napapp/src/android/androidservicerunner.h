#pragma once

// Local Includes
#include "app.h"
#include "appeventhandler.h"
#include "servicerunner.h"
#include "androidapp.h"

// External Includes
#include <rtti/typeinfo.h>
#include <nap/core.h>
#include <nap/datetime.h>
#include <thread>

namespace nap
{
	/**
	 * Specialization of the service runner, populates the application with all Android specific objects.	
	 */
    template<typename APP, typename HANDLER>
    class AndroidServiceRunner: public ServiceRunner<APP, HANDLER>
    {
        RTTI_ENABLE(ServiceRunner<APP, HANDLER>)
    public:
        /**
        * Constructor
        * @param core The nap core this runner uses in conjunction with the app and handler
        * @param jniEnv The JNI environment
        * @param androidContextObject The Android Context object. Typically the parent activity or service.
        */
        AndroidServiceRunner(nap::Core& core, JNIEnv *jniEnv, jobject androidContextObject);
    };


    //////////////////////////////////////////////////////////////////////////
    // Template definitions
    //////////////////////////////////////////////////////////////////////////

    template<typename APP, typename HANDLER>
    nap::AndroidServiceRunner<APP, HANDLER>::AndroidServiceRunner(nap::Core& core, JNIEnv *jniEnv, jobject androidContextObject) : ServiceRunner<APP, HANDLER>(core)
    {	
        nap::AndroidApp& app = ServiceRunner<APP, HANDLER>::getApp();
        app.populateAndroidVars(jniEnv, androidContextObject);
    }
}
