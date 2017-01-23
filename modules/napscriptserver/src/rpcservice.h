#pragma once

#include "asynctcpserver.h"
#include <nap.h>
#include <thread>


namespace nap
{
	template <typename T>
	static T* fromPtr(ObjPtr ptr)
	{
		T* obj = (T*)ptr;
		assert(obj);
		return obj;
	}

	class RpcService;

	class RPCObjectCallback
	{
	public:
		RPCObjectCallback(RpcService& server, Object& obj);
		~RPCObjectCallback();
		RpcService& mServer;

		void onNameChanged(const std::string& name);
		Slot<const std::string&> onNameChangedSlot = {this, &RPCObjectCallback::onNameChanged};

		void onChildAdded(Object& obj);
		Slot<Object&> onChildAddedSlot = {this, &RPCObjectCallback::onChildAdded};

		void onChildRemoved(Object& obj);
		Slot<Object&> onChildRemovedSlot = {this, &RPCObjectCallback::onChildRemoved};

		void onAttributeValueChanged(AttributeBase& attrib);
		Slot<AttributeBase&> onAttributeValueChangedSlot = {this, &RPCObjectCallback::onAttributeValueChanged};

		void onPlugConnected(InputPlugBase&);
		Slot<InputPlugBase&> onPlugConnectedSlot = {this, &RPCObjectCallback::onPlugConnected};

		void onPlugDisconnected(InputPlugBase&);
		Slot<InputPlugBase&> onPlugDisconnectedSlot = {this, &RPCObjectCallback::onPlugDisconnected};

	private:
		Object& mObject;
	};

	/**
	 *  This component will run a listening server receiving script calls and dispatch notifications to any remote
	 * listeners.
	 */
	class RpcService : public Service
	{
		RTTI_ENABLE_DERIVED_FROM(Service)
	public:
		using CallbackMap = std::map<Object*, std::unique_ptr<RPCObjectCallback>>;
		using ClientCallbackMap = std::map<AsyncTCPClient*, CallbackMap>;

		RpcService();

		virtual void run() = 0;

		Attribute<int>  rpcPort = {this, "rpcPort", 8888};
		Attribute<bool> running = {this, "running", false};
		Attribute<bool> broadcastChanges = { this, "broadcastChanges", true };
        Attribute<bool> threaded = { this, "threaded", true };
		Attribute<bool> manual = { this, "manual", false };		// TODO: Deprecate, make this automatic or based on enum

		virtual std::string evalScript(const std::string& cmd) = 0;
		virtual void handleLogMessage(LogMessage& msg) = 0;
		virtual void handleNameChanged(Object& obj) = 0;
		virtual void handleObjectAdded(Object& obj, Object& child) = 0;
		virtual void handleObjectRemoved(Object& child) = 0;
		virtual void handleAttributeValueChanged(AttributeBase& attrib) = 0;
		virtual void handlePlugConnected(InputPlugBase& plug) = 0;
		virtual void handlePlugDisconnected(InputPlugBase& plug) = 0;

        const std::string& getSessionID() const { return mSessionID; }

		/**
		* Trigger this to process server messages
		*/
		SignalAttribute update = { this, "update" };

		AsyncTCPServer mServer;

	protected:
		AsyncTCPServer& getServer() { return mServer; }

		void stopServer();
		void startServer();

		Object* resolvePath(const std::string& path);

		template <typename T>
		T* resolve(const std::string& path)
		{
			auto obj = resolvePath(path);
			if (!obj)
				return nullptr;

			if (!obj->getTypeInfo().isKindOf<T>()) {
				Logger::warn("Object is not of expected type '%s': '%s'", RTTI_OF(T).getName().c_str(), path.c_str());
				return nullptr;
			}
			return static_cast<T*>(obj);
		}

	private:
		void onRunningChanged(const bool& running);

		void onClientConnected(AsyncTCPClient& client);
		void onClientDisconnected(AsyncTCPClient& client);
		void onRequestReceived(AsyncTCPClient& client, const std::string& msg);

	private:
		std::vector<std::unique_ptr<RPCObjectCallback>> mCallbacks;
        std::string mSessionID;
	};
}
RTTI_DECLARE_BASE(nap::RpcService)
