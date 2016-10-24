#include "scriptservercomponent.h"
#include <zmq.hpp>

RTTI_DEFINE(nap::ScriptServerComponent)

static std::string s_recv(zmq::socket_t& socket)
{

	zmq::message_t message;
	socket.recv(&message);

	return std::string(static_cast<char*>(message.data()), message.size());
}

static bool
s_send (zmq::socket_t & socket, const std::string & string) {

    zmq::message_t message(string.size());
    memcpy (message.data(), string.data(), string.size());

    bool rc = socket.send (message);
    return (rc);
}

namespace nap
{

	ScriptServerComponent::ScriptServerComponent() : Component()
	{
		mInterpreter = &addChild<JSONRPCInterpreter>("ScriptInterpreter");
		run();
	}

	void ScriptServerComponent::run()
	{
		zmq::context_t context(1);
		zmq::socket_t socket(context, ZMQ_REP);
        std::string host = "tcp://*:8888";
		socket.bind(host.c_str());
        Logger::info("Started server: " + host);
		while (true) {
			zmq::message_t request;
			//  Wait for next request from client
			std::string msg = s_recv(socket);
            std::cout << "Recv: " << msg << std::endl;
            std::string reply = mInterpreter->evalScript(msg);
            std::cout << "Send: " << reply << std::endl;
            s_send(socket, reply);

		}
	}
}