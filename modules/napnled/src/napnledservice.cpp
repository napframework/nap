#include "napnledservice.h"
#include <nap/logger.h>
#include <nledservercommands.h>
#include <assert.h>
#include "napnledpanel.h"

// using directives
using namespace asio::detail::socket_ops;

namespace nap
{
	// Stop running
	NLedService::~NLedService()
	{
		mClient.stop();
	}


	// Start running
	void NLedService::start()
	{
		mClient.setServerName(serverName.getValue());
		mClient.setPortNumber(portNumber.getValue());
		mClient.start();
	}


	// Stop running
	void NLedService::stop()
	{
		mClient.stop();
	}


	// All types that should be registered with this service
	void NLedService::registerTypes(nap::Core& core)
	{
		core.registerType(*this, RTTI_OF(NledPanelComponent));
	}

}

RTTI_DEFINE(nap::NLedService)