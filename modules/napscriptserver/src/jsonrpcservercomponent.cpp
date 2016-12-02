#include "jsonrpcservercomponent.h"
#include <fstream>
#include <jsonserializer.h>
#include <rapidjson/prettywriter.h>


RTTI_DEFINE(nap::JSONRPCServerComponent)

namespace nap
{

	static bool s_send(zmq::socket_t& socket, const std::string& string)
	{

		zmq::message_t message(string.size());
		memcpy(message.data(), string.data(), string.size());

		bool rc = socket.send(message);
		return (rc);
	}

	std::vector<std::string> toStringList(TypeList types)
	{
		std::vector<std::string> typenames;
		for (const auto& type : types)
			typenames.push_back(type.getName());
		return typenames;
	}

	JSONRPCServerComponent::JSONRPCServerComponent() : mContext(1), ScriptServerComponent()
	{

		Logger::instance().log.connect(onLogSlot);

		mJsonServer.RegisterFormatHandler(mFormatHandler);

		auto& disp = mJsonServer.GetDispatcher();

		disp.AddMethod("getModules", [&]() -> std::vector<std::string> {
			std::vector<std::string> modulenames;
			for (auto mod : getCore().getModuleManager().getModules())
				modulenames.push_back(mod->getName());
			return modulenames;
		});

		disp.AddMethod("getDataTypes", [&](const std::string& modname) -> std::vector<std::string> {
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
		});


		disp.AddMethod("getModuleInfo", [&]() { return JSONSerializer().toString(getCore().getModuleManager()); });

		disp.AddMethod("getObjectTree",
					   [&]() -> std::string { return JSONSerializer().toString(*getRootObject(), true); });

		disp.AddMethod("copyObjectTree", [&](ObjPtr objPtr) -> std::string {
			Object* obj = fromPtr<Object>(objPtr);
			if (!obj)
				return 0;
			return JSONSerializer().toString(*obj, true);
		});

		disp.AddMethod("pasteObjectTree", [&](ObjPtr parentPtr, std::string jsonData) {
			Object* obj = fromPtr<Object>(parentPtr);
			if (!obj)
				return;
			Logger::info(jsonData);
			JSONSerializer().fromString(jsonData, getCore(), obj);
		});

		disp.AddMethod("getRoot", [&]() -> ObjPtr { return Serializer::toPtr(getRootObject()); });

		disp.AddMethod("getParent", [&](ObjPtr objPtr) -> ObjPtr {
			Object* obj = fromPtr<Object>(objPtr);
			if (!obj)
				return 0;
			return Serializer::toPtr(*obj->getParentObject());
		});

		disp.AddMethod("getAllChildren", [&](ObjPtr ptr) -> std::vector<ObjPtr> {
			std::vector<ObjPtr> children;

			auto parent = fromPtr<Object>(ptr);
			if (!parent)
				return children;

			for (auto child : parent->getChildren())
				children.push_back(Serializer::toPtr(child));
			return children;
		});

		disp.AddMethod("getChildren", [&](ObjPtr objPtr, const std::string& typeName) {
			std::vector<std::string> children;

			auto parent = fromPtr<Object>(objPtr);
			if (!parent)
				return children;

			RTTI::TypeInfo type = RTTI::TypeInfo::getByName(typeName);
			for (auto child : parent->getChildrenOfType(type))
				children.push_back(ObjectPath(child));
			return children;
		});

		disp.AddMethod("addChild", [&](ObjPtr parentPtr, const std::string& typeName) {
			auto parent = fromPtr<Object>(parentPtr);
			if (!parent)
				return;

			RTTI::TypeInfo type = RTTI::TypeInfo::getByName(typeName);
			if (!type.isValid()) {
				Logger::fatal("Failed to resolve type: %s", typeName.c_str());
				return;
			}

			parent->addChild(type.getName(), type);
		});

		disp.AddMethod("addEntity", [&](ObjPtr parentEntity) {
			std::lock_guard<std::mutex> lock(mMutex);
			Entity* parent = fromPtr<Entity>(parentEntity);
			Logger::info("Adding entity to: %s", parent->getName().c_str());
			if (!parent)
				return;
			parent->addEntity("NewEntity");
		});

		disp.AddMethod("getName", [&](ObjPtr ptr) -> std::string {
			auto obj = fromPtr<Object>(ptr);
			if (!obj)
				return "";
			return obj->getName();
		});

		disp.AddMethod("setName", [&](ObjPtr ptr, const std::string& name) {
			auto obj = fromPtr<Object>(ptr);
			if (obj)
				obj->setName(name);
		});

		disp.AddMethod("getTypeName", [&](ObjPtr ptr) -> std::string {
			auto obj = fromPtr<Object>(ptr);
			if (!obj)
				return "";
			return obj->getTypeInfo().getName();
		});

		disp.AddMethod("getAttributeValue", [&](ObjPtr ptr) -> std::string {
			auto attrib = fromPtr<AttributeBase>(ptr);
			if (!attrib)
				return "";

			std::string val;
			attrib->toString(val);
			return val;
		});

		disp.AddMethod("setAttributeValue", [&](ObjPtr ptr, const std::string& value) {
			auto attrib = fromPtr<AttributeBase>(ptr);
			if (attrib)
				attrib->fromString(value);
		});
//
//		disp.AddMethod("forceSetAttributeValue", [&](const std::string& ident, ObjPtr ptr, const std::string& attrName,
//													 const std::string& value, const std::string& dataType) {
//			AttributeObject* obj = fromPtr<AttributeObject>(ptr);
//			if (!obj)
//				return;
//			RTTI::TypeInfo valueType = RTTI::TypeInfo::getByName(Serializer::dirtyHack(dataType));
//			if (!valueType.isValid())
//				return;
//			AttributeBase* attrib = obj->getAttribute(attrName);
//			if (!attrib) {
//				attrib = &obj->addAttribute(attrName, valueType);
//				addCallbacks(ident, Serializer::toPtr(attrib));
//			}
//			if (!attrib->getValueType().isKindOf(valueType)) {
//				return;
//			}
//
//			attrib->fromString(value);
//		});

		disp.AddMethod("addObjectCallbacks", [&](const std::string& ident, ObjPtr ptr) {
			Object* obj = fromPtr<Object>(ptr);
			Logger::debug("Client '%s' requesting callbacks for '%s'", ident.c_str(), obj->getName().c_str());

			addCallbacks(ident, ptr);
		});

		disp.AddMethod("connectPlugs", [&](ObjPtr srcPlugPtr, ObjPtr dstPlugPtr) {
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
		});

		disp.AddMethod("exportObject", [&](ObjPtr ptr, const std::string& filename) {
			auto obj = fromPtr<Object>(ptr);
			if (!obj)
				return;
			JSONSerializer ser;
			std::ofstream os(filename);
			ser.writeObject(os, *obj, false);
			os.close();
		});

		disp.AddMethod("importObject", [&](ObjPtr parentPtr, const std::string& filename) {
			auto parentObj = fromPtr<Object>(parentPtr);
			if (!parentObj)
				return;
			JSONSerializer ser;
			std::ifstream is(filename);
			ser.readObject(is, getCore(), parentObj);
		});

		disp.AddMethod("removeObject", [&](ObjPtr ptr) {
			Object* obj = fromPtr<Object>(ptr);
			Logger::info("Removing object: %s", obj->getName().c_str());
			obj->getParentObject()->removeChild(*obj);
		});
	}

