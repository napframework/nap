#pragma once

#include <nap/service.h>
#include <nap/entity.h>

namespace nap
{
	class NAPAPI EtherDreamService : public Service
	{
		RTTI_ENABLE(Service)

	public:
		// Default Constructor
		EtherDreamService() = default;

		// Default Destructor
		virtual ~EtherDreamService() = default;
	};
}