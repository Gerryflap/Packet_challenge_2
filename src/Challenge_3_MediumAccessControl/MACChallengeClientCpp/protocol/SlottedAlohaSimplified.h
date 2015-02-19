/*
 * SlottedAlohaSimplified.h
 *
 *  Created on: 15 feb. 2015
 *      Author: Jaco
 */

#include "IMACProtocol.h"
#include <cstdint>
#include <cstdlib>
#include <ctime>
#include <random>

#ifndef PROTOCOL_SLOTTEDALOHASIMPLIFIED_H_
#define PROTOCOL_SLOTTEDALOHASIMPLIFIED_H_

namespace protocol {

class SlottedAlohaSimplified: public IMACProtocol {
public:
	SlottedAlohaSimplified();
	~SlottedAlohaSimplified();
	TransmissionInfo TimeslotAvailable(MediumState previousMediumState, int32_t controlInformation, int32_t localQueueLength);
private:
	std::default_random_engine rnd;
};

} /* namespace protocol */

#endif /* PROTOCOL_SLOTTEDALOHASIMPLIFIED_H_ */
