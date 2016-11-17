#include "jsonrpcservercomponent.h"
#include <chrono>
#include <fstream>
#include <jsonserializer.h>
#include <rapidjson/prettywriter.h>
#include <zmq.h>


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

		mServer.RegisterFormatHandler(mFormatHandler);

		auto& disp = mServer.GetDispatcher();

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

		disp.AddMethod("getObjectTree", [&]() -> std::string { return JSONSerializer().toString(*getRootObject(), true); });

		disp.AddMethod("getRoot", [&]() -> ObjPtr { return toPtr(getRootObject()); });

		disp.AddMethod("getParent", [&](ObjPtr objPtr) -> ObjPtr {
			Object* obj = fromPtr<Object>(objPtr);
			if (!obj)
				return 0;
			return toPtr(*obj->getParentObject());
		});

		disp.AddMethod("getAllChildren", [&](ObjPtr ptr) -> std::vector<ObjPtr> {
			std::vector<ObjPtr> children;

			auto parent = fromPtr<Object>(ptr);
			if (!parent)
				return children;

			for (auto child : parent->getChildren())
				children.push_back(toPtr(child));
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

			auto conv = getCore().getModuleManager().getTypeConverter(RTTI_OF(std::string), attrib->getValueType());
			if (!conv) {
				Logger::debug("Failed to get type converter from std::strAttr to '%s'.",
							  attrib->getValueType().getName().c_str());
				return;
			}

			Attribute<std::string> strAttr;
			strAttr.setValue(value);
			conv->convert(&strAttr, attrib);

			if (attrib)
				attrib->fromString(value);
		});

		disp.AddMethod("addObjectCallbacks", [&](const std::string& ident, ObjPtr ptr) {
            Object* obj = fromPtr<Object>(ptr);
			Logger::debug("Client '%s' requesting callbacks for '%s'", ident.c_str(), obj->getName().c_str());

			AsyncTCPClient* client = getServer().getClient(ident);
			if (!client)
				return;

			addCallbacks(*client, ptr);
		});


		disp.AddMethod("exportObject", [&](ObjPtr ptr, const std::string& filename) {
			auto obj = fromPtr<Object>(ptr);
			if (!obj)
				return;
			JSONSerializer ser;
			std::ofstream os(filename);
            ser.writeObject(os, *obj, false);
		});

		disp.AddMethod("importObject", [&](ObjPtr parentPtr, const std::string& filename) {
			auto parentObj = fromPtr<Object>(parentPtr);
			if (!parentObj)
				return;
			JSONDeserializer ser;
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
		return mServer.HandleRequest(cmd)->GetData();
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


	void JSONRPCServerComponent::handleNameChanged(AsyncTCPClient& client, Object& obj)
	{
		using namespace rapidjson;
		StringBuffer buf;
		PrettyWriter<StringBuffer> w(buf);
		w.StartObject();
		{
			w.String("ptr");
			w.Int64(toPtr(obj));
			w.String("name");
			w.String(obj.getName().c_str());
		}
		w.EndObject();

		client.enqueueEvent(callbackJSON("nameChanged", buf.GetString()));

		Logger::info("Sending Attribute Change: %s : %s", obj.getName().c_str(), obj.getName().c_str());
	}

    void JSONRPCServerComponent::handleAttributeValueChanged(AsyncTCPClient& client, AttributeBase& attrib) {
        std::string value;
        attrib.toString(value);

        using namespace rapidjson;
        StringBuffer buf;
        PrettyWriter<StringBuffer> w(buf);

        w.StartObject();
        {
            w.String("ptr");
            w.Int64(toPtr(attrib));
            w.String("name");
            w.String(attrib.getName().c_str());
            w.String("value");
            w.String(value.c_str());
        }
        w.EndObject();

        client.enqueueEvent(callbackJSON("attributeValueChanged", buf.GetString()));

        Logger::info("Sending Attribute Change: %s : %s", attrib.getName().c_str(), attrib.getName().c_str());
    }


    void JSONRPCServerComponent::handleObjectAdded(AsyncTCPClient& client, Object& obj, Object& child) {
        using namespace rapidjson;
        StringBuffer buf;
        PrettyWriter<StringBuffer> w(buf);
        w.StartObject();
        {
            w.String("ptr");
            w.Int64(toPtr(obj));
            w.String("child");
            w.String(JSONSerializer().toString(child, true).c_str());
        }
        w.EndObject();
        client.enqueueEvent(callbackJSON("objectAdded", buf.GetString()));
    }
    void JSONRPCServerComponent::handleObjectRemoved(AsyncTCPClient& client, Object& child) {
        using namespace rapidjson;
        StringBuffer buf;
        PrettyWriter<StringBuffer> w(buf);
        w.StartObject();
        {
            w.String("ptr");
            w.Int64(toPtr(child));
        }
        w.EndObject();
        client.enqueueEvent(callbackJSON("objectRemoved", buf.GetString()));
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


}