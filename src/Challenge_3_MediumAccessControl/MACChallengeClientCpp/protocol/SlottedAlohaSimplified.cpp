/*
 * SlottedAlohaSimplified.cpp
 *
 *  Created on: 15 feb. 2015
 *      Author: Jaco
 */

#include "../protocol/SlottedAlohaSimplified.h"

namespace protocol {

SlottedAlohaSimplified::SlottedAlohaSimplified() {
	this->rnd = std::default_random_engine((unsigned int)time(0));
}

SlottedAlohaSimplified::~SlottedAlohaSimplified() {
}

TransmissionInfo SlottedAlohaSimplified::TimeslotAvailable(
		MediumState previousMediumState, int32_t controlInformation,
		int32_t localQueueLength) {
	// No data to send, just be quiet
	if (localQueueLength == 0) {
		return TransmissionInfo { Silent, 0 };
	}

	// Randomly transmit with 60 probability
	if (this->rnd() % 100 < 60) {
		return TransmissionInfo { Data, 0 };
	} else {
		return TransmissionInfo { Silent, 0 };
	}

}

} /* namespace protocol */
