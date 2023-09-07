/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Local Includes
#include "samplerdeclaration.h"
#include "materialcommon.h"

// External Includes
#include <rtti/objectptr.h>
#include <utility/dllexport.h>
#include <nap/resource.h>
#include <nap/numeric.h>

namespace nap
{
	// Forward Declares
	class Texture2D;
	class TextureCube;
	class SamplerInstance;
	class RenderService;

	namespace sampler
	{
		constexpr const char* texture	= "Texture";
		constexpr const char* textures	= "Textures";
	}

	/**
	 *	Supported sampler filter modes.
	 */
	enum class EFilterMode : uint32
	{
		Nearest = 0,				///< Nearest sampling
		Linear						///< Linear sampling
	};


	/**
	 *	Supported sampler wrap modes
	 */
	enum class EAddressMode : uint32
	{
		Repeat = 0,					///< Repeat
		MirroredRepeat,				///< MirroredRepeat 
		ClampToEdge,				///< ClampToEdge
		ClampToBorder				///< ClampToBorder
	};


	/**
	 * Supported number of anisotropic sample overrides
	 */
	enum class EAnisotropicSamples : uint32
	{
		Default		= 0,			///< Pick system default, as defined by the render service configuration
		Four		= 4,			///< Four samples
		Eight		= 8,			///< Eight samples
		Sixteen		= 16			///< Sixteen samples
	};


	/**
	 * Supported sampler border colors
	 */
	enum class EBorderColor : uint32
	{
		FloatTransparentBlack = 0,	///< 
		IntTransparentBlack,
		FloatOpaqueBlack,
		IntOpaqueBlack,
		FloatOpaqueWhite,
		IntOpaqueWhite
	};


	//////////////////////////////////////////////////////////////////////////
	// Sampler
	//////////////////////////////////////////////////////////////////////////

	/**
	 * Sampler resource base class.
	 */
	class NAPAPI Sampler : public Resource
	{
		RTTI_ENABLE(Resource)
	public:
		Sampler() = default;

		std::string			mName;															///< Property: 'Name' sampler shader name
		EFilterMode			mMinFilter				= EFilterMode::Linear;					///< Property: 'MinFilter' minimizing filter
		EFilterMode			mMaxFilter				= EFilterMode::Linear;					///< Property: 'MaxFilter' maximizing filter	
		EFilterMode			mMipMapMode				= EFilterMode::Linear;					///< Property: 'MipMapMode' mip map mode
		EAddressMode		mAddressModeVertical	= EAddressMode::ClampToEdge;			///< Property: 'AddressModeVertical' vertical address mode
		EAddressMode		mAddressModeHorizontal	= EAddressMode::ClampToEdge;			///< Property: 'AddressModeHorizontal'	horizontal address mode
		EAnisotropicSamples	mMaxAnisotropy			= EAnisotropicSamples::Default;			///< Property: 'AnisotropicSamples' max number of anisotropic filter samples
		EBorderColor		mBorderColor			= EBorderColor::IntOpaqueBlack;			///< Property: 'BorderColor' border color used for texture lookups
		EDepthCompareMode	mCompareMode			= EDepthCompareMode::LessOrEqual;		///< Property: 'DepthCompareMode' 
		bool				mEnableCompare			= false;								///< Property: 'EnableCompare'
		uint32				mMaxLodLevel			= 1000;									///< Property: 'MaxLodLevel' max number of considered LODs, 0 = only consider highest lod
	};


	//////////////////////////////////////////////////////////////////////////
	// Sampler2D
	//////////////////////////////////////////////////////////////////////////

	/**
	 * 2D sampler resource. Assigns a single 2D texture to a shader.
	 * Applies filtering and transformations to compute the final color that is retrieved from a texture.
	 * myshader.frag example:
	 *
	 * ~~~~~{.cpp}
	 * uniform sampler2D inTexture;			//< Texture
	 * ~~~~~
	 */
	class NAPAPI Sampler2D : public Sampler
	{
		RTTI_ENABLE(Sampler)
	public:
		Sampler2D() = default;
		mutable rtti::ObjectPtr<Texture2D> mTexture = nullptr;		///< Property: 'Texture' the texture to bind
	};


	//////////////////////////////////////////////////////////////////////////
	// SamplerCube
	//////////////////////////////////////////////////////////////////////////

	/**
	 * Cube sampler resource. Assigns a single 2D texture to a shader.
	 * Applies filtering and transformations to compute the final color that is retrieved from a texture.
	 * myshader.frag example:
	 *
	 * ~~~~~{.cpp}
	 * uniform samplerCube inTexture;		//< Texture
	 * ~~~~~
	 */
	class NAPAPI SamplerCube : public Sampler
	{
		RTTI_ENABLE(Sampler)
	public:
		SamplerCube() = default;
		mutable rtti::ObjectPtr<TextureCube> mTextureCube;			///< Property: 'Texture' the texture to bind
	};


	//////////////////////////////////////////////////////////////////////////
	// SamplerArray
	//////////////////////////////////////////////////////////////////////////

	/**
	 * Base class of a list of sampler resources.
	 */
	class NAPAPI SamplerArray : public Sampler
	{
		RTTI_ENABLE(Sampler)

	public:
		/**
		 * Retrieve the number of elements in this array
		 * @return The number of elements in this array
		 */
		virtual int getNumElements() const = 0;
	};


	//////////////////////////////////////////////////////////////////////////
	// Sampler2DArray
	//////////////////////////////////////////////////////////////////////////

	/**
	 * 2D sampler array resource. Assigns multiple textures to a shader as an array.
	 * Note that number of textures must match the number of inputs on the shader.
	 * myshader.frag example:
	 *
	 * ~~~~~{.cpp}
	 * uniform sampler2D textures[20];			//< array of 20 textures
	 * ~~~~~
	 */
	class NAPAPI Sampler2DArray : public SamplerArray
	{
		RTTI_ENABLE(SamplerArray)
	public:

		Sampler2DArray() = default;
		Sampler2DArray(int inSize) :
			mTextures(inSize)									{ }

		/**
		 * @return The number of elements in this array
		 */
		virtual int getNumElements() const override				{ return mTextures.size(); }

		std::vector<rtti::ObjectPtr<Texture2D>> mTextures;		///< Property: 'Textures' textures to bind, must be of the same length as the shader declaration.
	};


	//////////////////////////////////////////////////////////////////////////
	// SamplerCubeArray
	//////////////////////////////////////////////////////////////////////////

	/**
	 * Cube sampler array resource. Assigns multiple textures to a shader as an array.
	 * Note that number of textures must match the number of inputs on the shader.
	 * myshader.frag example:
	 *
	 * ~~~~~{.cpp}
	 * uniform samplerCube textures[20];		//< array of 20 textures
	 * ~~~~~
	 */
	class NAPAPI SamplerCubeArray : public SamplerArray
	{
		RTTI_ENABLE(SamplerArray)
	public:

		SamplerCubeArray() = default;
		SamplerCubeArray(int inSize) :
			mTextures(inSize) { }

		/**
		 * @return The number of elements in this array
		 */
		virtual int getNumElements() const override { return mTextures.size(); }

		std::vector<rtti::ObjectPtr<TextureCube>> mTextures;	///< Property: 'Textures' textures to bind, must be of the same length as the shader declaration.
	};
}
