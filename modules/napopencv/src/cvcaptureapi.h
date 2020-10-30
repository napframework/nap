/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

// External Includes
#include <opencv2/videoio.hpp>

namespace nap
{
	/**
	 * All available OpenCV video capture backends.
	 */
	enum class ECVCaptureAPI : int
	{
		Auto			= 0,			///< Auto detect == 0
		V4L				= 200,			///< V4L/V4L2 capturing support};
		FireWire		= 300,			///< IEEE 1394 drivers
		DirectShow		= 700,			///< DirectShow (via videoInput)
		PvAPI			= 800,			///< PvAPI, Prosilica GigE SDK
		OpenNNI			= 900,			///< OpenNI (for Kinect)
		OpenNNIAsus		= 910,			///< OpenNI (for Asus Xtion)
		Android			= 1000,         ///< Android - not used
		XIMEA			= 1100,			///< XIMEA Camera API
		AVFoundation	= 1200,			///< AVFoundation framework for iOS (OS X Lion will have the same API)
		Giganetix		= 1300,			///< Smartek Giganetix GigEVisionSDK
		MicrosoftMF		= 1400,			///< Microsoft Media Foundation (via videoInput)
		WindowsRT		= 1410,			///< Microsoft Windows Runtime using Media Foundation
		RealSense		= 1500,			///< RealSense (former Intel Perceptual Computing SDK)
		OpenNNI2		= 1600,			///< OpenNI2 (for Kinect)
		OpenNNI2Asus	= 1610,			///< OpenNI2 (for Asus Xtion and Occipital Structure sensors)
		GPhoto			= 1700,			///< gPhoto2 connection
		GStreamer		= 1800,			///< GStreamer
		FFMPEG			= 1900,			///< Open and record video file or stream using the FFMPEG library
		ImageSequence	= 2000,			///< OpenCV Image Sequence (e.g. img_%02d.jpg)
		Aravis			= 2100,			///< Aravis SDK
		MotionJPEG		= 2200,			///< Built-in OpenCV MotionJPEG codec
		IntelMFX		= 2300,			///< Intel MediaSDK
		XINE			= 2400,			///< XINE engine (Linux)
	};
}