/*
 * TransmissionInfo.h
 *
 *  Created on: 15 feb. 2015
 *      Author: Jaco
 */

#include <cstdint>
#include "../protocol/TransmissionType.h"

#ifndef TRANSMISSIONINFO_H_
#define TRANSMISSIONINFO_H_

struct TransmissionInfo {
	TransmissionType transmissionType;
	int32_t controlInformation;
};

#endif /* TRANSMISSIONINFO_H_ */
