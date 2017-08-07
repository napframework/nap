// Local Includes
#include <rtti/pythonmodule.h>
#include "logger.h"
#include "service.h"

RTTI_DEFINE_BASE(nap::Service)

namespace nap
{
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
}