#pragma once

// External Includes
#include "layer.h"

namespace nap
{
	/**
	 * Contains a sequence of images. The ImageSequenceLayerInstance is capable of playback of this object.
	 */
	class NAPAPI ImageSequenceLayer : public Layer
	{
		RTTI_ENABLE(Layer)
	public:
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		 *	@return the texture associated with this layer
		 */
		nap::BaseTexture2D&			getTexture(int index) { return *mImages[index]; }

		/**
		 *	@return const texture associated with this layer
		 */
		const nap::BaseTexture2D&	getTexture(int index) const { return *mImages[index]; }

		/**
		* @return number of images in this sequence
		*/
		int getNumImages() const { return (int)mImages.size(); }

	protected:
		/**
		 *	@return Instance object for this ImageSequenceLayer
		 */
		virtual std::unique_ptr<LayerInstance> createInstance() override;

	public:
		std::string			mBaseFilename;				///< The base filename used to find all images for this sequence. Must contain %[0#]d format specifier
		int					mFPS = 30;					///< Playback framerate for this sequence

	private:
		std::vector<std::unique_ptr<Image>>	mImages;	///< The images created from the files found on disk
	};

	/**
	 * Instance object for ImageSequenceLayer. Plays back the sequence with the given framerate. Current texture can be retrieved through getTexture().
	 */
	class NAPAPI ImageSequenceLayerInstance : public LayerInstance
	{
	public:
		ImageSequenceLayerInstance(ImageSequenceLayer& layer);

		/**
		 *	Update the animation of this sequence.
		 */
		virtual void update(double deltaTime) override;

		/**
		 *	@return the texture associated with this layer
		 */
		virtual nap::BaseTexture2D&			getTexture() override { return mLayer->getTexture(mCurrentIndex); }

		/**
		 *	@return const texture associated with this layer
		 */
		virtual const nap::BaseTexture2D&	getTexture() const override { return mLayer->getTexture(mCurrentIndex); }


	private:
		ImageSequenceLayer* mLayer = nullptr;			///< Back pointer to the Layer resource
		double				mCurrentTime = 0.0;			///< Current playback time
		int					mCurrentIndex = 0;			///< Current playing frame index
	};
}
