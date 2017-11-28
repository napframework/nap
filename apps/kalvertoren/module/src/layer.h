#pragma once

// External Includes
#include <rtti/rttiobject.h>
#include <image.h>
#include <texture2d.h>

namespace nap
{
	/**
	 * Base layer
	 */
	class NAPAPI Layer : public rtti::RTTIObject
	{
		RTTI_ENABLE(rtti::RTTIObject)
	public:
		virtual ~Layer();

		/**
		* Initialize this object after de-serialization
		* @param errorState contains the error message when initialization fails
		*/
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 * @return the texture associated with this layer
		 * Every derived class should implement it's own method;
		 */
		virtual nap::BaseTexture2D& getTexture() = 0;

		/**
		 * @return the texture associated with this layer
		 * Every derived class should implement it's own method
		 */
		virtual const nap::BaseTexture2D& getTexture() const = 0;
	};


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
		virtual nap::BaseTexture2D&			getTexture() override					{ return mTexture; }

		/**
		 *	@return const texture associated with this layer
		 */
		virtual const nap::BaseTexture2D&	getTexture() const override				{ return mTexture; }


		std::string mImagePath;														///< Path to the image on disk

	private:
		nap::BaseTexture2D mTexture;												///< GPU texture from CPU
		nap::Pixmap mPixmap;														///< CPU Pixel representation
	};
}
