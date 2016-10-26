#include "scriptservercomponent.h"
#include <zmq.hpp>

#include <asio.hpp>

RTTI_DEFINE(nap::ScriptServerComponent)
RTTI_DEFINE(nap::JSONRPCServerComponent)
RTTI_DEFINE(nap::PythonServerComponent)

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

namespace nap
{

	void JSONRPCServerComponent::run()
	{
		zmq::context_t context(1);
		zmq::socket_t socket(context, ZMQ_REP);
		std::string host = "tcp://*:" + std::to_string(port.getValue());
		socket.bind(host.c_str());
		Logger::info("Started server: " + host);
		bool running = true;
		while (running) {
			zmq::message_t request;
			//  Wait for next request from client
			std::string msg = s_recv(socket);
			std::cout << "Recv: " << msg << std::endl;
			std::string reply = mInterpreter->evalScript(msg);
			std::cout << "Send: " << reply << std::endl;
			s_send(socket, reply);
		}
	}

	void PythonServerComponent::run()
	{
		using namespace asio::ip;
		asio::io_service ioService;
		tcp::acceptor acceptor(ioService, tcp::endpoint(tcp::v4(), port.getValue()));
		tcp::socket socket(ioService);
		nap::Logger::info("Waiting for client on port " + std::to_string(port.getValue()));
		acceptor.accept(socket);
		nap::Logger::info("Client connected");

		asio::error_code error;
		asio::streambuf buffer;
		bool running = true;
		while (running) {
			asio::read_until(socket, buffer, "\n", error);
			std::istream str(&buffer);
			std::string s;
			std::getline(str, s);
			std::string reply = mInterpreter->evalScript(s) + "\n";

			asio::write(socket, asio::buffer(reply), asio::transfer_all(), error);
		}
	}
}