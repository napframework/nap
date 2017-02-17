#include "jsonrpcservice.h"
#include <fstream>
#include <jsonserializer.h>
#include <rapidjson/prettywriter.h>


RTTI_DEFINE(nap::JsonRpcService)



namespace nap
{

	static std::vector<std::string> toStringList(const std::vector<RTTI::TypeInfo>& types)
	{
		std::vector<std::string> typenames;
		for (const auto& type : types)
			typenames.push_back(type.getName());
		return typenames;
	}


	static bool s_send(zmq::socket_t& socket, const std::string& string)
	{

		zmq::message_t message(string.size());
		memcpy(message.data(), string.data(), string.size());

		bool rc = socket.send(message);
		return (rc);
	}


	JsonRpcService::JsonRpcService() : mContext(1), RpcService()
	{

		Logger::instance().log.connect(onLogSlot);

		mJsonServer.RegisterFormatHandler(mFormatHandler);

		auto& disp = mJsonServer.GetDispatcher();

		disp.AddMethod("getModuleInfo", &JsonRpcService::rpc_getModuleInfo, *this);
		disp.AddMethod("getObjectTree", &JsonRpcService::rpc_getObjectTree, *this);
		disp.AddMethod("copyObjectTree", &JsonRpcService::rpc_copyObjectTree, *this);
		disp.AddMethod("pasteObjectTree", &JsonRpcService::rpc_pasteObjectTree, *this);
		disp.AddMethod("addChild", &JsonRpcService::rpc_addChild, *this);
		disp.AddMethod("addEntity", &JsonRpcService::rpc_addEntity, *this);
		disp.AddMethod("setName", &JsonRpcService::rpc_setName, *this);
		disp.AddMethod("setAttributeValue", &JsonRpcService::rpc_setAttributeValue, *this);
		disp.AddMethod("forceSetAttributeValue", &JsonRpcService::rpc_forceSetAttributeValue, *this);
		disp.AddMethod("connectPlugs", &JsonRpcService::rpc_connectPlugs, *this);
        disp.AddMethod("disconnectPlug", &JsonRpcService::rpc_disconnectPlug, *this);
		disp.AddMethod("exportObject", &JsonRpcService::rpc_exportObject, *this);
		disp.AddMethod("importObject", &JsonRpcService::rpc_importObject, *this);
		disp.AddMethod("removeObject", &JsonRpcService::rpc_removeObject, *this);
        disp.AddMethod("loadFile", &JsonRpcService::rpc_loadFile, *this);
        disp.AddMethod("triggerSignalAttribute", &JsonRpcService::rpc_triggerSignalAttribute, *this);
		//		disp.AddMethod("getModules", &JsonRpcService::rpc_getModules, *this);
		//		disp.AddMethod("getDataTypes", &JsonRpcService::rpc_getDataTypes, *this);
		//		disp.AddMethod("getRoot", &JsonRpcService::rpc_getRoot, *this);
		//		disp.AddMethod("getParent", &JsonRpcService::rpc_getParent, *this);
		//		disp.AddMethod("getName", &JsonRpcService::rpc_getName, *this);
		//		disp.AddMethod("getTypeName", &JsonRpcService::rpc_getTypeName, *this);
		//		disp.AddMethod("getAttributeValue", &JsonRpcService::rpc_getAttributeValue, *this);
		//		disp.AddMethod("addObjectCallbacks", &JsonRpcService::rpc_addObjectCallbacks, *this);

	}

	void JsonRpcService::initialized() {
		addCallbacks(&getCore().getRoot());
	}


	std::string JsonRpcService::evalScript(const std::string& cmd)
	{
		std::lock_guard<std::mutex> lock(mMutex);
        auto reply = mJsonServer.HandleRequest(cmd);
		return reply->GetData();
	}


	void JsonRpcService::run()
	{
		startRPCSocket();
	}

	std::string callbackJSON(const std::string& id, const std::string& value)
	{
		using namespace rapidjson;
		StringBuffer buf;
		PrettyWriter<StringBuffer> w(buf);
		w.StartObject();

		w.String("jsonrpc");
		w.String("2.0");

		w.String("id");
		w.String(id.c_str());

		w.String("result");
		w.String(value.c_str());

		w.EndObject();

		return buf.GetString();
	}

