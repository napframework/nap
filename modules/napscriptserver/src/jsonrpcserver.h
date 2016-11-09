#pragma once

#include "scriptservercomponent.h"
#include <rtti/rtti.h>
#include <vector>
#include <zmq.hpp>

#include "jsonrpc-lean/server.h"


namespace nap
{

	/**
	 * JSON-RPC based server
	 */
	class JSONRPCServerComponent : public ScriptServerComponent
	{
		RTTI_ENABLE_DERIVED_FROM(ScriptServerComponent)
	public:
		JSONRPCServerComponent();

        void run() override;

    protected:
        std::string evalScript(const std::string& cmd) override;

        virtual void handleAttributeChanged(AttributeBase &attrib) override;

    private:
        void startRPCSocket();
        void startSUBSocket();
    private:
        jsonrpc::Server mServer;
        jsonrpc::JsonFormatHandler mFormatHandler;
        zmq::context_t mContext;
        std::unique_ptr<zmq::socket_t> mPubSocket;
    };
}

RTTI_DECLARE(nap::JSONRPCServerComponent)
