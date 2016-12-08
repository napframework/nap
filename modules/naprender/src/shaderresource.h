#pragma once

// External Includes
#include <nshader.h>
#include <nap/resource.h>

namespace nap
{
	/**
	 * Wraps a shader that can be used as a resource for a material instance
	 */
	class ShaderResource : public Resource
	{
		RTTI_ENABLE()
	public:
		// Construct a shader resource using a vertex and fragment path
		ShaderResource(const std::string& vertPath, const std::string& fragPath);
		ShaderResource() = default;

		/**
		 * @return the shader resource display name
		 */
		virtual const std::string& getDisplayName() const override;

		/**
		 * @return a material that references the shader resource
		 */
		virtual Object* createInstance(Core& core, Object& parent) override;

	private:
		// Path to shader on disk
		std::string		mVertPath;
		std::string		mFragPath;
		std::string		mDisplayName;

		// Shader that is managed by this resource
		opengl::Shader	mShader;
	};

}

RTTI_DECLARE(nap::ShaderResource)


