/* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// Local Includes
#include "surfacedescriptor.h"

// External Includes
#include <nap/resource.h>
#include <nap/core.h>
#include <nap/signalslot.h>

#include <renderservice.h>
#include <bitmap.h>
#include <rect.h>
#include <perspcameracomponent.h>
#include <utility/threading.h>

namespace nap
{
	/**
	 * The Snapshot module renders objects at any given resolution and format (as long as it is supported by the hardware)
	 * and saves the result to a specified location on disk.
	 *
	 * Please take into account the endianness of your system. This means e.g. rendering to a BGRA format on Windows and 
	 * Linux (big-endian), and RGBA on OSX. One exception to this rule is when rendering using 16-bit RGBA color channels: 
	 * in this case FreeImage automatically carries over the correct channel ordering information with platform-specific macros.
	 */
	class NAPAPI Snapshot : public Resource
	{
		RTTI_ENABLE(Resource)
	public:

		/**
		* All supported output extensions for snapshots
		*/
		enum class EOutputExtension
		{
			PNG,					///< Portable Network Graphics (*.PNG)
			JPG,					///< Independent JPEG Group (*.JPG, *.JIF, *.JPEG, *.JPE)
			TIFF,					///< Tagged Image File Format (*.TIF, *.TIFF)
			BMP						///< Windows or OS/2 Bitmap File (*.BMP)
		};

		Snapshot(Core& core);
		virtual ~Snapshot();

		/**
		* Initialize this object after de-serialization
		* @param errorState contains the error message when initialization fails
		*/
		virtual bool init(utility::ErrorState& errorState) override;

		/**
		* Take a high-res snapshot of the scene and save to the configured location on disk
		* @param camera Camera to take snapshot with
		* @param comps Components to render
		*/
		bool takeSnapshot(PerspCameraComponentInstance& camera, std::vector<RenderableComponentInstance*>& comps);


		uint32_t mWidth = 0;													///< Property: 'Width' width of the snapshot in texels
		uint32_t mHeight = 0;													///< Property: 'Height' height of the snapshot in texels
		uint32_t mNumRows = 0;													///< Property: 'NumRows' desired width of a cell
		uint32_t mNumColumns = 0;												///< Property: 'NumColumns' desired height of a cell

		glm::vec4 mClearColor = glm::vec4(0.f, 0.f, 0.f, 1.f);					///< Property: 'ClearColor' color selection used for clearing the render target
		RenderTexture2D::EFormat mFormat = RenderTexture2D::EFormat::RGBA8;		///< Property: 'Format' texture format
		ERasterizationSamples mRequestedSamples = ERasterizationSamples::One;	///< Property: 'Samples' The number of samples used during Rasterization. For better results turn on 'SampleShading'.
		bool mSampleShading = false;											///< Property: 'SampleShading' Reduces texture aliasing when enabled, at higher computational cost.
		bool mStitch = true;													///< Property: 'Stitch' Enable stitching

		std::string mOutputDir = "";											///< Property: 'OutputPath' Location of the directory where snapshots are saved to.
		EOutputExtension mOutputExtension = EOutputExtension::PNG;				///< Property: 'OutputExtension' Extension of the snapshot image file.

	protected:
		RenderService* mRenderService = nullptr;

	private:
		bool stitchAndSaveBitmaps();

		nap::Signal<> onBitmapsUpdated;

		std::vector<rtti::ObjectPtr<RenderTarget>> mRenderTargets;
		std::vector<rtti::ObjectPtr<Bitmap>> mBitmaps;
		std::vector<bool> mBitmapUpdateFlags;

		uint32_t mNumCells = 0;
	};
}
