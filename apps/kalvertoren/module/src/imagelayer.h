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
		nap::BaseTexture2D&			getTexture()					{ return mTexture; }

		/**
		 *	@return const texture associated with this layer
		 */
		 const nap::BaseTexture2D&	getTexture() const				{ return mTexture; }

	protected:
		virtual std::unique_ptr<LayerInstance> createInstance() override;

	public:
		std::string mImagePath;														///< Path to the image on disk

	private:
		nap::BaseTexture2D mTexture;												///< GPU texture from CPU
		nap::Pixmap mPixmap;														///< CPU Pixel representation
	};


	/**
	 * Instance object for ImageLayer. Forwards texture to the resource.
	 */
	class NAPAPI ImageLayerInstance : public LayerInstance
	{
	public:
		ImageLayerInstance(ImageLayer& layer) :
			mImageLayer(&layer)
		{
		}

		/**
		 *	@return the texture associated with this layer
		 */
		virtual nap::BaseTexture2D&			getTexture() override { return mImageLayer->getTexture(); }

		/**
		 * 	@return const texture associated with this layer
		 */
		virtual const nap::BaseTexture2D&	getTexture() const override { return mImageLayer->getTexture(); }

	private:
		ImageLayer* mImageLayer;		///< Pointer to resource Layer
	};
}
