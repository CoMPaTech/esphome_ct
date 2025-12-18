#pragma once

#include "esphome/components/cc1101/cc1101.h"
#include "esphome/core/component.h"
#include "esphome/core/preferences.h"

namespace esphome {
  namespace ithofan {

// CC1101 command strobes (per TI datasheet)
enum : uint8_t {
  CC1101_SRX   = 0x34,
  CC1101_STX   = 0x35,
  CC1101_SIDLE = 0x36,
  CC1101_SFRX  = 0x3A,
  CC1101_SFTX  = 0x3B,
};

// CC1101 FIFOs and status regs
enum : uint8_t {
  CC1101_TXFIFO  = 0x3F,  // write-burst to load TX
  CC1101_RXFIFO  = 0x3F,  // read-burst to read RX
  CC1101_RXBYTES = 0x3B,  // status register; read to get RX FIFO count
};

// Helper to send a command strobe using the core API
inline void cc1101_strobe(cc1101::CC1101Component *radio, uint8_t cmd) {
  // Core only provides write_register; sending a strobe is a write to the strobe address
  radio->write_register(cmd, 0x00);
}


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
