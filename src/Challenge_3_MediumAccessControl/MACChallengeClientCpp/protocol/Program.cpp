/*
 * main.cpp
 *
 *  Created on: 13 jan. 2015
 *      Author: Jaco
 */

#include <iostream>
#include <string>
#include <sys/select.h>
#include <sys/types.h>
#include <cstdint>

#include "../client/MACChallengeClient.h"
#include "../protocol/SlottedAlohaSimplified.h"

using namespace protocol;

// Change to your group number (use your student number)
int32_t groupId = 0;

// Change to your group password (doesn't matter what it is,
// as long as everyone in the group uses the same string)
std::string password = "changeme";

// Change to your protocol implementation
IMACProtocol *protocolImpl;

// Challenge server address
std::string serverAddress = "netsys.student.utwente.nl";

// Challenge server port
int32_t serverPort = 8001;

int main() {
	protocolImpl = new SlottedAlohaSimplified();

	std::cout << "[FRAMEWORK] Starting client... ";

	client::MACChallengeClient macclient(serverAddress, serverPort, groupId,
			password);

	macclient.setListener(protocolImpl);

	std::cout << "[FRAMEWORK] Done." << std::endl;

	std::cout << "[FRAMEWORK] Press Enter to start the simulation..." << std::endl;
	std::cout
			<< "[FRAMEWORK] (Simulation will also be started automatically if another client in the group issues the start command)"
			<< std::endl;


	// listen for cin non-blocking
	struct timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec = 10000;

	bool startCommand = false;
	while (!macclient.isSimulationStarted()
			&& !macclient.isSimulationFinished()) {

		fd_set read_fds;
		FD_ZERO(&read_fds);
		FD_SET(0, &read_fds);
		size_t ins = select(1, &read_fds, NULL, NULL, &tv);
		if (!startCommand && ins > 0) {
			macclient.requestStart();
			startCommand = true;
		}
	}

	std::cout << "[FRAMEWORK] Simulation started!" << std::endl;

	// Wait until the simulation is finished
	while (!macclient.isSimulationFinished()) {
		usleep(100000);
	}

	std::cout << "Simulation finished! Check your performance on the server web interface." << std::endl;

	std::cout << "[FRAMEWORK] Shutting down client... " << std::endl;
	macclient.stop();
	std::cout << "[FRAMEWORK] Done." << std::endl;
}
