#pragma once

#include "scriptservercomponent.h"
#include <jsonserializer.h>
#include <rtti/rtti.h>
#include <vector>
#include <zmq.hpp>

#include "jsonrpc-lean/server.h"



namespace nap
{

	/**
	 * JSON-RPC based server
	 */
	class JsonRpcService : public RpcService
	{
		RTTI_ENABLE_DERIVED_FROM(RpcService)
	public:
		JsonRpcService();

		void run() override;

	protected:
		std::string evalScript(const std::string& cmd) override;

		virtual void handleLogMessage(AsyncTCPClient& client, LogMessage& msg) override;
		void handleNameChanged(AsyncTCPClient& client, Object& obj) override;
		void handleObjectAdded(AsyncTCPClient& client, Object& obj, Object& child) override;
		void handleObjectRemoved(AsyncTCPClient& client, Object& child) override;
		void handleAttributeValueChanged(AsyncTCPClient& client, AttributeBase& attrib) override;
		void handlePlugConnected(AsyncTCPClient& client, InputPlugBase& plug) override;
		void handlePlugDisconnected(AsyncTCPClient& client, InputPlugBase& plug) override;

	private:
		// RPC METHODS

		std::vector<std::string> rpc_getModules();
		std::vector<std::string> rpc_getDataTypes(const std::string &modname);
		std::string rpc_getModuleInfo();
		std::string rpc_getObjectTree();
		std::string rpc_copyObjectTree(ObjPtr ptr);
		void rpc_pasteObjectTree(ObjPtr parentPtr, const std::string& jsonData);
		ObjPtr rpc_getRoot();
		ObjPtr  rpc_getParent(ObjPtr objPtr);
		void rpc_addChild(ObjPtr parent, const std::string &typeName);
		void rpc_addEntity(ObjPtr parentEntity);
		std::string rpc_getName(ObjPtr ptr);
		void rpc_setName(ObjPtr ptr, const std::string &name);
		std::string rpc_getTypeName(ObjPtr ptr);
		std::string rpc_getAttributeValue(ObjPtr attribPtr);
		void rpc_setAttributeValue(ObjPtr attribPtr, const std::string& value);
		void rpc_forceSetAttributeValue(ObjPtr ptr, const std::string &attribName, const std::string &attribValue,
										const std::string &attribType);
		void rpc_addObjectCallbacks(const std::string &ident, ObjPtr ptr);
		void rpc_connectPlugs(ObjPtr srcPlugPtr, ObjPtr dstPlugPtr);
		void rpc_exportObject(ObjPtr ptr, const std::string &filename);
		void rpc_importObject(ObjPtr parentPtr, const std::string &filename);
		void rpc_removeObject(ObjPtr ptr);


	private:
		void startRPCSocket();

		Slot<LogMessage> onLogSlot = {this, &JsonRpcService::onLog};
		void onLog(LogMessage msg);

	private:
		jsonrpc::Server mJsonServer;
		jsonrpc::JsonFormatHandler mFormatHandler;
		zmq::context_t mContext;
		std::mutex mMutex;
	};
}

RTTI_DECLARE(nap::JsonRpcService)
