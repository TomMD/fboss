/*
 *  Copyright (c) 2004-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree. An additional grant
 *  of patent rights can be found in the PATENTS file in the same directory.
 *
 */
#include "fboss/agent/Platform.h"
#include "fboss/agent/hw/test/ConfigFactory.h"
#include "fboss/agent/hw/test/HwLinkStateDependentTest.h"
#include "fboss/agent/hw/test/HwTestCoppUtils.h"
#include "fboss/agent/hw/test/HwTestPacketUtils.h"
#include "fboss/agent/packet/EthHdr.h"
#include "fboss/agent/test/ResourceLibUtil.h"
#include "folly/Utility.h"

#include <folly/IPAddress.h>
#include <folly/container/Array.h>
#include <thread>

using namespace std::chrono_literals;

namespace facebook::fboss {

class HwPacketSendTest : public HwLinkStateDependentTest {
 protected:
  cfg::SwitchConfig initialConfig() const override {
    auto cfg = utility::oneL3IntfConfig(
        getHwSwitch(), masterLogicalPortIds()[0], cfg::PortLoopbackMode::MAC);
    utility::setDefaultCpuTrafficPolicyConfig(cfg, getAsic());
    utility::addCpuQueueConfig(cfg, getAsic());
    return cfg;
  }
  HwSwitchEnsemble::Features featuresDesired() const override {
    return {HwSwitchEnsemble::LINKSCAN, HwSwitchEnsemble::PACKET_RX};
  }
};

class HwPacketSendReceiveTest : public HwLinkStateDependentTest {
 protected:
  cfg::SwitchConfig initialConfig() const override {
    auto cfg = utility::oneL3IntfTwoPortConfig(
        getHwSwitch(),
        masterLogicalPortIds()[0],
        masterLogicalPortIds().back(),
        cfg::PortLoopbackMode::MAC);
    utility::setDefaultCpuTrafficPolicyConfig(cfg, getAsic());
    utility::addCpuQueueConfig(cfg, getAsic());
    return cfg;
  }
  HwSwitchEnsemble::Features featuresDesired() const override {
    return {HwSwitchEnsemble::LINKSCAN, HwSwitchEnsemble::PACKET_RX};
  }
  void packetReceived(RxPacket* pkt) noexcept override {
    XLOG(DBG1) << "packet received from port " << pkt->getSrcPort();
    srcPort_ = pkt->getSrcPort();
    numPkts_++;
  }
  int getLastPktSrcPort() {
    return srcPort_;
  }
  bool verifyNumPktsReceived(int expectedNumPkts) {
    auto retries = 5;
    do {
      if (numPkts_ == expectedNumPkts) {
        return true;
      }
      std::this_thread::sleep_for(20ms);
    } while (retries--);
    return false;
  }
  std::atomic<int> srcPort_{-1};
  std::atomic<int> numPkts_{0};
};

TEST_F(HwPacketSendTest, LldpToFrontPanelOutOfPort) {
  auto setup = [=]() {};
  auto verify = [=]() {
    auto portStatsBefore = getLatestPortStats(masterLogicalPortIds()[0]);
    auto vlanId = VlanID(*initialConfig().vlanPorts_ref()[0].vlanID_ref());
    auto intfMac = utility::getInterfaceMac(getProgrammedState(), vlanId);
    auto srcMac = utility::MacAddressGenerator().get(intfMac.u64NBO() + 1);
    auto payLoadSize = 256;
    auto txPacket = utility::makeEthTxPacket(
        getHwSwitch(),
        vlanId,
        srcMac,
        folly::MacAddress("01:80:c2:00:00:0e"),
        facebook::fboss::ETHERTYPE::ETHERTYPE_LLDP,
        std::vector<uint8_t>(payLoadSize, 0xff));
    // vlan tag should be removed
    auto pktLengthSent = EthHdr::SIZE + payLoadSize;
    getHwSwitchEnsemble()->ensureSendPacketOutOfPort(
        std::move(txPacket), masterLogicalPortIds()[0], std::nullopt);
    auto portStatsAfter = getLatestPortStats(masterLogicalPortIds()[0]);
    XLOG(INFO) << "Lldp Packet:"
               << " before pkts:" << *portStatsBefore.outMulticastPkts__ref()
               << ", after pkts:" << *portStatsAfter.outMulticastPkts__ref()
               << ", before bytes:" << *portStatsBefore.outBytes__ref()
               << ", after bytes:" << *portStatsAfter.outBytes__ref();
    EXPECT_EQ(
        pktLengthSent,
        *portStatsAfter.outBytes__ref() - *portStatsBefore.outBytes__ref());
    if (getAsic()->getAsicType() != HwAsic::AsicType::ASIC_TYPE_TAJO) {
      EXPECT_EQ(
          1,
          *portStatsAfter.outMulticastPkts__ref() -
              *portStatsBefore.outMulticastPkts__ref());
    }
  };
  verifyAcrossWarmBoots(setup, verify);
}

TEST_F(HwPacketSendTest, ArpRequestToFrontPanelPortSwitched) {
  auto setup = [=]() {};
  auto verify = [=]() {
    auto portStatsBefore = getLatestPortStats(masterLogicalPortIds()[0]);
    auto vlanId = VlanID(*initialConfig().vlanPorts_ref()[0].vlanID_ref());
    auto intfMac = utility::getInterfaceMac(getProgrammedState(), vlanId);
    auto srcMac = utility::MacAddressGenerator().get(intfMac.u64NBO() + 1);
    auto randomIP = folly::IPAddressV4("1.1.1.5");
    auto txPacket = utility::makeARPTxPacket(
        getHwSwitch(),
        vlanId,
        srcMac,
        folly::MacAddress("ff:ff:ff:ff:ff:ff"),
        folly::IPAddress("1.1.1.2"),
        randomIP,
        ARP_OPER::ARP_OPER_REQUEST,
        std::nullopt);
    getHwSwitchEnsemble()->ensureSendPacketSwitched(std::move(txPacket));
    auto portStatsAfter = getLatestPortStats(masterLogicalPortIds()[0]);
    XLOG(INFO) << "ARP Packet:"
               << " before pkts:" << *portStatsBefore.outBroadcastPkts__ref()
               << ", after pkts:" << *portStatsAfter.outBroadcastPkts__ref()
               << ", before bytes:" << *portStatsBefore.outBytes__ref()
               << ", after bytes:" << *portStatsAfter.outBytes__ref();
    EXPECT_NE(
        0, *portStatsAfter.outBytes__ref() - *portStatsBefore.outBytes__ref());
    if (getAsic()->getAsicType() != HwAsic::AsicType::ASIC_TYPE_TAJO) {
      EXPECT_EQ(
          1,
          *portStatsAfter.outBroadcastPkts__ref() -
              *portStatsBefore.outBroadcastPkts__ref());
    }
  };
  verifyAcrossWarmBoots(setup, verify);
}

TEST_F(HwPacketSendReceiveTest, LldpPacketReceiveSrcPort) {
  auto setup = [=]() {};
  auto verify = [=]() {
    if (!isSupported(HwAsic::Feature::PKTIO)) {
      return;
    }
    auto vlanId = VlanID(*initialConfig().vlanPorts_ref()[0].vlanID_ref());
    auto intfMac = utility::getInterfaceMac(getProgrammedState(), vlanId);
    auto srcMac = utility::MacAddressGenerator().get(intfMac.u64NBO() + 1);
    auto payLoadSize = 256;
    auto expectedNumPktsReceived = 1;
    for (auto port :
         {masterLogicalPortIds()[0], masterLogicalPortIds().back()}) {
      auto txPacket = utility::makeEthTxPacket(
          getHwSwitch(),
          vlanId,
          srcMac,
          folly::MacAddress("01:80:c2:00:00:0e"),
          facebook::fboss::ETHERTYPE::ETHERTYPE_LLDP,
          std::vector<uint8_t>(payLoadSize, 0xff));
      getHwSwitchEnsemble()->ensureSendPacketOutOfPort(
          std::move(txPacket), port, std::nullopt);
      ASSERT_TRUE(verifyNumPktsReceived(expectedNumPktsReceived++));
      EXPECT_EQ(port, getLastPktSrcPort());
    }
  };
  verifyAcrossWarmBoots(setup, verify);
}

} // namespace facebook::fboss
