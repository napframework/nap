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
	class ShaderResource : public Resource
	{
		friend class ShaderResourceLoader;
		RTTI_ENABLE_DERIVED_FROM(Resource)
	public:

		virtual bool init(InitResult& initResult);

		virtual void finish(Resource::EFinishMode mode) override;

		/**
		 * @return the shader resource display name
		 */
		virtual const std::string getDisplayName() const override;

		/**
		 * @return the opengl shader that can be used for drawing
		 * Note that this will also initialize the shader on the GPU
		 * if the shader can't be created a warning will be raised
		 * in that case future binding calls won't work
		 */
		opengl::Shader& getShader();

		Attribute<std::string>		mVertPath		= { this, "mVertShader", "" };
		Attribute<std::string>		mFragPath		= { this, "mFragShader", "" };

	private:
		// Path to shader on disk
		std::string					mDisplayName;

		// Shader that is managed by this resource
		opengl::Shader*				mShader = nullptr;
		opengl::Shader*				mPrevShader = nullptr;
	};

}

RTTI_DECLARE(nap::ShaderResource)


