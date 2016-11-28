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

        virtual void handleLogMessage(AsyncTCPClient& client, LogMessage& msg) override;
        void handleNameChanged(AsyncTCPClient& client, Object& obj) override;
        void handleObjectAdded(AsyncTCPClient& client, Object& obj, Object& child) override;
        void handleObjectRemoved(AsyncTCPClient& client, Object& child) override;
        virtual void handleAttributeValueChanged(AsyncTCPClient& client, AttributeBase& attrib) override;
    private:
        void startRPCSocket();

        Slot<LogMessage> onLogSlot = { this, &JSONRPCServerComponent::onLog };
        void onLog(LogMessage msg);

    private:
        jsonrpc::Server mJsonServer;
        jsonrpc::JsonFormatHandler mFormatHandler;
        zmq::context_t mContext;
        std::mutex mMutex;
    };
}

RTTI_DECLARE(nap::JSONRPCServerComponent)
