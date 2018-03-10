#pragma once

// External Includes
#include <nshader.h>
#include <utility/dllexport.h>
#include <rtti/rttiobject.h>

namespace nap
{
	/**
	 * Resource that loads and compiles a shader from disk using the provided vertex and fragment shader paths.
	 * A material and material instance link to a shader. The shader is compiled on initialization.
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