	void JsonRpcService::handleLogMessage(LogMessage& msg)
	{
		using namespace rapidjson;
		StringBuffer buf;
		PrettyWriter<StringBuffer> w(buf);
		w.StartObject();
		{
			w.String("level");
			w.Int(msg.level().level());
			w.String("levelName");
			w.String(msg.level().name().c_str());
			w.String("text");
			w.String(msg.text().c_str());
		}
		w.EndObject();
		enqueueEvent(callbackJSON("log", buf.GetString()));
	}

	void JsonRpcService::handleNameChanged(Object& obj)
	{
		using namespace rapidjson;
		StringBuffer buf;
		PrettyWriter<StringBuffer> w(buf);
		w.StartObject();
		{
			w.String("ptr");
			w.Int64(Serializer::toPtr(obj));
			w.String("name");
			w.String(obj.getName().c_str());
		}
		w.EndObject();

		enqueueEvent(callbackJSON("nameChanged", buf.GetString()));
	}

	void JsonRpcService::handleAttributeValueChanged(AttributeBase& attrib)
	{
		std::string value;
		attrib.toString(value);

		using namespace rapidjson;
		StringBuffer buf;
		PrettyWriter<StringBuffer> w(buf);

		w.StartObject();
		{
			w.String("ptr");
			w.Int64(Serializer::toPtr(attrib));
			w.String("name");
			w.String(attrib.getName().c_str());
			w.String("value");
			w.String(value.c_str());
		}
		w.EndObject();

		enqueueEvent(callbackJSON("attributeValueChanged", buf.GetString()));
	}


	void JsonRpcService::handleObjectAdded(Object& obj, Object& child)
	{
		using namespace rapidjson;
		StringBuffer buf;
		PrettyWriter<StringBuffer> w(buf);
		w.StartObject();
		{
			w.String("ptr");
			w.Int64(Serializer::toPtr(obj));
			w.String("child");
			w.String(JSONSerializer().toString(child, true).c_str());
		}
		w.EndObject();
		enqueueEvent(callbackJSON("objectAdded", buf.GetString()));
	}


	void JsonRpcService::handleObjectRemoved(Object& child)
	{
		using namespace rapidjson;
		StringBuffer buf;
		PrettyWriter<StringBuffer> w(buf);
		w.StartObject();
		{
			w.String("ptr");
			w.Int64(Serializer::toPtr(child));
		}
		w.EndObject();
		enqueueEvent(callbackJSON("objectRemoved", buf.GetString()));
	}


	void JsonRpcService::handlePlugConnected(InputPlugBase& plug)
	{
		assert(plug.isConnected());

		using namespace rapidjson;
		StringBuffer buf;
		PrettyWriter<StringBuffer> w(buf);
		w.StartObject();
		{
			w.String("srcPtr");
			w.Int64(Serializer::toPtr(plug.getConnection()));
			w.String("dstPtr");
			w.Int64(Serializer::toPtr(plug));
		}
		w.EndObject();
		enqueueEvent(callbackJSON("plugConnected", buf.GetString()));
	}


	void JsonRpcService::handlePlugDisconnected(InputPlugBase& plug)
	{
		assert(plug.isConnected());

		using namespace rapidjson;
		StringBuffer buf;
		PrettyWriter<StringBuffer> w(buf);
		w.StartObject();
		{
			w.String("srcPtr");
			w.Int64(Serializer::toPtr(plug.getConnection()));
			w.String("dstPtr");
			w.Int64(Serializer::toPtr(plug));
		}
		w.EndObject();


		enqueueEvent(callbackJSON("plugDisconnected", buf.GetString()));
	}


	void JsonRpcService::startRPCSocket()
	{ // RPC Socket
		zmq::socket_t socket(mContext, ZMQ_ROUTER);

		std::string host = "tcp://*:" + std::to_string(rpcPort.getValue());

		socket.bind(host.c_str());
		Logger::info("RPC Server: " + host);

		while (running.getValue()) {
			zmq::message_t message;
			bool received = socket.recv(&message, ZMQ_NOBLOCK);
			if (received) {
				std::string msg = std::string(static_cast<char*>(message.data()), message.size()).c_str();
				std::cout << msg.size() << std::endl;
				if (msg.size() > 0) {
					Logger::debug("recv: '%s'", msg.c_str());
					std::string reply = evalScript(msg);
					Logger::debug("send: '%s'", reply.c_str());
					s_send(socket, reply);
				}
			}

			std::this_thread::sleep_for(std::chrono::milliseconds(10));
		}
	}


	void JsonRpcService::onLog(LogMessage msg)
	{
		for (AsyncTCPClient* client : getServer().getClients())
			handleLogMessage(msg);
	}

