/*
 * MACChallengeClient.h
 *
 *  Created on: 15 feb. 2015
 *      Author: Jaco
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <cstdint>
#include <unistd.h>
#include <string.h>
#include <sstream>
#include <netdb.h>
#include <iostream>
#include <fcntl.h>
#include <thread>
#include <list>
#include <vector>
#include <mutex>
#include <fstream>
#include <math.h>
#include "../protocol/IMACProtocol.h"

#ifndef CLIENT_MACCHALLENGECLIENT_H_
#define CLIENT_MACCHALLENGECLIENT_H_

#define BUF_SIZE 65536

#define SSTR( x ) dynamic_cast< std::ostringstream & >( \
        ( std::ostringstream() << std::dec << x ) ).str()

namespace client {

class MACChallengeClient {
public:
	MACChallengeClient(std::string serverAddress, int32_t serverPort,
			int32_t clientGroupId, std::string clientPassword);
	virtual ~MACChallengeClient();
	void run();
	void requestStart();
	void start();
	bool isSimulationStarted();
	bool isSimulationFinished();
	void stop();
	void setListener(protocol::IMACProtocol*);

private:
	std::string protocolString = "MACCHALLENGE/1.0";
	ssize_t sock;
	std::string host;
	std::string port;
	ssize_t groupId;
	std::string password;
	std::string currentControlMessage = "";
	bool simulationStarted = false;
	bool simulationFinished = false;
	std::string socketBufferStr;
	std::thread eventLoopThread;
	protocol::IMACProtocol* listener = NULL;

	void clientConnect();
	std::string getControlMessageBlocking();
	std::string getControlMessage();
	void clearControlMessage();
	void sendControlMessage(std::string message);
	std::vector<std::string> &split(const std::string &s, char delim,
			std::vector<std::string> &elems);
	std::vector<std::string> split(const std::string &s, char delim);
};

} /* namespace client */

#endif /* CLIENT_MACCHALLENGECLIENT_H_ */
