#pragma once

// External Includes
#include <nshader.h>
#include <nap/resource.h>

namespace nap
{
	// Forward Declares
	class ShaderResourceLoader;

	/**
	 * Wraps a shader that can be used as a resource for a material instance
	 * Note that the shader is not initialized (created) when the resource is created
	 * this is deferred to actual rendering because of gl initialization
	 */
	class ShaderResource : public Resource
	{
		friend class ShaderResourceLoader;
		RTTI_ENABLE_DERIVED_FROM(Resource)
	public:
		// Construct a shader resource using a vertex and fragment path
		ShaderResource(const std::string& vertPath, const std::string& fragPath);

		/**
		 * @return the shader resource display name
		 */
		virtual const std::string& getDisplayName() const override;

		/**
		 * @return the opengl shader that can be used for drawing
		 * Note that this will also initialize the shader on the GPU
		 * if the shader can't be created a warning will be raised
		 * in that case future binding calls won't work
		 */
		opengl::Shader& getShader();

	private:
		// Path to shader on disk
		std::string		mVertPath;
		std::string		mFragPath;
		std::string		mDisplayName;

		// Shader that is managed by this resource
		opengl::Shader	mShader;

		// If the shader has been loaded
		bool			mLoaded = false;
	};


	/**
	 * Creates a shader resource, used by resourcemanager
	 * to check if based on the .frag and .vert extension of a file
	 * the resource can be loaded
	 */
	class ShaderResourceLoader : public ResourceLoader
	{
		RTTI_ENABLE()
	public:
		// Constructor
		ShaderResourceLoader();

		/**
		 * Creates a shader resource
		 * @return if the shader resource was created successfully or not
		 * @param resourcePath path to the resource to load
		 * @param outAsset the created resource
		 */
		virtual std::unique_ptr<Resource> loadResource(const std::string& resourcePath) const override;

		// Vert extension name ("vert")
		const static std::string vertExtension;

		// Frag extension name ("frag")
		const static std::string fragExtension;
	};

}

RTTI_DECLARE_BASE(nap::ShaderResource)
RTTI_DECLARE(nap::ShaderResourceLoader)


