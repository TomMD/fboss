// Copyright 2004-present Facebook. All Rights Reserved.

#include "fboss/agent/hw/bcm/BcmLabeledEgress.h"

extern "C" {
#include <bcm/l3.h>
}

namespace facebook::fboss {

void BcmLabeledEgress::prepareEgressObject(
    bcm_if_t /*intfId*/,
    bcm_port_t /*port*/,
    const std::optional<folly::MacAddress>& /*mac*/,
    RouteForwardAction /*action*/,
    bcm_l3_egress_t* /*eObj*/) const {
  CHECK(0); // no MPLS in OSS
}

} // namespace facebook::fboss
