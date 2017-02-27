#include "napnledservice.h"
#include <nap/logger.h>
#include <nledservercommands.h>
#include <assert.h>

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

}

RTTI_DEFINE(nap::NLedService)