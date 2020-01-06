#pragma once

// Local Includes
#include "nshaderutils.h"

// External Includes
#include <rtti/objectptr.h>
#include <utility/dllexport.h>
#include <nap/resource.h>

namespace nap
{
	class Texture2D;

	class NAPAPI Sampler : public Resource
	{
		RTTI_ENABLE(Resource)

	public:
		Sampler() = default;

		std::string mName;
	};


	/**
	 * Represents an array of 'texture' uniforms.
	 * Derived classes should return the correct amount of elements that are in the array.
 	 */
	class NAPAPI SamplerArray : public Sampler
	{
		RTTI_ENABLE(Sampler)

	public:
		/**
		 * Retrieve the number of elements in this array
		 *
		 * @return The number of elements in this array
		 */
		virtual int getNumElements() const = 0;
	};

	class NAPAPI SamplerInstance
	{
		RTTI_ENABLE()

	public:
		SamplerInstance(VkDevice device, const opengl::SamplerDeclaration& declaration);

		bool init(utility::ErrorState& errorState);

		const opengl::SamplerDeclaration& getDeclaration() const { assert(mDeclaration != nullptr); return *mDeclaration; }
		VkSampler getSampler() const { return mSampler; }

	private:
		VkDevice							mDevice;
		const opengl::SamplerDeclaration*	mDeclaration = nullptr;
		VkSampler							mSampler = nullptr;
	};

	/**
	 * Pointer to a single 2D Texture that can be pushed to a shader uniform.
	 */
	class NAPAPI Sampler2D : public Sampler
	{
		RTTI_ENABLE(Sampler)

	public:
		Sampler2D() = default;

		rtti::ObjectPtr<Texture2D> mTexture = nullptr;		///< Texture to use for this uniform
	};

	class NAPAPI Sampler2DInstance : public SamplerInstance
	{
		RTTI_ENABLE(SamplerInstance)

	public:
		Sampler2DInstance(VkDevice device, const opengl::SamplerDeclaration& declaration, const Sampler2D* sampler2D);

		/**
		* @param texture The texture resource to set for this uniform.
		*/
		void setTexture(Texture2D& texture) { mTexture2D = &texture; }

		rtti::ObjectPtr<Texture2D>					mTexture2D;
	};


	/**
	 * Stores and array of 2D textures.
	 * Number of textures must be equal or lower than the number of textures declared in the shader.
	 */
	class NAPAPI Sampler2DArray : public SamplerArray
	{
		RTTI_ENABLE(SamplerArray)

	public:

		Sampler2DArray() = default;

		/**
		 * Constructor to ensure the array has the correct size to match the shader
		 */
		Sampler2DArray(int inSize) :
			mTextures(inSize)
		{
		}

		/**
		 * Retrieve the number of elements in this array
		 *
		 * @return The number of elements in this array
		 */
		virtual int getNumElements() const override { return mTextures.size(); }

		std::vector<rtti::ObjectPtr<Texture2D>> mTextures;		///< Texture to use for this uniform
	};

	class NAPAPI Sampler2DArrayInstance : public SamplerInstance
	{
		RTTI_ENABLE(SamplerInstance)

	public:
		Sampler2DArrayInstance(VkDevice device, const opengl::SamplerDeclaration& declaration, const Sampler2DArray* sampler2DArray);

		std::vector<rtti::ObjectPtr<Texture2D>>				mTextures;
	};

}
