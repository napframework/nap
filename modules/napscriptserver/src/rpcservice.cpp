#include "rpcservice.h"

RTTI_DEFINE(nap::RpcService)




namespace nap
{
    std::string createSessionID() {
        using namespace std::chrono;
        auto now = duration_cast<milliseconds>(steady_clock::now().time_since_epoch());
        return std::to_string(now.count());
    }


	RPCObjectCallback::RPCObjectCallback(RpcService &server, Object &obj)
		: mServer(server), mObject(obj)
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

    RPCObjectCallback::~RPCObjectCallback() {
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

    void RPCObjectCallback::onNameChanged(const std::string& name)
    {
        mServer.handleNameChanged(mObject);
    }


    void RPCObjectCallback::onChildAdded(Object& obj) {
        mServer.handleObjectAdded(mObject, obj);
    }


    void RPCObjectCallback::onChildRemoved(Object& obj) {
        mServer.handleObjectRemoved(obj);
    }


    void RPCObjectCallback::onAttributeValueChanged(AttributeBase& attrib) {
        mServer.handleAttributeValueChanged(attrib);
    }


    void RPCObjectCallback::onPlugConnected(InputPlugBase& plug) {
        mServer.handlePlugConnected(plug);
    }


    void RPCObjectCallback::onPlugDisconnected(InputPlugBase& plug) {
        mServer.handlePlugDisconnected(plug);
    }


    RpcService::RpcService() : mSessionID(createSessionID())
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
	}


	void RpcService::startServer()
	{
        mServer.runServer(rpcPort.getValue(), threaded.getValue());
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
	}


	void RpcService::onRequestReceived(AsyncTCPClient& client, const std::string& msg)
	{
		Logger::debug("Client '%s' requested: %s", client.getIdent().c_str(), msg.c_str());
        std::string result = evalScript(msg);
		client.enqueueEvent(result);
	}



}