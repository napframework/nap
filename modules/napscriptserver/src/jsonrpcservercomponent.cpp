#include "jsonrpcservercomponent.h"
#include <fstream>
#include <jsonserializer.h>
#include <rapidjson/prettywriter.h>


RTTI_DEFINE(nap::JSONRPCServerComponent)



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


	JSONRPCServerComponent::JSONRPCServerComponent() : mContext(1), ScriptServerComponent()
	{

		Logger::instance().log.connect(onLogSlot);

		mJsonServer.RegisterFormatHandler(mFormatHandler);

		auto& disp = mJsonServer.GetDispatcher();

		disp.AddMethod("getModuleInfo", &JSONRPCServerComponent::rpc_getModuleInfo, *this);
		disp.AddMethod("getObjectTree", &JSONRPCServerComponent::rpc_getObjectTree, *this);
		disp.AddMethod("copyObjectTree", &JSONRPCServerComponent::rpc_copyObjectTree, *this);
		disp.AddMethod("pasteObjectTree", &JSONRPCServerComponent::rpc_pasteObjectTree, *this);
		disp.AddMethod("addChild", &JSONRPCServerComponent::rpc_addChild, *this);
		disp.AddMethod("addEntity", &JSONRPCServerComponent::rpc_addEntity, *this);
		disp.AddMethod("setName", &JSONRPCServerComponent::rpc_setName, *this);
		disp.AddMethod("setAttributeValue", &JSONRPCServerComponent::rpc_setAttributeValue, *this);
		disp.AddMethod("forceSetAttributeValue", &JSONRPCServerComponent::rpc_forceSetAttributeValue, *this);
		disp.AddMethod("connectPlugs", &JSONRPCServerComponent::rpc_connectPlugs, *this);
		disp.AddMethod("exportObject", &JSONRPCServerComponent::rpc_exportObject, *this);
		disp.AddMethod("importObject", &JSONRPCServerComponent::rpc_importObject, *this);
		disp.AddMethod("removeObject", &JSONRPCServerComponent::rpc_removeObject, *this);

		//		disp.AddMethod("getModules", &JSONRPCServerComponent::rpc_getModules, *this);
		//		disp.AddMethod("getDataTypes", &JSONRPCServerComponent::rpc_getDataTypes, *this);
		//		disp.AddMethod("getRoot", &JSONRPCServerComponent::rpc_getRoot, *this);
		//		disp.AddMethod("getParent", &JSONRPCServerComponent::rpc_getParent, *this);
		//		disp.AddMethod("getName", &JSONRPCServerComponent::rpc_getName, *this);
		//		disp.AddMethod("getTypeName", &JSONRPCServerComponent::rpc_getTypeName, *this);
		//		disp.AddMethod("getAttributeValue", &JSONRPCServerComponent::rpc_getAttributeValue, *this);
		//		disp.AddMethod("addObjectCallbacks", &JSONRPCServerComponent::rpc_addObjectCallbacks, *this);
	}

	std::string JSONRPCServerComponent::evalScript(const std::string& cmd)
	{
		std::lock_guard<std::mutex> lock(mMutex);
		return mJsonServer.HandleRequest(cmd)->GetData();
	}


	void JSONRPCServerComponent::run()
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

	void JSONRPCServerComponent::handleLogMessage(AsyncTCPClient& client, LogMessage& msg)
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
		client.enqueueEvent(callbackJSON("log", buf.GetString()));
	}

	void JSONRPCServerComponent::handleNameChanged(AsyncTCPClient& client, Object& obj)
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

		client.enqueueEvent(callbackJSON("nameChanged", buf.GetString()));

		Logger::info("Sending Attribute Change: %s : %s", obj.getName().c_str(), obj.getName().c_str());
	}

	void JSONRPCServerComponent::handleAttributeValueChanged(AsyncTCPClient& client, AttributeBase& attrib)
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

		client.enqueueEvent(callbackJSON("attributeValueChanged", buf.GetString()));

		Logger::info("Sending Attribute Change: %s : %s", attrib.getName().c_str(), attrib.getName().c_str());
	}


	void JSONRPCServerComponent::handleObjectAdded(AsyncTCPClient& client, Object& obj, Object& child)
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
		client.enqueueEvent(callbackJSON("objectAdded", buf.GetString()));
	}


	void JSONRPCServerComponent::handleObjectRemoved(AsyncTCPClient& client, Object& child)
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
		client.enqueueEvent(callbackJSON("objectRemoved", buf.GetString()));
	}


	void JSONRPCServerComponent::handlePlugConnected(AsyncTCPClient& client, InputPlugBase& plug)
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
		client.enqueueEvent(callbackJSON("plugConnected", buf.GetString()));
	}


	void JSONRPCServerComponent::handlePlugDisconnected(AsyncTCPClient& client, InputPlugBase& plug)
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
		client.enqueueEvent(callbackJSON("plugDisconnected", buf.GetString()));
	}


	void JSONRPCServerComponent::startRPCSocket()
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


	void JSONRPCServerComponent::onLog(LogMessage msg)
	{
		for (AsyncTCPClient* client : getServer().getClients())
			handleLogMessage(*client, msg);
	}

	/////////////////// RPC METHODS /////////////////////
	/////////////////////////////////////////////////////

	std::vector<std::string> JSONRPCServerComponent::rpc_getModules()
	{
		std::vector<std::string> modulenames;
		for (auto mod : getCore().getModuleManager().getModules())
			modulenames.push_back(mod->getName());
		return modulenames;
	}

	std::vector<std::string> JSONRPCServerComponent::rpc_getDataTypes(const std::string& modname)
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

	std::string JSONRPCServerComponent::rpc_getModuleInfo()
	{
		return JSONSerializer().toString(getCore().getModuleManager());
	}

	std::string JSONRPCServerComponent::rpc_getObjectTree()
	{
		return JSONSerializer().toString(*getRootObject(), true);
	}

	std::string JSONRPCServerComponent::rpc_copyObjectTree(ObjPtr objPtr)
	{
		Object* obj = fromPtr<Object>(objPtr);
		if (!obj)
			return 0;
		return JSONSerializer().toString(*obj, true);
	}

	void JSONRPCServerComponent::rpc_pasteObjectTree(ObjPtr parentPtr, const std::string& jsonData)
	{
		Object* obj = fromPtr<Object>(parentPtr);
		if (!obj)
			return;
		Logger::info(jsonData);
		JSONSerializer().fromString(jsonData, getCore(), obj);
	}

	ObjPtr JSONRPCServerComponent::rpc_getRoot() { return Serializer::toPtr(getRootObject()); }

	ObjPtr JSONRPCServerComponent::rpc_getParent(ObjPtr objPtr)
	{
		Object* obj = fromPtr<Object>(objPtr);
		if (!obj)
			return 0;
		return Serializer::toPtr(*obj->getParentObject());
	}

	void JSONRPCServerComponent::rpc_addChild(ObjPtr parentPtr, const std::string& typeName)
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

	void JSONRPCServerComponent::rpc_addEntity(ObjPtr parentEntity)
	{
		Entity* parent = fromPtr<Entity>(parentEntity);
		Logger::info("Adding entity to: %s", parent->getName().c_str());
		if (!parent)
			return;
		parent->addEntity("NewEntity");
	}

	std::string JSONRPCServerComponent::rpc_getName(ObjPtr ptr)
	{
		auto obj = fromPtr<Object>(ptr);
		if (!obj)
			return "";
		return obj->getName();
	}

	void JSONRPCServerComponent::rpc_setName(ObjPtr ptr, const std::string& name)
	{
		auto obj = fromPtr<Object>(ptr);
		if (obj)
			obj->setName(name);
	}

	std::string JSONRPCServerComponent::rpc_getTypeName(ObjPtr ptr)
	{
		auto obj = fromPtr<Object>(ptr);
		if (!obj)
			return "";
		return obj->getTypeInfo().getName();
	}

	std::string JSONRPCServerComponent::rpc_getAttributeValue(ObjPtr attribPtr)
	{
		auto attrib = fromPtr<AttributeBase>(attribPtr);
		if (!attrib)
			return "";

		std::string val;
		attrib->toString(val);
		return val;
	}

	void JSONRPCServerComponent::rpc_setAttributeValue(ObjPtr attribPtr, const std::string& value)
	{
		auto attrib = fromPtr<AttributeBase>(attribPtr);
		if (attrib)
			attrib->fromString(value);
	}

	void JSONRPCServerComponent::rpc_forceSetAttributeValue(ObjPtr ptr, const std::string& attribName,
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

	void JSONRPCServerComponent::rpc_addObjectCallbacks(const std::string& ident, ObjPtr ptr)
	{
		Object* obj = fromPtr<Object>(ptr);
		Logger::debug("Client '%s' requesting callbacks for '%s'", ident.c_str(), obj->getName().c_str());

		addCallbacks(ident, ptr);
	}

	void JSONRPCServerComponent::rpc_connectPlugs(ObjPtr srcPlugPtr, ObjPtr dstPlugPtr)
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

	void JSONRPCServerComponent::rpc_exportObject(ObjPtr ptr, const std::string& filename)
	{
		auto obj = fromPtr<Object>(ptr);
		if (!obj)
			return;
		JSONSerializer ser;
		std::ofstream os(filename);
		ser.writeObject(os, *obj, false);
		os.close();
	}

	void JSONRPCServerComponent::rpc_importObject(ObjPtr parentPtr, const std::string& filename)
	{
		auto parentObj = fromPtr<Object>(parentPtr);
		if (!parentObj)
			return;
		JSONSerializer ser;
		std::ifstream is(filename);
		ser.readObject(is, getCore(), parentObj);
	}

	void JSONRPCServerComponent::rpc_removeObject(ObjPtr ptr)
	{
		Object* obj = fromPtr<Object>(ptr);
		Logger::info("Removing object: %s", obj->getName().c_str());
		obj->getParentObject()->removeChild(*obj);
	}
}