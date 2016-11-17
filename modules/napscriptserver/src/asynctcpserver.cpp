#include "asynctcpserver.h"

namespace nap
{

	std::string AsyncTCPClient::popEvent()
	{
		std::string evt = mEventQueue.front();
		mEventQueue.pop();
		return evt;
	}

	AsyncTCPServer::AsyncTCPServer(int port) : mPort(port)
	{
		mThread = std::make_unique<std::thread>(std::bind(&AsyncTCPServer::runServer, this));
	}

	void AsyncTCPServer::runServer()
	{
		zmq::context_t ctx;
		zmq::socket_t sock(ctx, ZMQ_ROUTER);
		const char* hostname = ("tcp://*:" + std::to_string(mPort)).c_str();
		sock.bind(hostname);

		Logger::debug("AsyncTCPServer running on '%s'", hostname);

		while (mRunning) {
			validateClients();

			// Poll for requests / connections
			zmq::pollitem_t pollitem = {sock, 0, ZMQ_POLLIN, 0};
			zmq_poll(&pollitem, 1, 100);
            if (pollitem.revents & ZMQ_POLLIN) {
				// Receive multipart message
				zmq::message_t message;
				sock.recv(&message);
				std::string ident = std::string(static_cast<char*>(message.data()), message.size());
				sock.recv(&message);
				std::string msg = std::string(static_cast<char*>(message.data()), message.size());

				AsyncTCPClient* client = getOrAddClient(ident);
				client->updateHeartbeat();

				// Emit to listeners
				if (!msg.empty()) {
					client->messageReceived.trigger(msg);
					requestReceived.trigger(*client, msg);
				}
			}

			// For each client, process event queue
			for (auto it = mClients.begin(); it != mClients.end(); ++it) {
				AsyncTCPClient& client = *it->second.get();
				while (client.hasEvents()) {
					sendMultipart(sock, client.getIdent(), client.popEvent());
				}
			}
		}

        mClients.clear();
		ctx.close();
	}

    AsyncTCPClient* AsyncTCPServer::getClient(const std::string& ident) {
        auto it = mClients.find(ident);
        if (it != mClients.end())
            return it->second.get();
        return nullptr;
    }

	AsyncTCPClient* AsyncTCPServer::getOrAddClient(const std::string& ident)
	{
        AsyncTCPClient* clientPtr = getClient(ident);
        if (clientPtr) return clientPtr;

		// Client not found, create one and store
		auto client = std::unique_ptr<AsyncTCPClient>(new AsyncTCPClient(ident));
		clientPtr = client.get();
		mClients.emplace(ident, std::move(client));
		clientConnected.trigger(*clientPtr);
		return clientPtr;
	}


	void AsyncTCPServer::sendMultipart(zmq::socket_t& sock, const std::string& ident, const std::string& data)
	{
		zmq::message_t msgIdent(ident.c_str(), ident.size());
		zmq::message_t msgData(data.c_str(), data.size());
		sock.send(msgIdent, ZMQ_SNDMORE);
		sock.send(msgData);
	}


	void AsyncTCPServer::validateClients()
	{
		time_t currentTime = time(nullptr);
		for (auto it = mClients.cbegin(); it != mClients.cend();) {
			AsyncTCPClient& client = *it->second.get();
			if (currentTime - client.getLastHeartbeat() > mClientTimeout) {
                // Client has timed out, kill
				clientDisconnected(client);
				client.disconnected(client);
				it = mClients.erase(it++);
			} else {
				++it;
			}
		}
	}
}