#pragma once

// internal includes
#include "layer.h"

// external includes
#include <nap/signalslot.h>

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
		 *	@return The pixmap associated with this layer at the specified index
		 */
		nap::Pixmap& getPixmap(int index) { return *mPixmaps[index]; }

		/**
		 *	@return Const pixmap associated with this layer at the specified index
		 */
		const nap::Pixmap&	getPixmap(int index) const { return *mPixmaps[index]; }

		/**
		 * @return the length of the sequence in seconds
		 */
		float getLength() const;

		/**
		* @return Number of pixmaps in this sequence
		*/
		int getNumPixmaps() const { return (int)mPixmaps.size(); }

		/**
		* @return The settings that should be used to create the texture used to display the images in this sequence
		*/
		const opengl::Texture2DSettings& getTextureSettings() const { return mTextureSettings; }

	protected:
		/**
		 *	@return Instance object for this ImageSequenceLayer
		 */
		virtual std::unique_ptr<LayerInstance> createInstance() override;

	public:
		std::string								mBaseFilename;		///< The base filename used to find all images for this sequence. Must contain %[0#]d format specifier
		float									mFPS = 30.0f;		///< Playback framerate for this sequence

	private:
		std::vector<std::unique_ptr<Pixmap>>	mPixmaps;			///< The images created from the files found on disk
		opengl::Texture2DSettings				mTextureSettings;	///< The texture settings used create the texture. Inferred from the pixmaps in this sequence
	};

	/**
	 * Instance object for ImageSequenceLayer. Plays back the sequence with the given framerate. Current texture can be retrieved through getTexture().
	 */
	class NAPAPI ImageSequenceLayerInstance : public LayerInstance
	{
		RTTI_ENABLE(LayerInstance)
	public:
		ImageSequenceLayerInstance(ImageSequenceLayer& layer);

		/**
		 *	Update the animation of this sequence.
		 */
		virtual void update(double deltaTime) override;
		
		/**
		 *	@return the total amount of images in the sequence of the layer
		 */
		int getNumPixmaps() const												{ return mLayer->getNumPixmaps(); }

		/**
		 *	@return the length of the image sequence in seconds
		 */
		float getLength() const													{ return mLayer->getLength(); }

		/**
		 *	@return The texture associated with this layer
		 */
		virtual nap::Texture2D&			getTexture() override				{ return *mCurrentFrameTexture; }

		/**
		 *	@return Const texture associated with this layer
		 */
		virtual const nap::Texture2D&	getTexture() const override			{ return *mCurrentFrameTexture; }

		// Signal that is triggered when the sequence completed
		nap::Signal<ImageSequenceLayerInstance&> completed;

	private:
		ImageSequenceLayer*				mLayer = nullptr;			///< Back pointer to the Layer resource
		double							mCurrentTime = 0.0;			///< Current playback time
		int								mCurrentFrameIndex = -1;	///< Current playing frame index
		int								mNextFrameIndex = 0;		///< Holds the next frame index
		std::unique_ptr<Texture2D>	mCurrentFrameTexture;		///< Current GPU texture (updated whenever the frame change)
	};
}
