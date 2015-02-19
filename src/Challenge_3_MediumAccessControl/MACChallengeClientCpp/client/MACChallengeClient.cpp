/*
 * MACChallengeClient.cpp
 *
 *  Created on: 15 feb. 2015
 *      Author: Jaco
 */

#include "MACChallengeClient.h"

namespace client {

MACChallengeClient::MACChallengeClient(std::string serverAddress,
		int32_t serverPort, int32_t clientGroupId, std::string clientPassword) {
	if (clientPassword == "changeme") {
		std::cerr << "Please change the default password" << std::endl;
		exit(EXIT_FAILURE);
	}

	host = serverAddress;
	std::ostringstream ss;
	ss << serverPort;
	port = ss.str();
	groupId = clientGroupId;
	password = clientPassword;
	sock = 0;

	this->clientConnect();

}

MACChallengeClient::~MACChallengeClient() {
}

void MACChallengeClient::clientConnect() {
	struct addrinfo hints;
	struct addrinfo *result, *rp;
	ssize_t s;

	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_UNSPEC; /* Allow IPv4 or IPv6 */
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = 0;
	hints.ai_protocol = IPPROTO_TCP;

	s = getaddrinfo(host.c_str(), port.c_str(), &hints, &result);
	if (s != 0) {
		std::cerr << "getaddrinfo:" << gai_strerror(s) << std::endl;
		exit(EXIT_FAILURE);
	}

	for (rp = result; rp != NULL; rp = rp->ai_next) {
		sock = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
		if (sock == -1)
			continue;

		if (connect(sock, rp->ai_addr, rp->ai_addrlen) != -1)
			break; /* Success */

		close(sock);
	}

	if (rp == NULL) { /* No address succeeded */
		std::cerr << "Could not connect" << std::endl;
		exit(EXIT_FAILURE);
	}

	freeaddrinfo(result);

	// set non-blocking socket
	ssize_t x;
	x = fcntl(sock, F_GETFL, 0);
	fcntl(sock, F_SETFL, x | O_NONBLOCK);

	// expect hello message
	if (this->getControlMessageBlocking() != "REGISTER") {
		std::cerr << "Did not get expected hello from server" << std::endl;
		exit(EXIT_FAILURE);
	}
	this->clearControlMessage();

	// register
	std::ostringstream ss;
	ss << this->groupId;
	this->sendControlMessage("REGISTER " + ss.str() + " " + this->password);

	std::string reply = getControlMessageBlocking();
	if (reply != "OK") {
		std::string reason = reply.substr(reply.find(' ') + 1);
		std::cerr << "Could not register with server: " << reason << std::endl;
		exit(EXIT_FAILURE);
	}
	clearControlMessage();

	this->eventLoopThread = std::thread(&MACChallengeClient::run, this);
}

/**
 * Sets the listening protocol
 */
void MACChallengeClient::setListener(protocol::IMACProtocol* listener) {
	this->listener = listener;
}

/**
 * Reqests a simulation start from the server
 */
void MACChallengeClient::requestStart() {
	if (!simulationStarted) {
		sendControlMessage("START");
	}
}

/**
 * Starts the simulation
 */
void MACChallengeClient::start() {
	if (!simulationStarted) {
		simulationStarted = true;
	}
}

/**
 * @return whether the simulation has been started
 */
bool MACChallengeClient::isSimulationStarted() {
	return simulationStarted;
}

/**
 * @return whether the simulation has finished
 */
bool MACChallengeClient::isSimulationFinished() {
	return simulationFinished;
}

void MACChallengeClient::stop() {

	// stop simulation
	simulationStarted = false;
	simulationFinished = true;

	// stop the message loop
	this->eventLoopThread.join();

	// close comms
	this->sendControlMessage("CLOSED");
	shutdown(this->sock, 1);
	sleep(1);
	close(this->sock);
}

void MACChallengeClient::run() {
	bool stopThread = false;
	while (!stopThread && !simulationFinished) {
		std::string message = getControlMessageBlocking();
		std::vector<std::string> splitMessage = this->split(message, ' ');

		if (splitMessage.size() > 0 && splitMessage[0].find("FAIL") == 0) {
			if (splitMessage.size() > 1) {
				std::cerr << "Failure: "
						<< message.substr(message.find(' ') + 1);
			}
			clearControlMessage();
			stopThread = true;
			simulationStarted = false;
			simulationFinished = true;

		} else if (splitMessage.size() > 0
				&& splitMessage[0].find("INFO") == 0) {
			std::cerr << "Info: "
									<< message.substr(message.find(' ') + 1);
		} else if (splitMessage.size() > 0
				&& splitMessage[0].find("START") == 0) {
			// start the simulation
			simulationStarted = true;
		} else if (splitMessage.size() > 0
				&& splitMessage[0].find("TIMESLOT") == 0) {
			if (simulationStarted) {
				TransmissionInfo transmissionInfo;
				int32_t queueLength = 0;
				bool foundTransmissionInfo = false;

				// slot was idle
				if (splitMessage.size() == 4
						&& splitMessage[1].find("IDLE") == 0) {
					if (!(std::stringstream(splitMessage[3]) >> queueLength)) {
						std::cerr
								<< "Error parsing int queueLength. Input was: "
								<< splitMessage[3] << std::endl;
						exit(EXIT_FAILURE);
					}
					transmissionInfo = listener->TimeslotAvailable(Idle, 0,
							queueLength);
					foundTransmissionInfo = true;
				}

				// slot was collision
				if (splitMessage.size() == 4
						&& splitMessage[1].find("COLLISION") == 0) {
					if (!(std::stringstream(splitMessage[3]) >> queueLength)) {
						std::cerr
								<< "Error parsing int queueLength. Input was: "
								<< splitMessage[3] << std::endl;
						exit(EXIT_FAILURE);
					}
					transmissionInfo = listener->TimeslotAvailable(Collision, 0,
							queueLength);
					foundTransmissionInfo = true;
				}

				// slot was succesful
				if (splitMessage.size() == 5
						&& splitMessage[1].find("SUCCESS") == 0) {
					int32_t controlInformation;
					if (!(std::stringstream(splitMessage[4]) >> queueLength)) {
						std::cerr
								<< "Error parsing int queueLength. Input was: "
								<< splitMessage[4] << std::endl;
						exit(EXIT_FAILURE);
					}
					if (!(std::stringstream(splitMessage[2])
							>> controlInformation)) {
						std::cerr
								<< "Error parsing int controlInformation. Input was: "
								<< splitMessage[2] << std::endl;
						exit(EXIT_FAILURE);
					}
					transmissionInfo = listener->TimeslotAvailable(Succes,
							controlInformation, queueLength);
					foundTransmissionInfo = true;
				}

				// got strategy from protocol, send it back to server
				if (foundTransmissionInfo) {
					if (transmissionInfo.transmissionType == Data) {
						if (queueLength <= 0) {
							std::cerr
									<< "Cannot transmit data without packets in the queue.";
							exit(EXIT_FAILURE);
						}
						this->sendControlMessage(
								"TRANSMIT "
										+ SSTR(transmissionInfo.controlInformation)
										+ " DATA");
					}
					if (transmissionInfo.transmissionType == NoData) {
						sendControlMessage(
								"TRANSMIT "
										+ SSTR(transmissionInfo.controlInformation)
										+ " NODATA");
					}
					if (transmissionInfo.transmissionType == Silent) {
						sendControlMessage("TRANSMIT 0 SILENT");
					}
				}
			}
		} else if (splitMessage.size() > 0
				&& (splitMessage[0].find("FINISH") == 0
						|| splitMessage[0].find("CLOSED") == 0)) {
			simulationStarted = false;
			simulationFinished = true;
		}

		clearControlMessage();

		usleep(1000);
	}
}

std::string MACChallengeClient::getControlMessageBlocking() {
	std::string message = "";
	while (message == "") {
		message = this->getControlMessage();
		usleep(1000);
	}

	return message;
}

std::string MACChallengeClient::getControlMessage() {
	char socketBuffer[BUF_SIZE];
	ssize_t nread = read(sock, socketBuffer, BUF_SIZE);
	if (nread != -1) {
		socketBufferStr.append(socketBuffer, nread);
	}
	if (this->currentControlMessage == "") {
		ssize_t pos = socketBufferStr.find('\n');
		if (pos > 0) {
			std::string line = socketBufferStr.substr(0, pos);
			if (line.length() > this->protocolString.length() + 1
					&& line.substr(0, this->protocolString.length())
							== this->protocolString) {
				this->currentControlMessage = line.substr(
						this->protocolString.length() + 1,
						line.length() - (this->protocolString.length() + 1));
				socketBufferStr = socketBufferStr.substr(pos + 1,
						socketBufferStr.length() - (pos + 1));
			} else {
				std::cerr << "Protocol mismatch with server" << std::endl;
				exit(EXIT_FAILURE);
			}
		}
	}

	return this->currentControlMessage;
}

void MACChallengeClient::clearControlMessage() {
	this->currentControlMessage = "";
}

void MACChallengeClient::sendControlMessage(std::string message) {
	std::string controlMessage = this->protocolString + " " + message + "\n";
	if (write(sock, controlMessage.c_str(), controlMessage.length())
			!= (ssize_t) controlMessage.length()) {
		std::cerr << "Write failed" << std::endl;
		exit(EXIT_FAILURE);
	}
}

std::vector<std::string> &MACChallengeClient::split(const std::string &s,
		char delim, std::vector<std::string> &elems) {
	std::stringstream ss(s);
	std::string item;
	while (std::getline(ss, item, delim)) {
		elems.push_back(item);
	}
	return elems;
}

std::vector<std::string> MACChallengeClient::split(const std::string &s,
		char delim) {
	std::vector<std::string> elems;
	split(s, delim, elems);
	return elems;
}

} /* namespace client */
