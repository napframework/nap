#include "androidextension.h"

RTTI_DEFINE_BASE(nap::AndroidExtension)

namespace nap
{

	AndroidExtension::AndroidExtension(AAssetManager* assetManager, std::string nativeLibDir) : CoreExtension()
	{
		mAndroidAssetManager = assetManager;
		mAndroidNativeLibDir = nativeLibDir;
	}


	AAssetManager* AndroidExtension::getAssetManager() const
	{
		return mAndroidAssetManager;
	}


	const std::string& AndroidExtension::getNativeLibDir() const
	{
		return mAndroidNativeLibDir;
	}

}