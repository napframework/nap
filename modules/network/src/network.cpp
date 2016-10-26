//
//  network.cpp
//  Project
//
//  Created by Stijn van Beek on 7/6/16.
//
//

// Local Includes
#include "network.h"

// External Includes
#include <nap/logger.h>
#include <iostream>
#include <thread>
#include <sstream>

RTTI_DEFINE(nap::HttpClient)

namespace nap {

	std::size_t completion(const asio::error_code& error, std::size_t bytes_transfered)
	{
		return !error;
	}
    

	nap::HttpClient::Response HttpClient::sendRequest(RequestKind kind, const std::string& inPath, const std::string& body)
	{
		// path needs a leading "/"
		auto path = inPath;
		if (path[0] != '/')
			path.insert(0, "/");

		Response result;
		result.path = path;

		if (host.getValue() == "")
		{
			result.errorMessage = "No host specified to send request";
			return result;
		}


		try {
			// Get a list of endpoints corresponding to the server name.
			asio::io_service ioService;
			asio::ip::tcp::resolver resolver(ioService);
			asio::ip::tcp::resolver::query query(host.getValue(), "http");
			asio::ip::tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);

			// Try each endpoint until we successfully establish a connection.
			asio::ip::tcp::socket socket(ioService);
			asio::connect(socket, endpoint_iterator);

			// Form the request. We specify the "Connection: close" header so that the
			// server will close the socket after transmitting the response. This will
			// allow us to treat all data up until the EOF as the content.
			asio::streambuf request;
			std::ostream request_stream(&request);
			switch (kind)
			{
			case RequestKind::GET:
			{
				std::string requestStr = makeGetRequest(path);
				request_stream << requestStr;
				break;
			}
			case RequestKind::POST:
				request_stream << makePostRequest(path, body);
				break;
			}

			// Send the request.
			asio::write(socket, request);

			// Read the response status line. The response streambuf will automatically
			// grow to accommodate the entire line. The growth may be limited by passing
			// a maximum size to the streambuf constructor.
			asio::streambuf response;
			asio::error_code ec;
			asio::read_until(socket, response, "\r\n", ec);

			// Check that response is OK.
			std::istream response_stream(&response);

			std::string http_version;
			response_stream >> http_version;
			unsigned int status_code;
			response_stream >> status_code;
			std::string status_message;
			std::getline(response_stream, status_message);

			if (!response_stream || http_version.substr(0, 5) != "HTTP/")
			{
				result.errorMessage = "Invalid response";
				return result;
			}
// 			if (status_code != 200)
// 			{
// 				result.errorMessage = "Response returned with status code " + std::to_string(status_code);
// 				return result;
// 			}

			// Read the response headers, which are terminated by a blank line.
			asio::read_until(socket, response, "\r\n\r\n");

			// Process the response headers.
			std::string header;
			while (std::getline(response_stream, header) && header != "\r") {
             //   nap::Logger::debug(header);
            }

			std::string line;

			// read the rest of the reply
			asio::error_code error;
			asio::read(socket, response, completion, error);
			while (std::getline(response_stream, line))
				result.content.append(line);

			// check if an error has occured
			if (error && error != asio::error::eof)
				throw asio::system_error(error); // Some other error.

		}

		catch (std::exception& e)
		{
			result.errorMessage = e.what();
			return result;
		}

		return result;
	}


	void HttpClient::sendRequest(RequestKind kind, const std::string& path, const std::string& body, ResponseFunction responseFunction)
	{
		threadPool.enqueue([&, kind, path, body, responseFunction]() {
			Response result = sendRequest(kind, path, body);
			responseFunction(result);
		});
	}


	std::string HttpClient::makePostRequest(const std::string& path, const std::string& body)
	{
		std::stringstream result;

		result << "POST " << path << " HTTP/1.0" << std::endl;
		result << "Host: " << host.getValue() << std::endl;
		result << "Content-Type: application/x-www-form-urlencoded" << std::endl;
		result << "Content-Length: " << body.size() << std::endl;
		result << "Connection: close" << std::endl;
		result << std::endl;
		result << body;

		return result.str();
	}


	std::string HttpClient::makePostBody(const std::map<std::string, std::string>& values)
	{
		std::string result;
		auto last = values.end();
		last--;
		for (auto it = values.begin(); it != values.end(); ++it)
		{
			result.append(it->first + "=" + it->second);
			if (it != last)
				result.append("&");
		}

		return result;
	}


	std::string HttpClient::makeGetRequest(const std::string& path)
	{
		std::stringstream result;

		result << "GET " << path << " HTTP/1.0" << std::endl;
		result << "Host: " << host.getValue() << std::endl;
		result << "Accept: */*" << std::endl;
		result << "Connection: close" << std::endl;
		result << std::endl;

		return result.str();
	}

}


