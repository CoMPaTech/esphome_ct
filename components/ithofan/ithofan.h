#pragma once

#include "esphome/core/preferences.h"
#include "esphome/components/remote_transmitter/remote_transmitter.h"
#include "esphome/components/remote_receiver/remote_receiver.h"

namespace esphome {
namespace ithofan {

struct IthoRFDevice
{
  uint32_t deviceId{0};
  RemoteTypes remType{RemoteTypes::RFTCVE};
  //  char name[16];
  IthoCommand lastCommand{IthoUnknown};
  time_t timestamp;
  uint8_t counter;
  bool bidirectional{false};
  int32_t co2{0xEFFF};
  int32_t temp{0xEFFF};
  int32_t hum{0xEFFF};
  uint8_t pir{0xEF};
  int32_t dewpoint{0xEFFF};
  int32_t battery{0xEFFF};
};  

enum IthoCommand : uint16_t
{
  IthoUnknown = 0,

  IthoJoin = 1,
  IthoLeave = 2,

  IthoAway = 3,
  IthoLow = 4,
  IthoMedium = 5,
  IthoHigh = 6,
  IthoFull = 7,

  IthoTimer1 = 8,
  IthoTimer2 = 9,
  IthoTimer3 = 10,

  IthoAuto = 11,
  IthoAutoNight = 12,

  IthoCook30 = 13,
  IthoCook60 = 14,

  IthoTimerUser = 15,

  IthoJoinReply = 16,

