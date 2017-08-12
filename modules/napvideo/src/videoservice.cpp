/// local includes
#include <videoservice.h>

extern "C"
{
	#include <libavcodec/avcodec.h>
	#include <libavformat/avformat.h>
}

RTTI_BEGIN_CLASS(nap::VideoService)
RTTI_END_CLASS

namespace nap
{
	bool VideoService::init(nap::utility::ErrorState& errorState)
	{
		av_register_all();
		avcodec_register_all();
		return true;
	}
}

