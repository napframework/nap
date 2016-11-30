#pragma once

// External Includes
#include <stdio.h>
#include <nap/component.h>
#include <asio.hpp>
#include <ThreadPool.h>
#include <nap/coreattributes.h>

namespace nap {
    
    // Component that can send http get and post request and receive responses from a server
    class HttpClient : public nap::Component {
        RTTI_ENABLE_DERIVED_FROM(nap::Component)
    public:
		// response to a request sent back from a server
        class Response {
        public:
			// the URL path that the request for this response was sent to
            std::string path = "";
			// the content of the response
            std::string content = "";
			// if an error occured handling the request this contains the error message
            std::string errorMessage = "";
			// indicates wether an error occured handling the request
            bool failed() const { return errorMessage != ""; }
        };

		using ResponseFunction = std::function<void(const Response&)>;
        
    public:
        nap::Attribute<std::string> host = { this, "host", "" };

		// performs http GET request
		Response get(const std::string& path) { return sendRequest(RequestKind::GET, path, ""); }

		// performs http POST request
		Response post(const std::string& path, const std::map<std::string, std::string>& values) { return sendRequest(RequestKind::POST, path, makePostBody(values)); }

		// performs http post request async
		void post(const std::string& path, const std::map<std::string, std::string>& values, ResponseFunction responseFunction) { sendRequest(RequestKind::POST, path, makePostBody(values), responseFunction); }
        
    protected:
        // returns threadpool used for handling http requests on a separate thread
        lib::ThreadPool& getThreadPool() { return threadPool; }
       
    private:
		// specifying the nature of an http request
		enum class RequestKind { GET, POST };

		// sends a http request with @kind = "GET" or "POST", @path is the URI and @body the message body
		Response sendRequest(RequestKind kind, const std::string& path, const std::string& body);

		// sends the http request async and triggers responseFunction when done
		void sendRequest(RequestKind kind, const std::string& path, const std::string& body, ResponseFunction responseFunction);

		// generates the text for a get request to a certain URI
		std::string makeGetRequest(const std::string& path);
		// generates the text for a post request with content body
		std::string makePostRequest(const std::string& path, const std::string& body);
		// generates the body of a POST request from a map of key value pairs containing parameters and their values
		std::string makePostBody(const std::map<std::string, std::string>& values);

		// threadpool used for handling http requests on a separate thread
        lib::ThreadPool threadPool;
        
    };
    
}

RTTI_DECLARE(nap::HttpClient)

