/*
 * IMACProtocol.h
 *
 *  Created on: 14 jan. 2015
 *      Author: Jaco
 */
#include <cstdint>
#include "../protocol/MediumState.h"
#include "../protocol/TransmissionInfo.h"
#include "../protocol/TransmissionType.h"

#ifndef IMACPROTOCOL_H_
#define IMACPROTOCOL_H_

namespace protocol {

class IMACProtocol {
public:
	IMACProtocol() {
	}
	virtual ~IMACProtocol() {
	}

	/**
	 * The (emulated) physical layer will announce a new timeslot to the protocol by calling this method.
	 * @param previousMediumState The state of the medium in the latest timeslot
	 * @param controlInformation Control information, if available (when previousMediumState == MediumState.Success). Otherwise undefined.
	 * @param localQueueLength The length of the local packet queue.
	 * @return TransmissionInfo for the physical layer.
	 */
	virtual TransmissionInfo TimeslotAvailable(MediumState previousMediumState, int32_t controlInformation, int32_t localQueueLength)=0;
};
}

#endif /* IMACPROTOCOL_H_ */
