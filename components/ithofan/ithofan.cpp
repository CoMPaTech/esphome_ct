#include "esphome/core/log.h"
#include "esphome/core/helpers.h"
#include "esphome/core/hal.h"
#include "ithofan.h"
#include <cinttypes>

namespace esphome {
namespace ithofan {

static const char *const TAG = "ithofan";
static const int32_t SYMBOL = 640;
static IthoFanComponent *IthoFanComponent::singleton_ = nullptr;

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
  this->rx_->register_listener(this);
}

void IthoFanComponent::set_code(uint16_t code) {
  ESP_LOGD(TAG, "IthoFan updating code to %" PRIu16 " from %" PRIu16, code, this->code_);
  this->code_ = code;
  this->preferences_.save(&this->code_);
}

void IthoFanComponent::send_command(IthoFanCommand command, uint32_t repeat) {
  uint8_t frame[7];
  frame[0] = 0xA7;                   // encryption key. Doesn't matter much
  frame[1] = command << 4;           // which button did  you press? The 4 LSB will be the checksum
  frame[2] = this->code_ >> 8;       // rolling code (big endian)
  frame[3] = this->code_;            // rolling code
  frame[4] = this->address_ >> 16;   // remote address
  frame[5] = this->address_ >> 8;    // remote address
  frame[6] = this->address_;         // remote address

  ESP_LOGD(TAG, "IthoFan sending 0x%" PRIX8 " repeated %" PRIu32 " times", command, repeat);

  // crc
  uint8_t crc = 0;
  for (uint8_t i = 0; i < 7; i++) {
    crc ^= frame[i];
    crc ^= frame[i] >> 4;
  }
  frame[1] |= crc & 0xF;

  // obfuscation
  for (uint8_t i = 1; i < 7; i++) {
    frame[i] ^= frame[i - 1];
  }

  // update code
  this->code_ += 1;
  this->preferences_.save(&this->code_);

  // send frame
  auto call = id(this->tx_).transmit();
  remote_base::RemoteTransmitData *dst = call.get_data();
  dst->item(9415, 9565);
  dst->space(80000);
  for (uint32_t i = 0; i < (repeat + 1); i++) {
    // hardware sync, two sync for the first frame, seven for the following ones
    uint32_t syncs = (i == 0) ? 2 : 7;
    for (uint32_t j = 0; j < syncs; j++) {
      dst->item(SYMBOL * 4, SYMBOL * 4);
    }

    // software sync
    dst->item(4550, SYMBOL);

    // data
    for (uint8_t byte : frame) {
      for (uint32_t j = 0; j < 8; j++) {
        if ((byte & 0x80) != 0) {
          dst->space(SYMBOL);
          dst->mark(SYMBOL);
        } else {
          dst->mark(SYMBOL);
          dst->space(SYMBOL);
        }
        byte <<= 1;
      }
    }

    // inter frame silence
    dst->space(415);
    if (i < repeat) {
      dst->space(30000);
    }
  }
  call.perform();
}

bool IthoFanComponent::on_receive(remote_base::RemoteReceiveData data) {
  uint8_t sync_count = 0;
  while (true) {
    if (data.expect_item(SYMBOL * 4, SYMBOL * 4)) {
      sync_count++;
    } else if (data.expect_mark(4550)) {
      break;
    } else {
      return true;
    }
  }
  if (sync_count < 2) {
    return true;
  }
  data.expect_space(SYMBOL);

  uint8_t frame[7];
  for (uint8_t &byte : frame) {
    for (uint32_t i = 0; i < 8; i++) {
      byte <<= 1;
      if (data.expect_mark(SYMBOL) || data.expect_mark(SYMBOL * 2)) {
        data.expect_space(SYMBOL);
        byte |= 0;
      } else if (data.expect_space(SYMBOL) || data.expect_space(SYMBOL * 2)) {
        data.expect_mark(SYMBOL);
        byte |= 1;
      } else {
        return true;
      }
    }
  }

  for (uint8_t i = 6; i >= 1; i--) {
    frame[i] ^= frame[i - 1];
  }

  uint8_t crc = 0;
  for (uint8_t i = 0; i < 7; i++) {
    crc ^= frame[i];
    crc ^= frame[i] >> 4;
  }
  if ((crc & 0xF) == 0) {
    uint8_t command = frame[1] >> 4;
    uint16_t code = (frame[2] << 8) | frame[3];
    uint32_t address = (frame[4] << 16) | (frame[5] << 8) | frame[6];
    ESP_LOGD(TAG, "Received: command: %" PRIx8 ", code: %" PRIu16 ", address %" PRIx32,
             command, code, address);
    if (command == SOMFY_SENSOR) {
      for (auto *sensor : this->sensors_) {
        sensor->update_windy(address, (code & 1) != 0);
        sensor->update_sunny(address, (code & 2) != 0);
      }
    }
  }

  return true;
}

}  // namespace ithofan
}  // namespace esphome