	/////////////////// RPC METHODS /////////////////////
	/////////////////////////////////////////////////////

	std::vector<std::string> JsonRpcService::rpc_getModules()
	{
		std::vector<std::string> modulenames;
		for (auto mod : getCore().getModuleManager().getModules())
			modulenames.push_back(mod->getName());
		return modulenames;
	}

	std::vector<std::string> JsonRpcService::rpc_getDataTypes(const std::string& modname)
	{
		if (modname.empty())
			return toStringList(getCore().getModuleManager().getDataTypes());

		std::vector<std::string> typenames;
		Module* mod = getCore().getModuleManager().getModule(modname);
		if (!mod) {
			Logger::warn("Module not found: %s", modname.c_str());
			return typenames;
		}
		TypeList types;
		mod->getDataTypes(types);

		for (const auto& type : types)
			typenames.push_back(type.getName());
		return typenames;
	}

	std::string JsonRpcService::rpc_getModuleInfo()
	{
		return JSONSerializer().toString(getCore().getModuleManager());
	}

	std::string JsonRpcService::rpc_getObjectTree()
	{
		return JSONSerializer().toString(getCore().getRoot(), true);
	}

	std::string JsonRpcService::rpc_copyObjectTree(ObjPtr objPtr)
	{
		Object* obj = fromPtr<Object>(objPtr);
		if (!obj)
			return 0;
		return JSONSerializer().toString(*obj, true);
	}

	void JsonRpcService::rpc_pasteObjectTree(ObjPtr parentPtr, const std::string& jsonData)
	{
		Object* obj = fromPtr<Object>(parentPtr);
		if (!obj)
			return;
		Logger::info(jsonData);
		JSONSerializer().fromString(jsonData, getCore(), obj);
	}

	ObjPtr JsonRpcService::rpc_getRoot() { return Serializer::toPtr(getRootObject()); }

	ObjPtr JsonRpcService::rpc_getParent(ObjPtr objPtr)
	{
		Object* obj = fromPtr<Object>(objPtr);
		if (!obj)
			return 0;
		return Serializer::toPtr(*obj->getParentObject());
	}

	void JsonRpcService::rpc_addChild(ObjPtr parentPtr, const std::string& typeName)
	{
		auto parent = fromPtr<Object>(parentPtr);
		if (!parent)
			return;

		RTTI::TypeInfo type = RTTI::TypeInfo::getByName(typeName);
		if (!type.isValid()) {
			Logger::fatal("Failed to resolve type: %s", typeName.c_str());
			return;
		}

		parent->addChild(type.getName(), type);
	}

	void JsonRpcService::rpc_addEntity(ObjPtr parentEntity)
	{
		Entity* parent = fromPtr<Entity>(parentEntity);
		if (!parent)
			return;

		parent->addEntity("NewEntity");
	}

	std::string JsonRpcService::rpc_getName(ObjPtr ptr)
	{
		auto obj = fromPtr<Object>(ptr);
		if (!obj)
			return "";
		return obj->getName();
	}

	void JsonRpcService::rpc_setName(ObjPtr ptr, const std::string& name)
	{
		auto obj = fromPtr<Object>(ptr);
		if (obj)
			obj->setName(name);
	}

	std::string JsonRpcService::rpc_getTypeName(ObjPtr ptr)
	{
		auto obj = fromPtr<Object>(ptr);
		if (!obj)
			return "";
		return obj->getTypeInfo().getName();
	}

	std::string JsonRpcService::rpc_getAttributeValue(ObjPtr attribPtr)
	{
		auto attrib = fromPtr<AttributeBase>(attribPtr);
		if (!attrib)
			return "";

		std::string val;
		attrib->toString(val);
		return val;
	}

	void JsonRpcService::rpc_setAttributeValue(ObjPtr attribPtr, const std::string& value)
	{
		auto attrib = fromPtr<AttributeBase>(attribPtr);
        std::cout << "setAttributeValue()" << std::endl;
		if (attrib)
        {
            std::cout << "setAttributeValue() " << attrib->getName() << " " << value << std::endl;
			attrib->fromString(value);
        }
	}

