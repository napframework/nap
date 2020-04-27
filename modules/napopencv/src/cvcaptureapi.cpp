#include "cvcaptureapi.h"

#include <rtti/typeinfo.h>

RTTI_BEGIN_ENUM(nap::ECVCaptureAPI)
	RTTI_ENUM_VALUE(nap::ECVCaptureAPI::Auto,			"Automatic"),
	RTTI_ENUM_VALUE(nap::ECVCaptureAPI::Android,		"Android"),
	RTTI_ENUM_VALUE(nap::ECVCaptureAPI::Aravis,			"Aravis"),
	RTTI_ENUM_VALUE(nap::ECVCaptureAPI::AVFoundation,	"AVFoundation"),
	RTTI_ENUM_VALUE(nap::ECVCaptureAPI::DirectShow,		"DirectShow"),
	RTTI_ENUM_VALUE(nap::ECVCaptureAPI::FFMPEG,			"FFMPEG"),
	RTTI_ENUM_VALUE(nap::ECVCaptureAPI::FireWire,		"FireWire"),
	RTTI_ENUM_VALUE(nap::ECVCaptureAPI::Giganetix,		"Giganetix"),
	RTTI_ENUM_VALUE(nap::ECVCaptureAPI::GPhoto,			"GPhoto"),
	RTTI_ENUM_VALUE(nap::ECVCaptureAPI::GStreamer,		"GStreamer"),
	RTTI_ENUM_VALUE(nap::ECVCaptureAPI::ImageSequence,	"ImageSequence"),
	RTTI_ENUM_VALUE(nap::ECVCaptureAPI::IntelMFX,		"IntelMFX"),
	RTTI_ENUM_VALUE(nap::ECVCaptureAPI::MicrosoftMF,	"MicrosoftMediaFoundation"),
	RTTI_ENUM_VALUE(nap::ECVCaptureAPI::MotionJPEG,		"MotionJPEG"),
	RTTI_ENUM_VALUE(nap::ECVCaptureAPI::OpenNNI,		"OpenNI1 (Kinect)"),
	RTTI_ENUM_VALUE(nap::ECVCaptureAPI::OpenNNI2,		"OpenNI2 (Kinect)"),
	RTTI_ENUM_VALUE(nap::ECVCaptureAPI::OpenNNIAsus,	"OpenNI1 (Asus Xtion)"),
	RTTI_ENUM_VALUE(nap::ECVCaptureAPI::OpenNNI2Asus,	"OpenNI2 (Asus Xtion)"),
	RTTI_ENUM_VALUE(nap::ECVCaptureAPI::PvAPI,			"PvAPI"),
	RTTI_ENUM_VALUE(nap::ECVCaptureAPI::RealSense,		"RealSense"),
	RTTI_ENUM_VALUE(nap::ECVCaptureAPI::V4L,			"V4L/V4L2"),
	RTTI_ENUM_VALUE(nap::ECVCaptureAPI::WindowsRT,		"WindowsRuntime"),
	RTTI_ENUM_VALUE(nap::ECVCaptureAPI::XIMEA,			"XIMEA"),
	RTTI_ENUM_VALUE(nap::ECVCaptureAPI::XINE,			"XINE")
RTTI_END_ENUM
