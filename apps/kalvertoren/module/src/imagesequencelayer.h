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
		 *	@return The pixmap associated with this layer at the specified index
		 */
		nap::Pixmap& getPixmap(int index) { return *mPixmaps[index]; }

		/**
		 *	@return Const pixmap associated with this layer at the specified index
		 */
		const nap::Pixmap&	getPixmap(int index) const { return *mPixmaps[index]; }

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
		int										mFPS = 30;			///< Playback framerate for this sequence

	private:
		std::vector<std::unique_ptr<Pixmap>>	mPixmaps;			///< The images created from the files found on disk
		opengl::Texture2DSettings				mTextureSettings;	///< The texture settings used create the texture. Inferred from the pixmaps in this sequence
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
		 *	@return The texture associated with this layer
		 */
		virtual nap::BaseTexture2D&			getTexture() override { return *mCurrentFrameTexture; }

		/**
		 *	@return Const texture associated with this layer
		 */
		virtual const nap::BaseTexture2D&	getTexture() const override { return *mCurrentFrameTexture; }


	private:
		ImageSequenceLayer*				mLayer = nullptr;			///< Back pointer to the Layer resource
		double							mCurrentTime = 0.0;			///< Current playback time
		int								mCurrentFrameIndex = -1;	///< Current playing frame index
		std::unique_ptr<BaseTexture2D>	mCurrentFrameTexture;		///< Current GPU texture (updated whenever the frame changes)
	};
}
