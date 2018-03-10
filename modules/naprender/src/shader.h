#pragma once

// External Includes
#include <nshader.h>
#include <utility/dllexport.h>
#include <rtti/rttiobject.h>

namespace nap
{
	/**
	 * Wraps a shader that can be used as a resource for a material instance
	 * Note that the shader is not initialized (created) when the resource is created
	 * this is deferred to actual rendering because of gl initialization
	 */
	class NAPAPI Shader : public rtti::RTTIObject
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

		std::string							mVertPath;			///< Property: 'mVertShader' path to the vertex shader on disk
		std::string							mFragPath;			///< Property: 'mFragShader' path to the fragment shader on disk

	private:
		// Path to shader on disk
		std::string							mDisplayName;

		// Shader that is managed by this resource
		std::unique_ptr<opengl::Shader>		mShader;
	};

}

