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
	 */
	class ShaderResource : public Resource
	{
		friend class ShaderResourceLoader;
		RTTI_ENABLE()
	public:
		// Construct a shader resource using a vertex and fragment path
		ShaderResource(const std::string& vertPath, const std::string& fragPath);

		/**
		 * @return the shader resource display name
		 */
		virtual const std::string& getDisplayName() const override;

		/**
		 * @return a material that references the shader resource
		 */
		virtual Object* createInstance(Core& core, Object& parent) override;

		/**
		 * @return the opengl shader that can be used for drawing
		 */
		opengl::Shader& getShader()								{ return mShader; }

	private:
		// Path to shader on disk
		std::string		mVertPath;
		std::string		mFragPath;
		std::string		mDisplayName;

		// Shader that is managed by this resource
		opengl::Shader	mShader;
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


