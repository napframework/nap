#pragma once

// Nap Includes
#include <nap/service.h>
#include <queue>

namespace nap
{
	/**
	 * 
	 */
	class NAPAPI VideoService : public Service
	{
		RTTI_ENABLE(Service)

	public:
		// Default constructor
		VideoService() = default;

		// Disable copy
		VideoService(const VideoService& that) = delete;
		VideoService& operator=(const VideoService&) = delete;
	
		bool init(nap::utility::ErrorState& errorState);
	};
}