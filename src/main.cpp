/*
 * main.cpp
 *
 *  Created on: Sep 9, 2013
 *      Author: Martek01
 */

// includes
#include <iostream>
#include <string>
#include <boost/program_options.hpp>
#include <boost/format.hpp>
#include <boost/asio/ip/address.hpp>

#include "main.h"
#include "remote.h"

// global variables
boost::program_options::variables_map variablesMap;
Remote *remote = NULL;
unsigned short int port;

bool handlePrintAndVersionOption(boost::program_options::options_description description) {
	// --help
	if (variablesMap.count("help")) {
		std::cout << description << std::endl;
		return false;
	}

	// --version
	if (variablesMap.count("version")) {
		// get version
		boost::format version("%1%.%2%.%3%");
		version % VERSION_MAJOR;
		version % VERSION_MINOR;
		version % VERSION_MICRO;

		std::cout << "Philips TV remote\n";
		std::cout << "Version: " << version << "\n";
		return false;
	}

	return true;
}

bool verifyArguments() {
	try {
		// check if address was set
		if (!variablesMap.count("address")) {
			std::cout << "TV address was not set\n";
			return false;
		}

		// check if address is valid
		std::string addressString = variablesMap["address"].as<std::string>();
		boost::system::error_code errorCode;
		boost::asio::ip::address address = boost::asio::ip::address::from_string(addressString, errorCode);
		if (errorCode) {
			std::cout << "TV address is invalid: " << addressString << "\n";
			return false;
		}

		// create remote object
		remote = new Remote();
		if (remote == NULL) {
			std::cout << "Remote could not be created\n";
			return false;
		}

		// connect remote
		if (!remote->connect(address, 1925)) {
			return false;
		}
	} catch (std::exception &e) {
		std::cout << e.what() << "\n";
	}

	return true;
}

bool executeCommands() {
	// --key
	if (variablesMap.count("key")) {
		if (!remote->sendKey(variablesMap["key"].as<std::string>())) {
			return false;
		}
	}

	// --standby
	if (variablesMap.count("standby")) {
		if (!remote->standby()) {
			return false;
		}
	}

	// --volume
	if (variablesMap.count("volume")) {
		if (!remote->setVolume(variablesMap["volume"].as<int>())) {
			return false;
		}
	}

	// --channel
	if (variablesMap.count("channel")) {
		if (!remote->setChannel(variablesMap["channel"].as<std::string>())) {
			return false;
		}
	}

	// --source
	if (variablesMap.count("source")) {
		if (!remote->setSource(variablesMap["source"].as<std::string>())) {
			return false;
		}
	}

	return true;
}

int main (int argc, char **argv) {
	// declare program options
	boost::program_options::options_description description("Allowed options");
	description.add_options()
			("help,h", "Print help message")
			("version,v", "Print program version")
			("port,p", boost::program_options::value<unsigned short int>(&port)->default_value(1925), "Set connection port")
			("key,k", boost::program_options::value<std::string>(), "Send key down command")
			("standby,s", "Send TV to stand-by mode")
			("volume,V", boost::program_options::value<int>(), "Set the volume level")
			("channel,c", boost::program_options::value<std::string>(), "Set new channel")
			("channellist,l", "Get the channel list")
			("source,S", boost::program_options::value<std::string>(), "Set the input source")
			;

	boost::program_options::options_description hiddenDescription("Hidden options");
	hiddenDescription.add_options()
			("address", boost::program_options::value<std::string>(), "")
			;

	boost::program_options::positional_options_description positionalDescription;
	positionalDescription.add("address", 1);

	boost::program_options::options_description allDescriptions;
	allDescriptions.add(description);
	allDescriptions.add(hiddenDescription);

	// resolve options
	try {
		boost::program_options::store(boost::program_options::command_line_parser(argc, argv).options(allDescriptions).positional(positionalDescription).run(), variablesMap);
		boost::program_options::notify(variablesMap);
	} catch (std::exception &e) {
		std::cout << e.what() << std::endl;
	}

	// handle options
	if (!handlePrintAndVersionOption(description)) {
		return EXIT_SUCCESS;
	}

	if (!verifyArguments() || !executeCommands()) {
		return EXIT_FAILURE;
	}

	// close connection to the TV
	//remote->disconnect();
	delete remote;

	return EXIT_SUCCESS;
}
