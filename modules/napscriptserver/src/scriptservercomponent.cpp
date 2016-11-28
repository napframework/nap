#include "scriptservercomponent.h"

RTTI_DEFINE(nap::ScriptServerComponent)


namespace nap
{
	ScriptServerComponent::RPCObjectCallback::RPCObjectCallback(ScriptServerComponent& server, AsyncTCPClient& client,
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
	}

    ScriptServerComponent::RPCObjectCallback::~RPCObjectCallback() {
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

    void ScriptServerComponent::RPCObjectCallback::onNameChanged(const std::string& name)
    {
        Logger::info("Send to '%s' nameChanged: %s", mClient.getIdent().c_str(), name.c_str());
        mServer.handleNameChanged(mClient, mObject);
    }


    void ScriptServerComponent::RPCObjectCallback::onChildAdded(Object& obj) {
        mServer.handleObjectAdded(mClient, mObject, obj);
    }


    void ScriptServerComponent::RPCObjectCallback::onChildRemoved(Object& obj) {
        mServer.handleObjectRemoved(mClient, obj);
    }


    void ScriptServerComponent::RPCObjectCallback::onAttributeValueChanged(AttributeBase& attrib) {
        mServer.handleAttributeValueChanged(mClient, attrib);
    }


    ScriptServerComponent::ScriptServerComponent()
	{
        setFlag(Editable, false);
        running.valueChangedSignal.connect([&](const bool& running) { onRunningChanged(running); });
	}


	void ScriptServerComponent::onRunningChanged(const bool& running)
	{
		if (mServer) {
			stopServer();
		}

		if (running)
			startServer();
	}


	void ScriptServerComponent::stopServer()
	{
		assert(false);
		//		mJsonServer->clientConnected.disconnect(onClientConnectedSlot);
		//		mJsonServer->clientDisconnected.disconnect(onClientDisconnectedSlot);
		//		mJsonServer->requestReceived.disconnect(onRequestReceivedSlot);
		//
		//		mJsonServer = nullptr;
	}


	void ScriptServerComponent::startServer()
	{
		mServer = std::make_unique<AsyncTCPServer>(rpcPort.getValue());
		mServer->clientConnected.connect([&](AsyncTCPClient& client) { onClientConnected(client); });
		mServer->clientDisconnected.connect([&](AsyncTCPClient& client) { onClientDisconnected(client); });
		mServer->requestReceived.connect(
			[&](AsyncTCPClient& client, const std::string& msg) { onRequestReceived(client, msg); });
	}


	Object* ScriptServerComponent::resolvePath(const std::string& path)
	{
		Object* obj = ObjectPath(path).resolve(*getRootObject());
		if (!obj) {
			Logger::warn("Failed to resolve path '%s'", path.c_str());
			return nullptr;
		}
		return obj;
	}


	void ScriptServerComponent::onClientConnected(AsyncTCPClient& client)
	{
		Logger::debug("Client '%s' connected", client.getIdent().c_str());
	}


	void ScriptServerComponent::onClientDisconnected(AsyncTCPClient& client)
	{
		Logger::debug("Client '%s' disconnected", client.getIdent().c_str());
		removeCallbacks(client);
	}


	void ScriptServerComponent::onRequestReceived(AsyncTCPClient& client, const std::string& msg)
	{
		Logger::debug("Client '%s' requested: %s", client.getIdent().c_str(), msg.c_str());
		client.enqueueEvent(evalScript(msg));
	}


	void ScriptServerComponent::addCallbacks(AsyncTCPClient& client, ObjPtr ptr)
	{
        Object* obj = fromPtr<Object>(ptr);
		Logger::debug("Adding callbacks for client '%s': %s", client.getIdent().c_str(), obj->getName().c_str());
		if (!obj)
			return;

		std::unique_ptr<RPCObjectCallback> callback = std::make_unique<RPCObjectCallback>(*this, client, *obj);
		auto& map = getCallbackMap(client);
		map.emplace(obj, std::move(callback));
	}

	void ScriptServerComponent::addCallbacks(const std::string &clientIdent, ObjPtr ptr) {
		AsyncTCPClient* client = getServer().getClient(clientIdent);
		if (!client)
			return;

		addCallbacks(*client, ptr);
	}



	void ScriptServerComponent::removeCallbacks(AsyncTCPClient& client)
	{
		Logger::debug("Removing callbacks for client '%s'", client.getIdent().c_str());
		const auto& it = mCallbacks.find(&client);
		if (it == mCallbacks.end())
			return;

		mCallbacks.erase(it);
	}


	void ScriptServerComponent::removeCallbacks(AsyncTCPClient& client, const std::string& path)
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


	ScriptServerComponent::CallbackMap& ScriptServerComponent::getCallbackMap(AsyncTCPClient& client)
	{
		const auto& itMap = mCallbacks.find(&client);
		if (itMap != mCallbacks.end())
			return itMap->second;

		mCallbacks.emplace(&client, CallbackMap());
		return mCallbacks.find(&client)->second;
	}

}