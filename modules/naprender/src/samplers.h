#pragma once

// Local Includes
#include "samplerdeclaration.h"

// External Includes
#include <rtti/objectptr.h>
#include <utility/dllexport.h>
#include <nap/resource.h>

namespace nap
{
	class Texture2D;
	class SamplerInstance;
	class RenderService;

	using SamplerChangedCallback = std::function<void(SamplerInstance&)>;

	/**
	 *	Texture min filter
	 */
	enum class EFilterMode : int
	{
		Nearest = 0,				///< Nearest
		Linear						///< Linear
	};

	/**
	 *	Texture wrap mode
	 */
	enum class EAddressMode : int
	{
		Repeat = 0,					///< Repeat 
		MirroredRepeat,				///< MirroredRepeat
		ClampToEdge,				///< ClampToEdge
		ClampToBorder				///< ClampToBorder
	};

	class NAPAPI Sampler : public Resource
	{
		RTTI_ENABLE(Resource)

	public:
		Sampler() = default;

		std::string		mName;

		EFilterMode		mMinFilter				= EFilterMode::Linear;					///< Property: 'MinFilter' minimizing filter
		EFilterMode		mMaxFilter				= EFilterMode::Linear;					///< Property: 'MaxFilter' maximizing filter	
		EFilterMode		mMipMapMode				= EFilterMode::Linear;					///< Property: 'MipMapMode' mip map mode
		EAddressMode	mAddressModeVertical	= EAddressMode::ClampToEdge;			///< Property: 'AddressModeVertical' vertical address mode
		EAddressMode	mAddressModeHorizontal	= EAddressMode::ClampToEdge;			///< Property: 'AddressModeHorizontal'	horizontal address mode
		int				mMaxLodLevel			= 1000;									///< Property: 'MaxLodLevel' max number of considered LODs, 0 = only consider highest lod
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
		SamplerInstance(RenderService& renderService, const SamplerDeclaration& declaration, const Sampler* sampler, const SamplerChangedCallback& samplerChangedCallback);
		virtual ~SamplerInstance();

		bool init(utility::ErrorState& errorState);

		const SamplerDeclaration& getDeclaration() const { assert(mDeclaration != nullptr); return *mDeclaration; }
		VkSampler getVulkanSampler() const { return mVulkanSampler; }

	protected:
		void raiseChanged() { if (mSamplerChangedCallback) mSamplerChangedCallback(*this); }

	private:
		RenderService*					mRenderService = nullptr;
		VkDevice						mDevice;
		const Sampler*					mSampler = nullptr;
		const SamplerDeclaration*		mDeclaration = nullptr;
		VkSampler						mVulkanSampler = nullptr;
		SamplerChangedCallback			mSamplerChangedCallback;
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
		Sampler2DInstance(RenderService& renderService, const SamplerDeclaration& declaration, const Sampler2D* sampler2D, const SamplerChangedCallback& samplerChangedCallback);

		/**
		* @param texture The texture resource to set for this uniform.
		*/
		void setTexture(Texture2D& texture);

		bool hasTexture() const { return mTexture2D != nullptr; }
		const Texture2D& getTexture() const { return *mTexture2D; }

	private:
		void onTextureChanged(const Texture2D&);

	private:
		rtti::ObjectPtr<Texture2D>		mTexture2D;
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
		Sampler2DArrayInstance(RenderService& renderService, const SamplerDeclaration& declaration, const Sampler2DArray* sampler2DArray, const SamplerChangedCallback& samplerChangedCallback);

		int getNumElements() const { return mTextures.size(); }

		bool hasTexture(int index) const { return mTextures[index] != nullptr; }
		const Texture2D& getTexture(int index) const { return *mTextures[index]; }
		void setTexture(int index, Texture2D& texture);

	private:
		std::vector<rtti::ObjectPtr<Texture2D>>				mTextures;
	};

}