	std::string JSONRPCServerComponent::evalScript(const std::string& cmd)
	{
		std::lock_guard<std::mutex> lock(mMutex);
		return mJsonServer.HandleRequest(cmd)->GetData();
	}


	void JSONRPCServerComponent::run()
	{
		//		startSUBSocket();
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


	void JSONRPCServerComponent::handlePlugConnected(AsyncTCPClient& client, Plug::Connection connection)
	{
		using namespace rapidjson;
		StringBuffer buf;
		PrettyWriter<StringBuffer> w(buf);
		w.StartObject();
		{
			w.String("srcPtr");
			w.Int64(Serializer::toPtr(connection.srcPlug));
			w.String("dstPtr");
			w.Int64(Serializer::toPtr(connection.dstPlug));
		}
		w.EndObject();
		client.enqueueEvent(callbackJSON("plugConnected", buf.GetString()));
	}


	void JSONRPCServerComponent::handlePlugDisconnected(AsyncTCPClient& client, Plug::Connection connection)
	{
		using namespace rapidjson;
		StringBuffer buf;
		PrettyWriter<StringBuffer> w(buf);
		w.StartObject();
		{
			w.String("srcPtr");
			w.Int64(Serializer::toPtr(connection.srcPlug));
			w.String("dstPtr");
			w.Int64(Serializer::toPtr(connection.dstPlug));
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
}