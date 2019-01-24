#include "emographyandroidapp.h"
#include "androidwrapper.h"

#include <nap/core.h>
#include <nap/logger.h>
#include <utility/errorstate.h>
#include <android/androidservicerunner.h>
#include <apiservice.h>

namespace nap
{
    namespace android
    {
        ///////////////////////////////////////////////////////
        // Application Instance
        ///////////////////////////////////////////////////////

        /**
         * Thread safe emography nap application instance
         * For now implemented as a singleton, could be instantiated if required in the future
         * This is the case when multiple apps share this context?
         */
        class Instance final
        {
        public:
            /**
             * @return the global instance of this app.
             */
            static Instance& get()
            {
                // Return if instance was created already
                static Instance sInstance;
                return sInstance;
            }

            /**
             * Creates the instance, only allowed if there is no instance created already.
             * @return if creation succeeded or not
             */
            bool init(JNIEnv *env, jobject contextObject)
            {
                if (mRunner != nullptr)
                {
                    nap::Logger::warn("instance of emography nap environment already exists!");
                    return false;
                }

                // Create core, replacing previous instance if allocated already.
                mCore   = std::make_unique<nap::Core>();
                mRunner = std::make_unique<EmographyRunner>(*mCore, env, contextObject);

                // Start
                nap::utility::ErrorState error;
                if (!mRunner->init(error))
                {
                    nap::Logger::fatal("error: %s", error.toString().c_str());
                    return false;
                }

                mAPIService = mCore->getService<nap::APIService>();
                if(mAPIService == nullptr)
                {
                    nap::Logger::fatal("error: %s", "unable to acquire handle to API service");
                    return false;
                }
                return true;
            }

            /**
             * Shutdown the emography app environment
             * @return application exit code
             */
            int shutDown()
            {
                if(mRunner == nullptr)
                {
                    nap::Logger::warn("unable to shutdown nap env, not initialized");
                    return -1;
                }

                // Call shutdown reset ptr
                int v = mRunner->shutdown();
                mRunner.reset(nullptr);
                return v;
            }


            /**
             * Update the emography app environment
             */
            void update()
            {
                if(mRunner == nullptr)
                    nap::Logger::warn("unable to update nap env, not initialized");
                mRunner->update();
            }

            /**
             * @return the application runner, nullptr if not initialized
             */
            EmographyRunner* runner()
            {
                if(mRunner == nullptr)
                    nap::Logger::warn("unable to acquire app runner, not initialized");
                return mRunner.get();
            }

            /**
             * Destructor
             */
            ~Instance()
            {
                mRunner.reset(nullptr);
                mCore.reset(nullptr);
            }


        private:
            // Hide constructor
            Instance()                                  {   }

            // Members
            nap::APIService* mAPIService                = nullptr;
            std::unique_ptr<nap::Core> mCore            = nullptr;
            std::unique_ptr<EmographyRunner> mRunner    = nullptr;
        };


        ///////////////////////////////////////////////////////
        // C calls
        ///////////////////////////////////////////////////////

        bool init(JNIEnv *env, jobject contextObject)
        {
            return Instance::get().init(env, contextObject);
        }


        void update(JNIEnv* env, jobject contextObject)
        {
            Instance::get().update();
        }


        void shutdown(JNIEnv* env, jobject contextObject)
        {
            Instance::get().shutDown();
        }


        void sendMessage(JNIEnv* env, jobject contextObject, jstring jdata)
        {
            EmographyRunner* runner = Instance::get().runner();
            if(runner == nullptr)
                return;

            const char *s = env->GetStringUTFChars(jdata, NULL);
            std::string data = s;
            env->ReleaseStringUTFChars(jdata, s);
            runner->getApp().call(data);
        }


        jstring pullLogFromApp(JNIEnv* env, jobject contextObject)
        {
            EmographyRunner* runner = Instance::get().runner();
            if(runner == nullptr)
                return nullptr;
            return env->NewStringUTF(runner->getApp().pullLogAndFlush().c_str());
        }
    }
}
