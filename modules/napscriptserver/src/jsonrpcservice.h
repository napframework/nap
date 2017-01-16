#pragma once

#include "rpcservice.h"
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

		void initialized() override;

	protected:
		std::string evalScript(const std::string& cmd) override;

		void handleLogMessage(LogMessage& msg) override;
		void handleNameChanged(Object& obj) override;
		void handleObjectAdded(Object& obj, Object& child) override;
		void handleObjectRemoved(Object& child) override;
		void handleAttributeValueChanged(AttributeBase& attrib) override;
		void handlePlugConnected(InputPlugBase& plug) override;
		void handlePlugDisconnected(InputPlugBase& plug) override;

	private:
		// RPC METHODS

		std::string rpc_getModuleInfo();
		std::string rpc_getObjectTree();
		std::string rpc_copyObjectTree(ObjPtr ptr);
		void rpc_pasteObjectTree(ObjPtr parentPtr, const std::string& jsonData);
		void rpc_addChild(ObjPtr parent, const std::string& typeName);
		void rpc_addEntity(ObjPtr parentEntity);
		void rpc_setName(ObjPtr ptr, const std::string& name);
		void rpc_setAttributeValue(ObjPtr attribPtr, const std::string& value);
		void rpc_forceSetAttributeValue(ObjPtr ptr, const std::string& attribName, const std::string& attribValue, const std::string& attribType);
		void rpc_connectPlugs(ObjPtr srcPlugPtr, ObjPtr dstPlugPtr);
        void rpc_disconnectPlug(ObjPtr srcPlugPtr);
		void rpc_exportObject(ObjPtr ptr, const std::string& filename);
		void rpc_importObject(ObjPtr parentPtr, const std::string& filename);
		void rpc_removeObject(ObjPtr ptr);
		void rpc_loadFile(const std::string& filename);
        void rpc_triggerSignalAttribute(ObjPtr ptr);

		void rpc_addObjectCallbacks(const std::string& ident, ObjPtr ptr);
		std::vector<std::string> rpc_getModules();
		std::vector<std::string> rpc_getDataTypes(const std::string& modname);
		ObjPtr rpc_getRoot();
		ObjPtr rpc_getParent(ObjPtr objPtr);
		std::string rpc_getName(ObjPtr ptr);
		std::string rpc_getTypeName(ObjPtr ptr);
		std::string rpc_getAttributeValue(ObjPtr attribPtr);


	private:
		void startRPCSocket();

		Slot<LogMessage> onLogSlot = {this, &JsonRpcService::onLog};
		void onLog(LogMessage msg);

		/**
		 * Register callback handlers for the provided object tree.
		 * @param obj
		 */
		void addCallbacks(Object* obj);

		void enqueueEvent(const std::string& msg);

	private:
		jsonrpc::Server mJsonServer;
		jsonrpc::JsonFormatHandler mFormatHandler;
		zmq::context_t mContext;
		std::mutex mMutex;
	};
}

RTTI_DECLARE(nap::JsonRpcService)
