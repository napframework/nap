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

	/**
	 *  This component will run a listening server receiving script calls and dispatch notifications to any remote
	 * listeners.
	 */
	class RpcService : public Service
	{
	RTTI_ENABLE_DERIVED_FROM(Service)
	protected:
		/**
		* Holds a slot that may receive signals and forward them to the script server
		*/
		class RPCObjectCallback
		{
		public:
			RPCObjectCallback(RpcService& server, AsyncTCPClient& client, Object& obj);
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
			AsyncTCPClient& mClient;
		};


	public:
		using CallbackMap = std::map<Object*, std::unique_ptr<RPCObjectCallback>>;
		using ClientCallbackMap = std::map<AsyncTCPClient*, CallbackMap>;

		RpcService();

		virtual void run() = 0;

		Attribute<int> rpcPort = {this, "rpcPort", 8888};
		Attribute<bool> running = {this, "running", false};

	protected:
		virtual std::string evalScript(const std::string& cmd) = 0;
        virtual void handleLogMessage(AsyncTCPClient& client, LogMessage& msg) = 0;
		virtual void handleNameChanged(AsyncTCPClient& client, Object& obj) = 0;
		virtual void handleObjectAdded(AsyncTCPClient& client, Object& obj, Object& child) = 0;
		virtual void handleObjectRemoved(AsyncTCPClient& client, Object& child) = 0;
		virtual void handleAttributeValueChanged(AsyncTCPClient& client, AttributeBase& attrib) = 0;
        virtual void handlePlugConnected(AsyncTCPClient& client, InputPlugBase& plug) = 0;
        virtual void handlePlugDisconnected(AsyncTCPClient& client, InputPlugBase& plug) = 0;
		AsyncTCPServer& getServer() { return mServer; }

		void stopServer();
		void startServer();

		void addCallbacks(AsyncTCPClient& client, ObjPtr path);
		void addCallbacks(const std::string& clientIdent, ObjPtr ptr);

		void removeCallbacks(AsyncTCPClient& client);

		void removeCallbacks(AsyncTCPClient& client, const std::string& path);

        // TODO: Move this map to the AsyncTCPClient
		CallbackMap& getCallbackMap(AsyncTCPClient& client);

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
		ClientCallbackMap mCallbacks;
		AsyncTCPServer mServer;
	};
}
RTTI_DECLARE_BASE(nap::RpcService)
