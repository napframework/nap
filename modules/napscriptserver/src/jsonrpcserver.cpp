#include "jsonrpcserver.h"
#include <jsonserializer.h>
#include <fstream>


RTTI_DEFINE(nap::JSONRPCServerComponent)

namespace nap
{
	static std::string s_recv(zmq::socket_t& socket)
	{

		zmq::message_t message;
		socket.recv(&message);

		return std::string(static_cast<char*>(message.data()), message.size());
	}

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

		disp.AddMethod("getComponentTypes", [&](const std::string& modname) -> std::vector<std::string> {
			if (modname.empty())
				return toStringList(getCore().getModuleManager().getComponentTypes());

			std::vector<std::string> typenames;
			Module* mod = getCore().getModuleManager().getModule(modname);
			if (!mod) {
				Logger::warn("Module not found: %s", modname.c_str());
				return typenames;
			}
			TypeList types;
			mod->getComponentTypes(types);

			for (const auto& type : types)
				typenames.push_back(type.getName());
			return typenames;
		});

		disp.AddMethod("getOperatorTypes", [&](const std::string& modname) -> std::vector<std::string> {
			if (modname.empty())
				return toStringList(getCore().getModuleManager().getOperatorTypes());

			std::vector<std::string> typenames;
			Module* mod = getCore().getModuleManager().getModule(modname);
			if (!mod) {
				Logger::warn("Module not found: %s", modname.c_str());
				return typenames;
			}
			TypeList types;
			mod->getOperatorTypes(types);

			for (const auto& type : types)
				typenames.push_back(type.getName());
			return typenames;
		});

		disp.AddMethod("getObjectTree", [&]() -> std::string {
			JSONSerializer ser;
			return ser.toString(*getRootObject());
		});

		disp.AddMethod("getRoot", [&]() -> std::string { return ObjectPath(getRootObject()).toString(); });

		disp.AddMethod("getParent", [&](const std::string& objPath) -> std::string {
			Object* obj = resolvePath(objPath);
			if (!obj)
				return "";
			return ObjectPath(obj->getParentObject());
		});

		disp.AddMethod("getAllChildren", [&](const std::string& objPath) -> std::vector<std::string> {
			std::vector<std::string> children;

			auto parent = resolvePath(objPath);
			if (!parent)
				return children;

			for (auto child : parent->getChildren())
				children.push_back(ObjectPath(child));
			return children;
		});

		disp.AddMethod("getChildren", [&](const std::string& objPath, const std::string& typeName) {
			std::vector<std::string> children;

			auto parent = resolvePath(objPath);
			if (!parent)
				return children;

			RTTI::TypeInfo type = RTTI::TypeInfo::getByName(typeName);
			for (auto child : parent->getChildrenOfType(type))
				children.push_back(ObjectPath(child));
			return children;
		});

		disp.AddMethod("addChild",
					   [&](const std::string& parentEntity, const std::string& typeName, const std::string& name) {
						   auto parent = resolvePath(parentEntity);
						   if (!parent)
							   return;

						   RTTI::TypeInfo type = RTTI::TypeInfo::getByName(typeName);
						   if (!type.isValid())
							   return;

						   parent->addChild(name, type);
					   });

		disp.AddMethod("addEntity", [&](const std::string& parentEntity, const std::string& name) -> std::string {
			auto parent = resolve<Entity>(parentEntity);
			if (!parent)
				return "";
			return ObjectPath(&parent->addEntity(name)).toString();
		});

		disp.AddMethod("getName", [&](const std::string& objPath) -> std::string {
			auto obj = resolvePath(objPath);
			if (!obj)
				return "";
			return obj->getName();
		});

		disp.AddMethod("setName", [&](const std::string& objPath, const std::string& name) {
			auto obj = resolvePath(objPath);
			if (obj)
				obj->setName(name);
		});

		disp.AddMethod("getTypeName", [&](const std::string& objPath) -> std::string {
			auto obj = resolvePath(objPath);
			if (!obj)
				return "";
			return obj->getTypeInfo().getName();
		});

		disp.AddMethod("getAttributeValue", [&](const std::string& objPath) -> std::string {
			auto attrib = resolve<AttributeBase>(objPath);
			if (!attrib)
				return "";

			std::string val;
			attrib->toString(val);
			return val;
		});

		disp.AddMethod("setAttributeValue", [&](const std::string& objPath, const std::string& value) {
			auto attrib = resolve<AttributeBase>(objPath);

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

		disp.AddMethod("connectValueChanged", [&](const std::string& objPath) {
			Logger::info("Add listener for %s", objPath.c_str());
			addAttributeChangedCallback(objPath);
		});

		disp.AddMethod("disconnectValueChanged",
					   [&](const std::string& objPath) { removeAttributeChangedCallback(objPath); });

		disp.AddMethod("exportObject", [&](const std::string& objPath, const std::string& filename) {
			auto obj = resolvePath(objPath);
			if (!obj)
				return;
			JSONSerializer ser;
			std::ofstream os(filename);
			ser.writeObject(os, *obj);
		});

		disp.AddMethod("importObject", [&](const std::string& parentPath, const std::string& filename) {
			auto parentObj = resolvePath(parentPath);
			if (!parentObj)
				return;
			JSONDeserializer ser;
			std::ifstream is(filename);
			ser.readObject(is, getCore(), parentObj);
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

	void JSONRPCServerComponent::handleAttributeChanged(AttributeBase& attrib)
	{
		std::string val;
		attrib.toString(val);
		Logger::info("Sending Attribute Change: %s : %s", attrib.getName().c_str(), val.c_str());
		s_send(*mPubSocket, "attrchange " + val);
	}

	void JSONRPCServerComponent::startSUBSocket()
	{
		mPubSocket = std::make_unique<zmq::socket_t>(mContext, ZMQ_PAIR);
		std::string host = "tcp://*:" + std::to_string(pubPort.getValue());
		Logger::info("PUB Server: " + host);
		mPubSocket->bind(host);
	}

	void JSONRPCServerComponent::startRPCSocket()
	{ // RPC Socket
		zmq::socket_t socket(mContext, ZMQ_REP);

		std::string host = "tcp://*:" + std::to_string(rpcPort.getValue());

		socket.bind(host.c_str());
		Logger::info("RPC Server: " + host);

		while (running.getValue()) {
			zmq::message_t request;
			//  Wait for next request from client
			std::string msg = s_recv(socket);
			std::string reply = evalScript(msg);
			s_send(socket, reply);
		}
	}
}