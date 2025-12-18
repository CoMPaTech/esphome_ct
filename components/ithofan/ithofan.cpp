#include "esphome/core/log.h"
#include "esphome/core/helpers.h"
#include "esphome/core/hal.h"
#include "ithofan.h"
#include <cinttypes>

namespace esphome {
namespace ithofan {

static const char *const TAG = "ithofan";
static const int32_t SYMBOL = 640;

IthoFanComponent *IthoFanComponent::singleton_ = nullptr;

void IthoFanComponent::dump_config() {
  ESP_LOGCONFIG(TAG, "IthoFan:");
  ESP_LOGCONFIG(TAG, "  Name: %" PRIx32, this->address_);
  ESP_LOGCONFIG(TAG, "  Code: %" PRIu16, this->code_);
}

void IthoFanComponent::setup() {
  uint32_t type = fnv1_hash(std::string("IthoFan: ") + format_hex(this->address_));
  this->preferences_ = global_preferences->make_preference<uint16_t>(type);
  this->preferences_.load(&this->code_);
  auto addr_pref = global_preferences->make_preference<uint32_t>(type + 1);
  if (!addr_pref.load(&this->address_)) {
    // If never set, default to 0x000000
    this->address_ = 0x000000;
    addr_pref.save(&this->address_);
  }
}

void IthoFanComponent::set_code(uint16_t code) {
  ESP_LOGD(TAG, "IthoFan updating code to %" PRIu16 " from %" PRIu16, code, this->code_);
  this->code_ = code;
  this->preferences_.save(&this->code_);
}

void IthoFanComponent::send_command(IthoCommand command, uint32_t repeat) {
  // Build the 7‑byte frame exactly as you already do
  uint8_t frame[7];
  frame[0] = 0xA7;
  frame[1] = command << 4;
  frame[2] = this->code_ >> 8;
  frame[3] = this->code_;
  frame[4] = this->address_ >> 16;
  frame[5] = this->address_ >> 8;
  frame[6] = this->address_;

  // CRC and obfuscation as before
  uint8_t crc = 0;
  for (int i = 0; i < 7; i++) {
    crc ^= frame[i];
    crc ^= frame[i] >> 4;
  }
  frame[1] |= crc & 0xF;
  for (int i = 1; i < 7; i++) {
    frame[i] ^= frame[i - 1];
  }

  // bump rolling code
  this->code_++;
  this->preferences_.save(&this->code_);

  auto call = cc1101::CC1101Call(id(itho_cc1101));
  call.set_data(frame, sizeof(frame));
  call.set_repeat(repeat);
  call.perform();

}

bool IthoFanComponent::on_receive(const std::vector<uint8_t> &raw) {
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

}  // namespace ithofan
}  // namespace esphome
