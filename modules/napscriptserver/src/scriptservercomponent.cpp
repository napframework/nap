#include "scriptservercomponent.h"

RTTI_DEFINE(nap::RpcService)


namespace nap
{
	RpcService::RPCObjectCallback::RPCObjectCallback(RpcService& server, AsyncTCPClient& client,
																Object& obj)
		: mServer(server), mClient(client), mObject(obj)
	{
		obj.nameChanged.connect(onNameChangedSlot);
        obj.childAdded.connect(onChildAddedSlot);
        obj.childRemoved.connect(onChildRemovedSlot);

        // Handle attribute changes as well
        if (obj.getTypeInfo().isKindOf<AttributeBase>()) {
            auto& attrib = *static_cast<AttributeBase*>(&obj);
            attrib.valueChanged.connect(onAttributeValueChangedSlot);
        }

        if (obj.getTypeInfo().isKindOf<InputPlugBase>()) {
            auto& plug = *static_cast<InputPlugBase*>(&obj);
            plug.connected.connect(onPlugConnectedSlot);
            plug.disconnected.connect(onPlugDisconnectedSlot);
        }


	}

    RpcService::RPCObjectCallback::~RPCObjectCallback() {
//        mObject.nameChanged.disconnect(this, &RPCObjectCallback::onNameChanged);
//        mObject.childAdded.disconnect(this, &RPCObjectCallback::onChildAdded);
//        mObject.childRemoved.disconnect(this, &RPCObjectCallback::onChildRemoved);
//
//        // Handle attribute changes as well
//        if (mObject.getTypeInfo().isKindOf<AttributeBase>()) {
//            auto& attrib = *static_cast<AttributeBase*>(&mObject);
//            attrib.valueChanged.disconnect(this, &RPCObjectCallback::onAttributeValueChanged);
//        }
    }

    void RpcService::RPCObjectCallback::onNameChanged(const std::string& name)
    {
        Logger::info("Send to '%s' nameChanged: %s", mClient.getIdent().c_str(), name.c_str());
        mServer.handleNameChanged(mClient, mObject);
    }


    void RpcService::RPCObjectCallback::onChildAdded(Object& obj) {
        mServer.handleObjectAdded(mClient, mObject, obj);
    }


    void RpcService::RPCObjectCallback::onChildRemoved(Object& obj) {
        mServer.handleObjectRemoved(mClient, obj);
    }


    void RpcService::RPCObjectCallback::onAttributeValueChanged(AttributeBase& attrib) {
        mServer.handleAttributeValueChanged(mClient, attrib);
    }


    void RpcService::RPCObjectCallback::onPlugConnected(InputPlugBase& plug) {
        mServer.handlePlugConnected(mClient, plug);
    }


    void RpcService::RPCObjectCallback::onPlugDisconnected(InputPlugBase& plug) {
        mServer.handlePlugDisconnected(mClient, plug);
    }


    RpcService::RpcService()
	{
        setFlag(Editable, false);
        running.valueChangedSignal.connect([&](const bool& running) { onRunningChanged(running); });

		mServer.clientConnected.connect([&](AsyncTCPClient& client) { onClientConnected(client); });
		mServer.clientDisconnected.connect([&](AsyncTCPClient& client) { onClientDisconnected(client); });
		mServer.requestReceived.connect(
				[&](AsyncTCPClient& client, const std::string& msg) { onRequestReceived(client, msg); });
	}


	void RpcService::onRunningChanged(const bool& running)
	{
		//stopServer();

		if (running)
			startServer();
	}


	void RpcService::stopServer()
	{
		assert(false);
		//		mJsonServer->clientConnected.disconnect(onClientConnectedSlot);
		//		mJsonServer->clientDisconnected.disconnect(onClientDisconnectedSlot);
		//		mJsonServer->requestReceived.disconnect(onRequestReceivedSlot);
		//
		//		mJsonServer = nullptr;
	}


	void RpcService::startServer()
	{
		mServer.runServer(rpcPort.getValue());
	}


	Object* RpcService::resolvePath(const std::string& path)
	{
		Object* obj = ObjectPath(path).resolve(*getRootObject());
		if (!obj) {
			Logger::warn("Failed to resolve path '%s'", path.c_str());
			return nullptr;
		}
		return obj;
	}


	void RpcService::onClientConnected(AsyncTCPClient& client)
	{
		Logger::debug("Client '%s' connected", client.getIdent().c_str());
	}


	void RpcService::onClientDisconnected(AsyncTCPClient& client)
	{
		Logger::debug("Client '%s' disconnected", client.getIdent().c_str());
		removeCallbacks(client);
	}


	void RpcService::onRequestReceived(AsyncTCPClient& client, const std::string& msg)
	{
		Logger::debug("Client '%s' requested: %s", client.getIdent().c_str(), msg.c_str());
		client.enqueueEvent(evalScript(msg));
	}


	void RpcService::addCallbacks(AsyncTCPClient& client, ObjPtr ptr)
	{
        Object* obj = fromPtr<Object>(ptr);
		Logger::debug("Adding callbacks for client '%s': %s", client.getIdent().c_str(), obj->getName().c_str());
		if (!obj)
			return;

		std::unique_ptr<RPCObjectCallback> callback = std::make_unique<RPCObjectCallback>(*this, client, *obj);
		auto& map = getCallbackMap(client);
		map.emplace(obj, std::move(callback));
	}

	void RpcService::addCallbacks(const std::string &clientIdent, ObjPtr ptr) {
		AsyncTCPClient* client = getServer().getClient(clientIdent);
		if (!client)
			return;

		addCallbacks(*client, ptr);
	}



	void RpcService::removeCallbacks(AsyncTCPClient& client)
	{
		Logger::debug("Removing callbacks for client '%s'", client.getIdent().c_str());
		const auto& it = mCallbacks.find(&client);
		if (it == mCallbacks.end())
			return;

		mCallbacks.erase(it);
	}


	void RpcService::removeCallbacks(AsyncTCPClient& client, const std::string& path)
	{
		Object* obj = resolvePath(path);
		if (!obj)
			return;

		CallbackMap& map = getCallbackMap(client);
		const auto& it = map.find(obj);
		if (it == map.end())
			return;
		map.erase(it);
	}


	RpcService::CallbackMap& RpcService::getCallbackMap(AsyncTCPClient& client)
	{
		const auto& itMap = mCallbacks.find(&client);
		if (itMap != mCallbacks.end())
			return itMap->second;

		mCallbacks.emplace(&client, CallbackMap());
		return mCallbacks.find(&client)->second;
	}

}