#pragma once

// External Includes
#include <nshader.h>
#include <nap/resource.h>

namespace nap
{
	/**
	 * Wraps a shader that can be used as a resource for a material instance
	 * Note that the shader is not initialized (created) when the resource is created
	 * this is deferred to actual rendering because of gl initialization
	 */
	class ShaderResource : public rtti::RTTIObject
	{
		friend class ShaderResourceLoader;
		RTTI_ENABLE(rtti::RTTIObject)
	public:

		/**
		 * Creates and inits opengl shader.
		 */
		virtual bool init(utility::ErrorState& errorState);

		/**
		 * @return the opengl shader that can be used for drawing
		 */
		opengl::Shader& getShader();

		std::string							mVertPath;
		std::string							mFragPath;

	private:
		// Path to shader on disk
		std::string							mDisplayName;

		// Shader that is managed by this resource
		std::unique_ptr<opengl::Shader>		mShader;
	};

}

