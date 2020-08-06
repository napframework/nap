// Local Includes
#include "python.h"
#include "logger.h"
#include "service.h"

RTTI_DEFINE_BASE(nap::ServiceConfiguration)

RTTI_BEGIN_CLASS_NO_DEFAULT_CONSTRUCTOR(nap::Service)
	RTTI_CONSTRUCTOR(nap::ServiceConfiguration*)
RTTI_END_CLASS

namespace nap
{
	Service::Service(ServiceConfiguration* configuration) :
		mConfiguration(configuration)
	{
	}

	/**
	@brief Destructor
	**/
	Service::~Service()
	{}


	/**
	@brief Returns the core in which service lives
	**/
	Core& Service::getCore()
	{
		assert(mCore);
		return *mCore;
	}

	const std::string Service::getTypeName() const
	{
		return rtti::TypeInfo::get(*this).get_name().data();
	}

	ServiceConfiguration::~ServiceConfiguration()
	{
		nap::Logger::info("Kakkerdekak");
	}
}