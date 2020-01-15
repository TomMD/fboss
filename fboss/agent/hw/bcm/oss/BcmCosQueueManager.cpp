/*
 *  Copyright (c) 2004-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree. An additional grant
 *  of patent rights can be found in the PATENTS file in the same directory.
 *
 */
#include "fboss/agent/hw/bcm/BcmCosQueueManager.h"

namespace facebook::fboss {

int BcmCosQueueManager::getControlValue(
    cfg::StreamType /*streamType*/,
    bcm_gport_t /*gport*/,
    bcm_cos_queue_t /*cosQ*/,
    BcmCosQueueControlType /*ctrlType*/) const {
  return 0;
}

void BcmCosQueueManager::programControlValue(
    cfg::StreamType /*streamType*/,
    bcm_gport_t /*gport*/,
    bcm_cos_queue_t /*cosQ*/,
    BcmCosQueueControlType /*ctrlType*/,
    int /*value*/) {}

void BcmCosQueueManager::getSchedulingAndWeight(
    bcm_gport_t /*gport*/,
    bcm_cos_queue_t /*cosQ*/,
    PortQueue* /*queue*/) const {}

void BcmCosQueueManager::programSchedulingAndWeight(
    bcm_gport_t /*gport*/,
    bcm_cos_queue_t /*cosQ*/,
    const PortQueue& /*queue*/) {}

void BcmCosQueueManager::getReservedBytes(
    bcm_gport_t /*gport*/,
    bcm_cos_queue_t /*cosQ*/,
    PortQueue* /*queue*/) const {}
void BcmCosQueueManager::programReservedBytes(
    bcm_gport_t /*gport*/,
    int /*queueIdx*/,
    const PortQueue& /*queue*/) {}

void BcmCosQueueManager::getSharedBytes(
    bcm_gport_t /*gport*/,
    bcm_cos_queue_t /*cosQ*/,
    PortQueue* /*queue*/) const {}
void BcmCosQueueManager::programSharedBytes(
    bcm_gport_t /*gport*/,
    int /*queueIdx*/,
    const PortQueue& /*queue*/) {}

void BcmCosQueueManager::getBandwidth(
    bcm_gport_t /*gport*/,
    int /*queueIdx*/,
    PortQueue* /*queue*/) const {}
void BcmCosQueueManager::programBandwidth(
    bcm_gport_t /*gport*/,
    bcm_cos_queue_t /*cosQ*/,
    const PortQueue& /*queue*/) {}

void BcmCosQueueManager::updateQueueAggregatedStat(
    const BcmCosQueueCounterType& /*type*/,
    facebook::stats::MonotonicCounter* /*counter*/,
    std::chrono::seconds /*now*/,
    HwPortStats* /*portStats*/) {}

void BcmCosQueueManager::getCosQueueGportsFromHw() {}

} // namespace facebook::fboss
