/*
 * remote.cpp
 *
 *  Created on: Sep 10, 2013
 *      Author: Martek01
 */

#include "remote.h"

#include <sstream>
#include <boost/asio.hpp>
#include <boost/property_tree/json_parser.hpp>

boost::asio::io_service Remote::ioService;

Remote::Remote() {
	// create socket
	socket = new boost::asio::ip::tcp::socket(ioService);
}

Remote::~Remote() {
	// close socket connection
	disconnect();

	delete socket;
}

bool Remote::connect(boost::asio::ip::address address, unsigned short int port) {
	// resolve address and connect to endpoint
	boost::asio::ip::tcp::endpoint endpoint(address, port);
	boost::system::error_code errorCode;

	socket->connect(endpoint, errorCode);
	if (errorCode) {
		std::cout << "Could not connect to address: " << address.to_string() << ":" << port << ": " << errorCode.message() << "\n";
		return false;
	}

	return true;
}

void Remote::disconnect() {
	socket->shutdown(boost::asio::ip::tcp::socket::shutdown_send);
	socket->close();
}

boost::asio::ip::address Remote::getAddress() {
	return socket->remote_endpoint().address();
}

unsigned short int Remote::getPort() {
	return socket->remote_endpoint().port();
}

bool Remote::sendKey(std::string keyName) {
	// create property tree
	boost::property_tree::ptree propertyTree;
	propertyTree.add("key", keyName);

	return sendPostRequest("/1/input/key", propertyTree);
}

bool Remote::standby() {
	// create property tree
	boost::property_tree::ptree propertyTree;
	propertyTree.add("key", "Standby");

	return sendPostRequest("/1/input/key", propertyTree);
}

bool Remote::setVolume(int level) {
	// create property tree
	boost::property_tree::ptree propertyTree;
	propertyTree.add("current", level);

	return sendPostRequest("/1/audio/volume", propertyTree);
}

bool Remote::setChannel(std::string channelID) {
	// create property tree
	boost::property_tree::ptree propertyTree;
	propertyTree.add("id", channelID);

	return sendPostRequest("/1/channels/current", propertyTree);
}

bool Remote::setSource(std::string sourceName) {
	// create property tree
	boost::property_tree::ptree propertyTree;
	propertyTree.add("id", sourceName);

	return sendPostRequest("/1/sources/current", propertyTree);
}

std::string Remote::getChannelList() {
	return "";
}

boost::property_tree::ptree Remote::sendGetRequest(std::string path) {
	// create buffer stream
	boost::asio::streambuf buffer;
	std::ostream bufferStream(&buffer);

	// add get request header
	bufferStream << "GET " << path << " HTTP/1.1\r\n";
	bufferStream << "Host: " << getAddress() << ":" << getPort() << "\r\n";
	bufferStream << "Accept: */*\r\n\r\n";

	// send request
	boost::asio::write(*socket, buffer);

	// get response
	boost::asio::streambuf response;
	boost::asio::read_until(*socket, response, "\r\n");

	// validate response
	std::istream responseStream(&response);
	std::string httpVersion;
	unsigned int statusCode;
	std::string statusMessage;

	responseStream >> httpVersion;
	responseStream >> statusCode;
	std::getline(responseStream, statusMessage);

	if (!responseStream || httpVersion.substr(0, 5) != "HTTP/") {
		std::cout << "Invalid response\n";
		boost::property_tree::ptree emptyTree;
		return emptyTree;
	}

	if (statusCode != 200) {
		std::cout << "Response returned with status code: " << statusCode << "\n";
		std::cout << statusMessage << std::endl;
		boost::property_tree::ptree emptyTree;
		return emptyTree;
	}

	// get response header
	boost::asio::read_until(*socket, response, "\r\n\r\n");
	std::cout << "Response: " << &response << std::endl;
}

bool Remote::sendPostRequest(std::string path, boost::property_tree::ptree bodyTree) {
	// create buffer stream
	boost::asio::streambuf buffer;
	std::ostream bufferStream(&buffer);

	// create body string from property tree
	std::stringstream bodyStream;
	boost::property_tree::write_json(bodyStream, bodyTree);
	std::string body = bodyStream.str();

	// add get request header
	bufferStream << "POST " << path << " HTTP/1.1\r\n";
	bufferStream << "Host: " << getAddress() << ":" << getPort() << "\r\n";
	bufferStream << "Accept: */*\r\n";
	bufferStream << "Content-Length: " << body.length() + 4 << "\r\n";
	bufferStream << "Content-type: application/json\r\n\r\n";

	bufferStream << body << "\r\n\r\n";

	// send request
	boost::asio::write(*socket, buffer);

	// get response
	boost::asio::streambuf response;
	boost::asio::read_until(*socket, response, "\r\n");

	// validate response
	std::istream responseStream(&response);
	std::string httpVersion;
	unsigned int statusCode;
	std::string statusMessage;

	responseStream >> httpVersion;
	responseStream >> statusCode;
	std::getline(responseStream, statusMessage);

	if (!responseStream || httpVersion.substr(0, 5) != "HTTP/") {
		std::cout << "Invalid response\n";
		return false;
	}

	if (statusCode != 200) {
		std::cout << "Response returned with status code: " << statusCode << "\n";
		std::cout << statusMessage << std::endl;
		return false;
	}

	return true;
}
