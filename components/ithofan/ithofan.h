#pragma once

#include "esphome/core/component.h"
#include "esphome/components/fan/fan.h"
#include "esphome/components/cc1101/cc1101.h"

namespace esphome {
namespace ithofan {

enum IthoCommand : uint8_t {
  IthoAway    = 3,
  IthoLow     = 4,
  IthoMedium  = 5,
  IthoHigh    = 6,
  IthoFull    = 7,
  IthoTimer1  = 8,
  IthoTimer2  = 9,
  IthoTimer3  = 10,
};

class IthoFanComponent : public fan::Fan, public Component {
 public:
  void setup() override;
  void dump_config() override;
  void control(const fan::FanCall &call) override;

  void set_radio(cc1101::CC1101Component *radio) { this->radio_ = radio; }
  void set_address(uint32_t addr) { this->address_ = addr; }
  void set_code(uint16_t code) { this->code_ = code; }

 protected:
  void send_command(IthoCommand cmd);

  cc1101::CC1101Component *radio_{nullptr};
  uint32_t address_{0};
  uint16_t code_{0};
};

}  // namespace ithofan
}  // namespace esphome

