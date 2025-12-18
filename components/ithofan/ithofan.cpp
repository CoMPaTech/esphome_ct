#include "esphome/core/log.h"
#include "ithofan.h"
#include <cinttypes>

namespace esphome {
  namespace ithofan {

    static const char *const TAG = "ithofan";

    void IthoFanComponent::setup() {
      uint32_t type = fnv1_hash(std::string("IthoFan: ") + format_hex(this->address_));
      this->preferences_ = global_preferences->make_preference<uint16_t>(type);
      this->preferences_.load(&this->code_);
    }

    void IthoFanComponent::dump_config() {
      ESP_LOGCONFIG(TAG, "IthoFan:");
      ESP_LOGCONFIG(TAG, "  Address: %" PRIx32, this->address_);
      ESP_LOGCONFIG(TAG, "  Code: %" PRIu16, this->code_);
    }

    void IthoFanComponent::set_code(uint16_t code) {
      ESP_LOGD(TAG, "Updating rolling code to %" PRIu16 " from %" PRIu16, code, this->code_);
      this->code_ = code;
      this->preferences_.save(&this->code_);
    }

    void IthoFanComponent::send_command(IthoCommand command, uint32_t repeat) {
      if (!this->radio_) {
        ESP_LOGW(TAG, "No CC1101 radio bound, cannot send");
        return;
      }

      uint8_t frame[7];
      frame[0] = 0xA7;
      frame[1] = command << 4;
      frame[2] = this->code_ >> 8;
      frame[3] = this->code_;
      frame[4] = this->address_ >> 16;
      frame[5] = this->address_ >> 8;
      frame[6] = this->address_;

      // CRC
      uint8_t crc = 0;
      for (int i = 0; i < 7; i++) {
        crc ^= frame[i];
        crc ^= frame[i] >> 4;
      }
      frame[1] |= crc & 0xF;

      // obfuscation
      for (int i = 1; i < 7; i++) {
        frame[i] ^= frame[i - 1];
      }

      this->code_++;
      this->preferences_.save(&this->code_);

      ESP_LOGD(TAG, "Sending command %u, repeat %u", command, repeat);

      // Use CC1101Component API
      #this->radio_->begin_tx();
      cc1101_strobe(this->radio_, CC1101_SIDLE);
      cc1101_strobe(this->radio_, CC1101_SFTX);
      for (uint32_t i = 0; i < repeat; i++) {
        // Write frame into FIFO
        this->radio_->write_burst(CC1101_TXFIFO, frame, sizeof(frame));
        // Start transmission
        cc1101_strobe(CC1101_STX);
      }
      cc1101_strobe(this->radio_, CC1101_SIDLE);
    }

    void IthoFanComponent::begin_rx() {
      if (!this->radio_) return;
      cc1101_strobe(this->radio_, CC1101_SIDLE);
      cc1101_strobe(this->radio_, CC1101_SFRX);
      cc1101_strobe(this->radio_, CC1101_SRX);
    }

    bool IthoFanComponent::on_receive(uint8_t *raw, size_t len) {
      if (len < 7) return false;  // minimum frame size

      // de‑obfuscate
      for (int i = 6; i >= 1; i--) {
        raw[i] ^= raw[i - 1];
      }

      // CRC check
      uint8_t crc = 0;
      for (int i = 0; i < 7; i++) {
        crc ^= raw[i];
        crc ^= raw[i] >> 4;
      }
      if ((crc & 0xF) != 0) return false;

      // extract fields
      uint8_t command = raw[1] >> 4;
      uint16_t code   = (raw[2] << 8) | raw[3];
      uint32_t addr   = (raw[4] << 16) | (raw[5] << 8) | raw[6];

      ESP_LOGD(TAG, "Received: cmd=%" PRIu8 ", code=%" PRIu16 ", addr=%" PRIu32,
               command, code, addr);

      this->lastCommand = static_cast<IthoCommand>(command);
      this->code_ = code;
      this->address_ = addr;

      return true;
    }

    void IthoFanComponent::loop() {
      if (!this->radio_) return;
      // Check RX bytes
      uint8_t rx_bytes = this->radio_->read_register(CC1101_RXBYTES, true) & 0x7F;
      if (rx_bytes >= 7) {
        uint8_t buffer[7];
        this->radio_->read_burst(CC1101_RXFIFO, buffer, 7);
        this->on_receive(buffer, 7);
        }
      }

  }  // namespace ithofan
}  // namespace esphome

