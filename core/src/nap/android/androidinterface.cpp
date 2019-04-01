#include "androidinterface.h"

RTTI_DEFINE_BASE(nap::AndroidInterface)

namespace nap
{

	AndroidInterface::AndroidInterface(AAssetManager* assetManager, std::string nativeLibDir)
	{
		mAndroidAssetManager = assetManager;
		mAndroidNativeLibDir = nativeLibDir;
	}


	AAssetManager* AndroidInterface::getAssetManager() const
	{
		return mAndroidAssetManager;
	}


	const std::string& AndroidInterface::getNativeLibDir() const
	{
		return mAndroidNativeLibDir;
	}

}