	void JsonRpcService::rpc_forceSetAttributeValue(ObjPtr ptr, const std::string& attribName,
															const std::string& attribValue,
															const std::string& attribType)
	{
		RTTI::TypeInfo valueType = RTTI::TypeInfo::getByName(attribType);
		if (!valueType.isValid()) {
			Logger::fatal("Invalid valueType: %s", attribType.c_str());
			return;
		}

		AttributeObject* obj = fromPtr<AttributeObject>(ptr);
		AttributeBase* attrib = obj->getOrCreateAttribute(attribName, valueType);
		if (!attrib) {
			Logger::warn("Attribute not found and could not create.");
			return;
		}
		attrib->setValue(attribValue);
	}

	void JsonRpcService::rpc_addObjectCallbacks(const std::string& ident, ObjPtr ptr)
	{
		Object* obj = fromPtr<Object>(ptr);
		Logger::debug("Client '%s' requesting callbacks for '%s'", ident.c_str(), obj->getName().c_str());

//		addCallbacks(ident, ptr);
	}

	void JsonRpcService::rpc_connectPlugs(ObjPtr srcPlugPtr, ObjPtr dstPlugPtr)
	{
		OutputPlugBase* srcPlug = fromPtr<OutputPlugBase>(srcPlugPtr);
		InputPlugBase* dstPlug = fromPtr<InputPlugBase>(dstPlugPtr);
		if (!srcPlug) {
			Logger::fatal("Failed to retrieve source plug while connecting");
			return;
		}
		if (!dstPlug) {
			Logger::fatal("Failed to retrieve destination plug while connecting");
			return;
		}
		dstPlug->connect(*srcPlug);
	}
    
    
    void JsonRpcService::rpc_disconnectPlug(ObjPtr dstPlugPtr)
    {
        InputPlugBase* dstPlug = fromPtr<InputPlugBase>(dstPlugPtr);
        if (!dstPlug) {
            Logger::fatal("Failed to retrieve destination plug while disconnecting");
            return;
        }
        dstPlug->disconnect();
    }
    

	void JsonRpcService::rpc_exportObject(ObjPtr ptr, const std::string& filename)
	{
		auto obj = fromPtr<Object>(ptr);
		if (!obj)
			return;
		JSONSerializer ser;
		std::ofstream os(filename);
		ser.writeObject(os, *obj, false);
		os.close();
	}

	void JsonRpcService::rpc_importObject(ObjPtr parentPtr, const std::string& filename)
	{
		auto parentObj = fromPtr<Object>(parentPtr);
		if (!parentObj)
			return;
		JSONSerializer ser;
		ser.load(filename, getCore(), parentObj);
	}

    void JsonRpcService::rpc_loadFile(const std::string& filename) {
        if (!fileExists(filename)) {
            Logger::fatal("File does not exist: %s", filename.c_str());
            return;
        }

        std::string fname = filename;
        JSONSerializer ser;
        ser.load(filename, getCore());
    }


    void JsonRpcService::rpc_removeObject(ObjPtr ptr)
	{
		Object* obj = fromPtr<Object>(ptr);
		Logger::info("Removing object: %s", obj->getName().c_str());
		obj->getParentObject()->removeChild(*obj);
	}

	void JsonRpcService::addCallbacks(Object *obj)
	{
		obj->nameChanged.connect([=](const std::string& name) {
			handleNameChanged(*obj);
		});

		obj->childAdded.connect([=](Object& child) {
			handleObjectAdded(*child.getParentObject(), child);
			addCallbacks(&child);
		});

		obj->childRemoved.connect([=](Object& child) {
			handleObjectRemoved(child);
		});

		if (auto attrib = rtti_cast<AttributeBase>(obj)) {
			attrib->valueChanged.connect([=](AttributeBase& a) {
				handleAttributeValueChanged(a);
			});
		}

		if (auto plug = rtti_cast<InputPlugBase>(obj)) {
			plug->connected.connect([=](InputPlugBase& p) {
				handlePlugConnected(p);
			});
			plug->disconnected.connect([=](InputPlugBase& p) {
				handlePlugDisconnected(p);
			});
		}

		for (auto c : obj->getChildren())
			addCallbacks(c);
	}

	void JsonRpcService::enqueueEvent(const std::string &msg) {
		if (!broadcastChanges.getValue())
			return;
//		Logger::info("Sending to clients: '%s'", msg.c_str());
		for (AsyncTCPClient* client : getServer().getClients()) {
			client->enqueueEvent(msg);
		}
	}
    void JsonRpcService::rpc_triggerSignalAttribute(ObjPtr ptr) {
        auto attrib = fromPtr<SignalAttribute>(ptr);
        attrib->trigger();
    }

}