#pragma once

// External Includes
#include "layer.h"

namespace nap
{
	/**
	 * Loads an image from disk that can be used as a layer
	 * This object discards the pixel data after load to save memory
	 */
	class NAPAPI ImageLayer : public Layer
	{
		RTTI_ENABLE(Layer)
	public:
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 *	@return the texture associated with this layer
		 */
		nap::Texture2D&			getTexture()					{ return mImage; }

		/**
		 *	@return const texture associated with this layer
		 */
		 const nap::Texture2D&	getTexture() const				{ return mImage; }

	protected:
		virtual std::unique_ptr<LayerInstance> createInstance() override;

	public:
		std::string mImagePath;		///< Path to the image on disk

	private:
		ImageFromFile mImage;	///< Texture
	};


	/**
	 * Instance object for ImageLayer. Forwards texture to the resource.
	 */
	class NAPAPI ImageLayerInstance : public LayerInstance
	{
		RTTI_ENABLE(LayerInstance)
	public:
		ImageLayerInstance(ImageLayer& layer) :
			mImageLayer(&layer)
		{
		}

		/**
		 *	@return the texture associated with this layer
		 */
		virtual nap::Texture2D&			getTexture() override { return mImageLayer->getTexture(); }

		/**
		 * 	@return const texture associated with this layer
		 */
		virtual const nap::Texture2D&	getTexture() const override { return mImageLayer->getTexture(); }

	private:
		ImageLayer* mImageLayer;		///< Pointer to resource Layer
	};
}
