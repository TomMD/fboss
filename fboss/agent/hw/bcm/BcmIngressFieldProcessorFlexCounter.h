/*
 *  Copyright (c) 2004-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree. An additional grant
 *  of patent rights can be found in the PATENTS file in the same directory.
 *
 */
#pragma once

#include "fboss/agent/hw/bcm/BcmFlexCounter.h"

namespace facebook::fboss {

class BcmIngressFieldProcessorFlexCounter : public BcmFlexCounter {
 public:
  BcmIngressFieldProcessorFlexCounter(int unit, int groupID);
  ~BcmIngressFieldProcessorFlexCounter() = default;

  void attach(BcmAclEntryHandle acl);

 private:
  // Forbidden copy constructor and assignment operator
  BcmIngressFieldProcessorFlexCounter(
      BcmIngressFieldProcessorFlexCounter const&) = delete;
  BcmIngressFieldProcessorFlexCounter& operator=(
      BcmIngressFieldProcessorFlexCounter const&) = delete;
};
} // namespace facebook::fboss
