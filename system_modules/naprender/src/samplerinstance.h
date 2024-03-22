/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Local Includes
#include "sampler.h"

namespace nap
{
	// Called when the bound sampler resource changes
	using SamplerChangedCallback = std::function<void(SamplerInstance&, int)>;

	/**
	 * Sampler instance base class. Allows for interfacing with samplers at run-time.
	 */
	class NAPAPI SamplerInstance
	{
		RTTI_ENABLE()
	public:
		/**
		 * Constructs the instance based on the shader declaration and resource.
		 * @param renderService render engine
		 * @param declaration sampler shader declaration
		 * @param sampler the sampler resource used to create this instance.
		 * @param samplerChangedCallback called when the 'texture' changes.
		 */
		SamplerInstance(RenderService& renderService, const SamplerDeclaration& declaration, const Sampler* sampler, const SamplerChangedCallback& samplerChangedCallback);
		virtual ~SamplerInstance();

		/**
		 * Initializes the sampler.
		 * @param errorState contains the error if initialization fails
		 * @return if initialization succeeded.
		 */
		bool init(utility::ErrorState& errorState);

		/**
		 * @return sampler shader declaration.
		 */
		const SamplerDeclaration& getDeclaration() const	{ assert(mDeclaration != nullptr); return *mDeclaration; }
		
		/**
		 * @return the vulkan sampler handle
		 */
		VkSampler getVulkanSampler() const					{ return mVulkanSampler; }

	protected:
		/**
		 * Called when the texture changes.
		 * @param the texture index, zero unless the noninitial texture in a sampler array is changed.
		 */
		void raiseChanged(int index = 0)					{ if (mSamplerChangedCallback) mSamplerChangedCallback(*this, index); }

		RenderService*					mRenderService = nullptr;
		VkSampler						mVulkanSampler = VK_NULL_HANDLE;
		const Sampler*					mSampler = nullptr;
		const SamplerDeclaration*		mDeclaration = nullptr;
		SamplerChangedCallback			mSamplerChangedCallback;
	};


	//////////////////////////////////////////////////////////////////////////
	// SamplerArrayInstance
	//////////////////////////////////////////////////////////////////////////

	/**
	 * Sampler array instance.
	 */
	class NAPAPI SamplerArrayInstance : public SamplerInstance
	{
		RTTI_ENABLE(SamplerInstance)
	public:
		SamplerArrayInstance(RenderService& renderService, const SamplerDeclaration& declaration, const Sampler* sampler, const SamplerChangedCallback& samplerChangedCallback);
		virtual ~SamplerArrayInstance();

		/**
		 * @return total number of textures that can be assigned
		 */
		virtual int getNumElements() const					{ return mDeclaration->mNumElements; }
	};


	//////////////////////////////////////////////////////////////////////////
	// Sampler2DInstance
	//////////////////////////////////////////////////////////////////////////

	/**
	 * 2D sampler instance. Binds a single 2D texture to a shader.
	 * The texture can be changed at runtime.
	 */
	class NAPAPI Sampler2DInstance : public SamplerInstance
	{
		RTTI_ENABLE(SamplerInstance)
	public:
		/**
		 * Constructs the instance based on the shader declaration and resource.
		 * @param renderService render engine
		 * @param declaration sampler shader declaration
		 * @param sampler2D the sampler resource used to create this instance.
		 * @param samplerChangedCallback called when the 'texture' changes.
		 */
		Sampler2DInstance(RenderService& renderService, const SamplerDeclaration& declaration, const Sampler2D* sampler2D, const SamplerChangedCallback& samplerChangedCallback);

		/**
		 * Binds a texture to the 2D sampler.
		 * @param texture the new texture to bind
		 */
		void setTexture(Texture2D& texture);

		/**
		 * @return if a texture is bound to the 2D sampler.
		 */
		bool hasTexture() const								{ return mTexture2D != nullptr; }

		/**
		 * @return currently bound texture.
		 */
		const Texture2D& getTexture() const					{ return *mTexture2D; }

	private:
		/**
		 * Whenever a new texture is set that differs from the one registered in the data file,
		 * we must ensure that the sampler refers to valid memory after destruction of said texture.
		 */
		void onTextureDestroyed();
		nap::Slot<> textureDestroyedSlot = { [&]() -> void { onTextureDestroyed();  } };

		rtti::ObjectPtr<Texture2D> mTexture2D;
	};


	//////////////////////////////////////////////////////////////////////////
	// Sampler2DArrayInstance
	//////////////////////////////////////////////////////////////////////////

	/**
	 * 2D sampler array instance. Binds multiple 2D textures to a shader.
	 * The array can be changed at runtime.
	 */
	class NAPAPI Sampler2DArrayInstance : public SamplerArrayInstance
	{
		RTTI_ENABLE(SamplerArrayInstance)
	public:
		/**
		 * Constructs the instance based on the shader declaration and resource.
		 * @param renderService render engine
		 * @param declaration sampler shader declaration
		 * @param sampler2DArray the sampler resource used to create this instance.
		 * @param samplerChangedCallback called when the 'texture' changes.
		 */
		Sampler2DArrayInstance(RenderService& renderService, const SamplerDeclaration& declaration, const Sampler2DArray* sampler2DArray, const SamplerChangedCallback& samplerChangedCallback);