  IthoPIRmotionOn = 17,
  IthoPIRmotionOff = 18,

};

enum RemoteTypes : uint16_t
{
  UNSETTYPE = 0x0000,
  RFTCVE = 0x22F1,
  RFTAUTO = 0x22F3,
  RFTAUTON = 0x22F4,
  DEMANDFLOW = 0x22F8,
  RFTRV = 0x12A0,
  RFTCO2 = 0x1298,
  RFTPIR = 0x2E10,
  ORCON15LF01 = 0x6710
};

#define F_MASK 0x03
#define F_RQ 0x00
#define F_I 0x01
#define F_W 0x02
#define F_RP 0x03

#define F_ADDR0 0x10
#define F_ADDR1 0x20
#define F_ADDR2 0x40

#define F_PARAM0 0x04
#define F_PARAM1 0x08
#define F_RSSI 0x80

// Only used for received fields
#define F_OPCODE 0x01
#define F_LEN 0x02

#define MAX_PAYLOAD 64
#define MAX_DECODED MAX_PAYLOAD + 18

static char const *const MsgType[4] = {"RQ", "_I", "_W", "RP"};

// General command structure:
// < opcode 2 bytes >< len 1 byte >< command len bytes >

// message command bytes for CVE/HRU remote (536-0124)
const uint8_t ithoMessageAwayCommandBytes[] = {0x22, 0xF1, 0x03, 0x00, 0x01, 0x04};
const uint8_t ithoMessageLowCommandBytes[] = {0x22, 0xF1, 0x03, 0x00, 0x02, 0x04};
const uint8_t ithoMessageMediumCommandBytes[] = {0x22, 0xF1, 0x03, 0x00, 0x03, 0x04};
const uint8_t ithoMessageHighCommandBytes[] = {0x22, 0xF1, 0x03, 0x00, 0x04, 0x04};
const uint8_t ithoMessageFullCommandBytes[] = {0x22, 0xF1, 0x03, 0x00, 0x04, 0x04};
const uint8_t ithoMessageTimer1CommandBytes[] = {0x22, 0xF3, 0x03, 0x00, 0x00, 0x0A}; // 10 minutes full speed
const uint8_t ithoMessageTimer2CommandBytes[] = {0x22, 0xF3, 0x03, 0x00, 0x00, 0x14}; // 20 minutes full speed
const uint8_t ithoMessageTimer3CommandBytes[] = {0x22, 0xF3, 0x03, 0x00, 0x00, 0x1E}; // 30 minutes full speed

// message command bytes specific for AUTO RFT (536-0150)
const uint8_t ithoMessageAUTORFTLowCommandBytes[] = {0x22, 0xF1, 0x03, 0x63, 0x02, 0x04};
const uint8_t ithoMessageAUTORFTAutoCommandBytes[] = {0x22, 0xF1, 0x03, 0x63, 0x03, 0x04};
const uint8_t ithoMessageAUTORFTAutoNightCommandBytes[] = {0x22, 0xF8, 0x03, 0x63, 0x02, 0x03};
const uint8_t ithoMessageAUTORFTHighCommandBytes[] = {0x22, 0xF1, 0x03, 0x63, 0x04, 0x04};
const uint8_t ithoMessageAUTORFTTimer1CommandBytes[] = {0x22, 0xF3, 0x03, 0x63, 0x00, 0x0A};
const uint8_t ithoMessageAUTORFTTimer2CommandBytes[] = {0x22, 0xF3, 0x03, 0x63, 0x00, 0x14};
const uint8_t ithoMessageAUTORFTTimer3CommandBytes[] = {0x22, 0xF3, 0x03, 0x63, 0x00, 0x1E};

// message command bytes specific for RFT-RV (04-00046) and RFT-CO2  (04-00045)
const uint8_t ithoMessageRV_CO2MediumCommandBytes[] = {0x22, 0xF1, 0x03, 0x00, 0x03, 0x07};
const uint8_t ithoMessageRV_CO2AutoCommandBytes[] = {0x22, 0xF1, 0x03, 0x00, 0x05, 0x07};
const uint8_t ithoMessageRV_CO2AutoNightCommandBytes[] = {0x22, 0xF1, 0x03, 0x00, 0x0B, 0x0B};
const uint8_t ithoMessageRV_CO2Timer1CommandBytes[] = {0x22, 0xF3, 0x07, 0x00, 0x00, 0x0A, 0x00, 0x00, 0x00, 0x00};
const uint8_t ithoMessageRV_CO2Timer2CommandBytes[] = {0x22, 0xF3, 0x07, 0x00, 0x00, 0x14, 0x00, 0x00, 0x00, 0x00};
const uint8_t ithoMessageRV_CO2Timer3CommandBytes[] = {0x22, 0xF3, 0x07, 0x00, 0x00, 0x1E, 0x00, 0x00, 0x00, 0x00};

// message command bytes specific for DemandFlow remote (536-0146)
const uint8_t ithoMessageDFLowCommandBytes[] = {0x22, 0xF8, 0x03, 0x00, 0x01, 0x02};
const uint8_t ithoMessageDFHighCommandBytes[] = {0x22, 0xF8, 0x03, 0x00, 0x02, 0x02};
const uint8_t ithoMessageDFTimer1CommandBytes[] = {0x22, 0xF3, 0x05, 0x00, 0x42, 0x03, 0x03, 0x03};
const uint8_t ithoMessageDFTimer2CommandBytes[] = {0x22, 0xF3, 0x05, 0x00, 0x42, 0x06, 0x03, 0x03};
const uint8_t ithoMessageDFTimer3CommandBytes[] = {0x22, 0xF3, 0x05, 0x00, 0x42, 0x09, 0x03, 0x03};
const uint8_t ithoMessageDFCook30CommandBytes[] = {0x22, 0xF3, 0x05, 0x00, 0x02, 0x1E, 0x02, 0x03};
const uint8_t ithoMessageDFCook60CommandBytes[] = {0x22, 0xF3, 0x05, 0x00, 0x02, 0x3C, 0x02, 0x03};

// message command bytes specific for RFT PIR remote
const uint8_t ithoMessageRFTPIRonCommandBytes[] = {0x2E, 0x10, 0x03, 0x00, 0x01, 0x00};
const uint8_t ithoMessageRFTPIRoffCommandBytes[] = {0x2E, 0x10, 0x03, 0x00, 0x00, 0x00};

// Join/Leave command structure:
// < opcode 2 bytes >< len 1 byte >[next command + device ID block repeats len/6 times]< command 3 bytes >< device ID 3 bytes >

// Join/Leave commands:
const uint8_t ithoMessageCVERFTJoinCommandBytes[] = {0x1F, 0xC9, 0x0C, 0x00, 0x22, 0xF1, 0x00, 0x00, 0x00, 0x01, 0x10, 0xE0, 0x00, 0x00, 0x00};                                                                                                          // join command of CVE/HRU remote (536-0124)
const uint8_t ithoMessageAUTORFTJoinCommandBytes[] = {0x1F, 0xC9, 0x0C, 0x63, 0x22, 0xF8, 0x00, 0x00, 0x00, 0x01, 0x10, 0xE0, 0x00, 0x00, 0x00};                                                                                                         // join command of AUTO RFT (536-0150)
const uint8_t ithoMessageDFJoinCommandBytes[] = {0x1F, 0xC9, 0x0C, 0x00, 0x22, 0xF8, 0x00, 0x00, 0x00, 0x00, 0x10, 0xE0, 0x00, 0x00, 0x00};                                                                                                              // join command of DemandFlow remote (536-0146)
const uint8_t ithoMessageRVJoinCommandBytes[] = {0x1F, 0xC9, 0x18, 0x00, 0x31, 0xE0, 0x00, 0x00, 0x00, 0x00, 0x12, 0xA0, 0x00, 0x00, 0x00, 0x01, 0x10, 0xE0, 0x00, 0x00, 0x00, 0x00, 0x1F, 0xC9, 0x00, 0x00, 0x00};                                      // join command of RFT-RV (04-00046)
const uint8_t ithoMessageCO2JoinCommandBytes[] = {0x1F, 0xC9, 0x1E, 0x00, 0x31, 0xE0, 0x00, 0x00, 0x00, 0x00, 0x12, 0x98, 0x00, 0x00, 0x00, 0x00, 0x2E, 0x10, 0x00, 0x00, 0x00, 0x01, 0x10, 0xE0, 0x00, 0x00, 0x00, 0x00, 0x1F, 0xC9, 0x00, 0x00, 0x00}; // join command of RFT-CO2  (04-00045)
const uint8_t ithoMessagePIRJoinCommandBytes[] = {0x1F, 0xC9, 0x06, 0x00, 0x2E, 0x10, 0x00, 0x00, 0x00};                                                                                                                                                 // join command of RF PIR
const uint8_t ithoMessageLeaveCommandBytes[] = {0x1F, 0xC9, 0x06, 0x00, 0x1F, 0xC9, 0x00, 0x00, 0x00};                                                                                                                                                   // standard leave command
const uint8_t ithoMessageAUTORFTLeaveCommandBytes[] = {0x1F, 0xC9, 0x06, 0x63, 0x1F, 0xC9, 0x00, 0x00, 0x00};                                                                                                                                            // leave command of AUTO RFT (536-0150)
const uint8_t ithoMessageAUTORFTNJoinCommandBytes[] = {0x1F, 0xC9, 0x12, 0x00, 0x22, 0xF8, 0x00, 0x00, 0x00, 0x01, 0x10, 0xE0, 0x00, 0x00, 0x00, 0x00, 0x1F, 0xC9, 0x00, 0x00, 0x00};                                                                    // join command of Auto RFT-N (04-00161) (bi-directioal)

// Join/Leave reply commands:
const uint8_t ithoMessageJoinReplyCommandBytes[] = {0x1F, 0xC9, 0x0C, 0x00, 0x31, 0xD9, 0x00, 0x00, 0x00, 0x00, 0x31, 0xDA, 0x00, 0x00, 0x00}; // leave command of AUTO RFT (536-0150)

// Orcon remote VMN-15LF01
const uint8_t orconMessageAwayCommandBytes[] = {0x22, 0xF1, 0x03, 0x00, 0x00, 0x04};
const uint8_t orconMessageAutoCommandBytes[] = {0x22, 0xF1, 0x03, 0x00, 0x04, 0x04};
const uint8_t orconMessageButton1CommandBytes[] = {0x22, 0xF1, 0x03, 0x00, 0x01, 0x04};
const uint8_t orconMessageButton2CommandBytes[] = {0x22, 0xF1, 0x03, 0x00, 0x02, 0x04};
const uint8_t orconMessageButton3CommandBytes[] = {0x22, 0xF1, 0x03, 0x00, 0x03, 0x04};
const uint8_t orconMessageTimer1CommandBytes[] = {0x22, 0xF3, 0x07, 0x00, 0x02, 0x0F, 0x03, 0x04, 0x00, 0x00};
const uint8_t orconMessageTimer2CommandBytes[] = {0x22, 0xF3, 0x07, 0x00, 0x02, 0x1E, 0x03, 0x04, 0x00, 0x00};
const uint8_t orconMessageTimer3CommandBytes[] = {0x22, 0xF3, 0x07, 0x00, 0x02, 0x3C, 0x03, 0x04, 0x00, 0x00};
const uint8_t orconMessageFilterCleanCommandBytes[] = {0x10, 0xD0, 0x02, 0x00, 0xFF};
const uint8_t orconMessageJoinCommandBytes[] = {0x1F, 0xC9, 0x12, 0x00, 0x22, 0xF1, 0x00, 0x00, 0x00, 0x00, 0x22, 0xF3, 0x00, 0x00, 0x00, 0x67, 0x10, 0xE0, 0x00, 0x00, 0x00};
const uint8_t orconMessageBatteryStatusCommandBytes[] = {0x10, 0x60, 0x03, 0x00, 0xFF, 0x01};


class IthoFanSensor {
 public:
  // virtual void update_sunny(uint32_t address, bool value) {}
  // virtual void update_windy(uint32_t address, bool value) {}
};

class IthoFanComponent : public Component, public remote_base::RemoteReceiverListener {
 public:
  static IthoFanComponent *singleton_;   // NEW

  IthoFanComponent() { singleton_ = this; }  // NEW

  static IthoFanComponent *get_singleton() { return singleton_; } // NEW

  float get_setup_priority() const override { return setup_priority::LATE; }
  void setup() override;
  void dump_config() override;
  bool on_receive(remote_base::RemoteReceiveData data) override;
  void send_command(IthoFanCommand command, uint32_t repeat = 4);
  void set_code(uint16_t code);
  void set_tx(remote_transmitter::RemoteTransmitterComponent *tx) { this->tx_ = tx; }
  void set_rx(remote_receiver::RemoteReceiverComponent *rx) { this->rx_ = rx; }
  void set_address(uint32_t address) { this->address_ = address; }
 protected:
  remote_transmitter::RemoteTransmitterComponent *tx_{nullptr};
  remote_receiver::RemoteReceiverComponent *rx_{nullptr};
  ESPPreferenceObject preferences_;
  uint32_t address_;
  uint16_t code_;
};


}  // namespace ithofan
}  // namespace esphome
