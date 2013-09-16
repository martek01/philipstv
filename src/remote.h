/*
 * remote.h
 *
 *  Created on: Sep 10, 2013
 *      Author: Martek01
 */

#ifndef REMOTE_H_
#define REMOTE_H_

#include <string>
#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/address.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/streambuf.hpp>
#include <boost/property_tree/ptree.hpp>

class Remote {
protected:
	boost::asio::ip::tcp::socket *socket;

	static boost::asio::io_service ioService;

public:
	Remote();
	virtual ~Remote();

	bool connect(boost::asio::ip::address address, unsigned short int port = 1925);
	void disconnect();

	boost::asio::ip::address getAddress();
	unsigned short int getPort();

	bool sendKey(std::string keyName);
	bool standby();
	bool setVolume(int level);
	bool setChannel(std::string channelID);
	bool setSource(std::string sourceName);

	std::string getChannelList();

protected:
	boost::property_tree::ptree sendGetRequest(std::string path);
	bool sendPostRequest(std::string path, boost::property_tree::ptree bodyTree);
};

#endif /* REMOTE_H_ */