		/**
		 * @return if a texture is bound to the given index.
		 */
		bool hasTexture(int index) const						{ assert(index < mTextures.size()); return mTextures[index] != nullptr; }

		/**
		 * @param index the index to get the texture for.
		 * @return the texture at the given index
		 */
		const Texture2D& getTexture(int index) const			{ assert(index < mTextures.size()); return *mTextures[index]; }

		/**
		 * Binds a texture at the given index
		 * @param index the index to bind the texture to
		 * @param texture the texture to bind.
		 */
		void setTexture(int index, Texture2D& texture);

		/**
		 * Array subscript operator, returns a specific texture in the array as a reference,
		 * making the following possible: `mTextures[0] = myTexture`;
		 * @return a specific value in the array as a reference.
		 */
		rtti::ObjectPtr<Texture2D>& operator[](size_t index)	{ assert(index < mTextures.size()); return mTextures[index]; }

	private:
		std::vector<rtti::ObjectPtr<Texture2D>> mTextures;
	};


	//////////////////////////////////////////////////////////////////////////
	// SamplerCubeInstance
	//////////////////////////////////////////////////////////////////////////

	/**
	 * Cube sampler instance. Binds a single Cube texture to a shader.
	 * The texture can be changed at runtime.
	 */
	class NAPAPI SamplerCubeInstance : public SamplerInstance
	{
		RTTI_ENABLE(SamplerInstance)
	public:
		/**
		 * Constructs the instance based on the shader declaration and resource.
		 * @param renderService render engine
		 * @param declaration sampler shader declaration
		 * @param samplerCube the sampler resource used to create this instance.
		 * @param samplerChangedCallback called when the 'texture' changes.
		 */
		SamplerCubeInstance(RenderService& renderService, const SamplerDeclaration& declaration, const SamplerCube* samplerCube, const SamplerChangedCallback& samplerChangedCallback);

		/**
		 * Binds a texture to the cube sampler.
		 * @param texture the new texture to bind
		 */
		void setTexture(TextureCube& textureCube);

		/**
		 * @return if a texture is bound to the cube sampler.
		 */
		bool hasTexture() const									{ return mTextureCube != nullptr; }

		/**
		 * @return currently bound texture.
		 */
		const TextureCube& getTexture() const					{ (mTextureCube != nullptr); return *mTextureCube; }

	private:
		/**
		 * Whenever a new texture is set that differs from the one registered in the data file,
		 * we must ensure that the sampler refers to valid memory after destruction of said texture.
		 */
		void onTextureDestroyed();
		nap::Slot<> textureDestroyedSlot = { [&]() -> void { onTextureDestroyed();  } };

		rtti::ObjectPtr<TextureCube> mTextureCube;
	};


	//////////////////////////////////////////////////////////////////////////
	// SamplerCubeArrayInstance
	//////////////////////////////////////////////////////////////////////////

	/**
	 * Cube sampler array instance. Binds multiple cube textures to a shader.
	 * The array can be changed at runtime.
	 */
	class NAPAPI SamplerCubeArrayInstance : public SamplerArrayInstance
	{
		RTTI_ENABLE(SamplerArrayInstance)
	public:
		/**
		 * Constructs the instance based on the shader declaration and resource.
		 * @param renderService render engine
		 * @param declaration sampler shader declaration
		 * @param samplerCubeArray the sampler resource used to create this instance.
		 * @param samplerChangedCallback called when the 'texture' changes.
		 */
		SamplerCubeArrayInstance(RenderService& renderService, const SamplerDeclaration& declaration, const SamplerCubeArray* samplerCubeArray, const SamplerChangedCallback& samplerChangedCallback);

		/**
		 * @return if a texture is bound to the given index.
		 */
		bool hasTexture(int index) const						{ assert(index < mTextures.size()); return mTextures[index] != nullptr; }

		/**
		 * @param index the index to get the texture for.
		 * @return the texture at the given index
		 */
		const TextureCube& getTexture(int index) const			{ assert(index < mTextures.size()); return *mTextures[index]; }

		/**
		 * Binds a texture at the given index
		 * @param index the index to bind the texture to
		 * @param texture the texture to bind.
		 */
		void setTexture(int index, TextureCube& textureCube);

		/**
		 * Array subscript operator, returns a specific texture in the array as a reference,
		 * making the following possible: `mTextures[0] = myTexture`;
		 * @return a specific value in the array as a reference.
		 */
		rtti::ObjectPtr<TextureCube>& operator[](size_t index)	{ assert(index < mTextures.size()); return mTextures[index]; }

	private:
		std::vector<rtti::ObjectPtr<TextureCube>> mTextures;
	};
}
