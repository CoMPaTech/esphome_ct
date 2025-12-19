#include "ithofan.h"
#include "esphome/core/log.h"

namespace esphome {
namespace ithofan {

static const char *const TAG = "ithofan";

void IthoFanComponent::setup() {
  ESP_LOGCONFIG(TAG, "Setting up IthoFan");
}

void IthoFanComponent::dump_config() {
  ESP_LOGCONFIG(TAG, "IthoFan:");
  ESP_LOGCONFIG(TAG, "  Address: 0x%08X", this->address_);
  ESP_LOGCONFIG(TAG, "  Code: %u", this->code_);
}

void IthoFanComponent::control(const fan::FanCall &call) {
  if (call.get_state().has_value()) {
    if (!*call.get_state()) {
      send_command(IthoAway);
      this->state = false;
      return;
    }
  }
  if (call.get_speed().has_value()) {
    auto speed = *call.get_speed();
    switch (speed) {
      case fan::FanSpeed::FAN_SPEED_LOW:    send_command(IthoLow); break;
      case fan::FanSpeed::FAN_SPEED_MEDIUM: send_command(IthoMedium); break;
      case fan::FanSpeed::FAN_SPEED_HIGH:   send_command(IthoHigh); break;
      default:                              send_command(IthoAway); break;
    }
    this->state = true;
    this->speed = speed;
  }
}

static inline void build_itho_frame(uint8_t *frame, uint8_t cmd, uint16_t code, uint32_t addr) {
  frame[0] = 0xA7;
  frame[1] = (cmd << 4);
  frame[2] = code >> 8;
  frame[3] = code & 0xFF;
  frame[4] = (addr >> 16) & 0xFF;
  frame[5] = (addr >> 8) & 0xFF;
  frame[6] = addr & 0xFF;

  // CRC nibble
  uint8_t crc = 0;
  for (int i = 0; i < 7; i++) {
    crc ^= frame[i];
    crc ^= frame[i] >> 4;
  }
  frame[1] |= (crc & 0x0F);

  // Obfuscation
  for (int i = 1; i < 7; i++) frame[i] ^= frame[i - 1];
}

void IthoFanComponent::send_command(IthoCommand cmd) {
  if (!radio_) {
    ESP_LOGW(TAG, "No CC1101 radio bound");
    return;
  }

  uint8_t frame[7];
  build_itho_frame(frame, static_cast<uint8_t>(cmd), this->code_, this->address_);
  this->code_++;

  // Packet-mode path: begin_tx -> action send_packet -> end_tx
  // If CC1101Component exposes an action API from C++, call the method.
  // Otherwise, you may need to expose a small helper in CC1101Component to send the packet buffer.
  radio_->begin_tx();

  // IMPORTANT: call the actual method your cc1101.cpp exposes for packet-mode sending.
  // If there is no C++ public method, add one, e.g. `send_packet(const uint8_t *data, size_t len)`.
  radio_->send_packet(frame, sizeof(frame));

  radio_->end_tx();
  ESP_LOGD(TAG, "Sent Itho command %u", static_cast<uint8_t>(cmd));
}

}  // namespace ithofan
}  // namespace esphome

