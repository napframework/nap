#pragma once

// Local Includes
#include <nap/coreattributes.h>
#include "attributeobject.h"

// External Includes
#include <string>

namespace nap
{
	// Forward Declares
	class ResourceManagerService;
	class Core;

	struct InitResult
	{
		std::string mErrorString;

		bool check(bool successCondition, const std::string& errorString)
		{
			if (!successCondition)
				mErrorString = errorString;

			return successCondition;
		}

		template <typename... Args>
		bool check(bool successCondition, const std::string& format, Args&&... args)
		{
			return check(successCondition, stringFormat(format, std::forward<Args>(args)...));
		}
	};

	/**
	* Abstract base class for any Asset. Could be a TextureAsset, ModelAsset or AudioAsset for example.
	* WARNING: A resource may only be created through the ResourceManagerService providing a valid resource path
	*/
	class Resource : public AttributeObject
	{
		RTTI_ENABLE_DERIVED_FROM(AttributeObject)
        friend class ResourceManagerService;
	public:
		Resource() = default;

		virtual bool init(InitResult& initResult) { return true; }	// TODO: pure

		/**
		* @return Human readable string representation of this path
		*/
		virtual const std::string getDisplayName() const = 0;

        /**
         * @return The resource path to this resource
         */
        std::string getResourcePath() const;

		Attribute<std::string> mID = { this, "mID", "" };

    private:
        ResourceManagerService* mResourceManager = nullptr;
	};


	/**
	* Abstract base class for any AssetFactories,
	*/
	class ResourceLoader
	{
		RTTI_ENABLE()
		friend class ResourceManagerService;

	public:
		// Default constructor
		ResourceLoader() = default;
		
		// Default destructor
		virtual ~ResourceLoader() = default;

		/**
		* Check whether this factory can load the provided file.
		* The base implementation will check the path for file extension.
		* This method may be reimplemented to provide different ways of identifying whether the faile may
		* @param assetPath The relative (or absolute URI) path to the asset
		* @return True if the asset can be handled by this factory, false otherwise
		*/
		virtual bool canHandle(const std::string& assetPath) const;

		/**
		* @return The currently registered file extensions
		*/
		std::vector<std::string> getFileExtensions() const { return mFileExtensions; }

		/**
		* Load the asset from disk and return an instance
		* @param assetFilename The relative filename of the asset
		* @return An instance of asset or nullptr if the loading failed
		*/
		virtual std::unique_ptr<Resource> loadResource(const std::string& resourcePath) const = 0;

		/**
		* @param ext The file extension without the dot, such as "jpeg"
		* @return True when the file extension has been found, false otherwise
		*/
		bool supportsExtension(const std::string& extension) const;

	protected:
		/**
		* Add a file extension to this factory
		* @param ext The extension without the dot such as "jpeg"
		*/
		void addFileExtension(const std::string& ext);

		/**
		* Make sure this factory has access to nap::Core
		* @param core
		*/
		void setCore(Core& core);

	private:
		std::vector<std::string> mFileExtensions;
		Core* mCore;
	};
}

RTTI_DECLARE_BASE(nap::Resource)
RTTI_DECLARE_BASE(nap::ResourceLoader)
