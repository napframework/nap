#pragma once

// External Includes
#include <nap/coreinterface.h>
#include <utility/dllexport.h>
#include <rtti/typeinfo.h>
#include <android/asset_manager.h>

namespace nap
{
	/**
	 * Interface required by Android specific applications.
	 * Provides access to the android asset manager and shared library directory.
	 * The AssetManager is used to load assets bundled included with the Android APK.
	 */
	class NAPAPI AndroidInterface : public CoreInterface
	{
		RTTI_ENABLE(CoreInterface)
	public:
		AndroidInterface() = delete;

		/**
		 * Constructor
		 * @param assetManager the android asset manager
		 * @param naiveLibDir the android native library directory
		 */
		AndroidInterface(AAssetManager* assetManager, std::string nativeLibDir);

		/**
		 * @return the android asset manager
		 */
		AAssetManager* getAssetManager() const;

		/**
		 * @return the android library directory	
		 */
		const std::string& getNativeLibDir() const;

	private:
		// The AssetManager is used to load assets bundled included with the Android APK. Bundled assets with the app
		// aren't files on disk and need to be accessed via this mechanism. Currently used by the ResourceManager to 
		// load the project structure JSON and the ProjectInfoManager for the.. project info but may likely be useful 
		// elsewhere later.
		AAssetManager* mAndroidAssetManager;

		// The location of the shared libraries for the Android app on disk. Used by the ModuleManager to load the 
		// module shared libs.
		std::string mAndroidNativeLibDir;
	};
}