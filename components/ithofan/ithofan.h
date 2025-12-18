#pragma once

#include "esphome/core/component.h"
#include "esphome/core/components/cc1101/cc1101.h"
#include "esphome/core/preferences.h"

namespace esphome {
  namespace ithofan {

    enum IthoCommand : uint16_t {
      IthoUnknown = 0,
      IthoAway = 3,
      IthoLow = 4,
      IthoMedium = 5,
      IthoHigh = 6,
      IthoFull = 7,
      IthoTimer1 = 8,
      IthoTimer2 = 9,
      IthoTimer3 = 10,
      // … add the rest you need
    };

    class IthoFanComponent : public Component {
     public:
      float get_setup_priority() const override { return setup_priority::LATE; }
      void setup() override;
      void dump_config() override;

      void send_command(IthoCommand command, uint32_t repeat = 4);
      void set_code(uint16_t code);
      void set_address(uint32_t address) { this->address_ = address; }
      void set_radio(cc1101::CC1101Component *radio) { this->radio_ = radio; }

     protected:
      cc1101::CC1101Component *radio_{nullptr};
      ESPPreferenceObject preferences_;
      uint32_t address_{0};
      uint16_t code_{0};
    };

  }  // namespace ithofan
}  // namespace esphome
