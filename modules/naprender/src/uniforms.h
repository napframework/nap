#pragma once

#include "nshaderutils.h"
#include "nap/resource.h"
#include "glm/glm.hpp"

namespace nap
{
	class TextureResource;

	/**
	 *
	 */
	class Uniform : public Resource	// TODO: derive from simple Object class
	{
		RTTI_ENABLE(Resource)
	public:
		/**
		*
		*/
		virtual bool init(utility::ErrorState& errorState) override { return true; }

		/**
		*
		*/
		virtual void finish(EFinishMode mode) override {}

		/**
		*
		*/
		virtual const std::string getDisplayName() const  override { return "uniform"; }

		/**
		*
		*/
		virtual opengl::GLSLType getGLSLType() const = 0;

	public:
		std::string mName;
	};


	/**
	*
	*/
	class UniformValue : public Uniform
	{
		RTTI_ENABLE(Uniform)
	public:

		/**
		*
		*/
		virtual void push(const opengl::UniformDeclaration& declaration) const = 0;
	};


	/**
	*
	*/
	class UniformTexture : public Uniform
	{
		RTTI_ENABLE(Uniform)
	public:

		/**
		*
		*/
		virtual void push(const opengl::UniformDeclaration& declaration, int textureUnit) const = 0;
	};


	/**
	*
	*/
	class UniformInt : public UniformValue
	{
		RTTI_ENABLE(UniformValue)
	public:

		/**
		*
		*/
		void setValue(int value)
		{
			mValue = value;
		}

		/**
		*
		*/
		virtual void push(const opengl::UniformDeclaration& declaration) const override;

		/**
		*
		*/
		virtual opengl::GLSLType getGLSLType() const override
		{
			return opengl::GLSLType::Int;
		}

		int mValue;
	};


	/**
	*
	*/
	class UniformVec4 : public UniformValue
	{
		RTTI_ENABLE(UniformValue)
	public:

		/**
		*
		*/
		void setValue(const glm::vec4& value)
		{
			mValue = value;
		}

		/**
		*
		*/
		virtual void push(const opengl::UniformDeclaration& declaration) const override;

		/**
		*
		*/
		virtual opengl::GLSLType getGLSLType() const override
		{
			return opengl::GLSLType::Vec4;
		}

		glm::vec4 mValue;
	};


	/**
	*
	*/
	class UniformMat4 : public UniformValue
	{
		RTTI_ENABLE(UniformValue)
	public:

		/**
		*
		*/
		void setValue(const glm::mat4& value)
		{
			mValue = value;
		}

		/**
		*
		*/
		virtual void push(const opengl::UniformDeclaration& declaration) const override;


		/**
		*
		*/
		virtual opengl::GLSLType getGLSLType() const override
		{
			return opengl::GLSLType::Mat4;
		}

		glm::mat4 mValue;
	};


	/**
	*
	*/
	class UniformTexture2D : public UniformTexture
	{
		RTTI_ENABLE(UniformTexture)
	public:

		/**
		*
		*/
		void setTexture(TextureResource& texture)
		{
			mTexture = &texture;
		}

		/**
		*
		*/
		virtual void push(const opengl::UniformDeclaration& declaration, int textureUnit) const override;

		/**
		*
		*/
		virtual opengl::GLSLType getGLSLType() const override
		{
			return opengl::GLSLType::Tex2D;
		}

		TextureResource* mTexture = nullptr;
	};
}
