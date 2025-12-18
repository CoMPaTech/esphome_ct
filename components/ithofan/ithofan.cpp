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

bool IthoFanComponent::on_receive(const std::vector<uint16_t> &symbols) {
  uint8_t sync_count = 0;
  size_t idx = 0;

  // hardware sync: look for repeated SYMBOL*4 marks/spaces
  while (idx + 1 < symbols.size()) {
    if (symbols[idx] == SYMBOL * 4 && symbols[idx + 1] == SYMBOL * 4) {
      sync_count++;
      idx += 2;
    } else if (symbols[idx] == 4550) {
      // software sync mark
      idx++;
      break;
    } else {
      return false;  // unexpected pattern
    }
  }

  if (sync_count < 2) {
    return false;
  }

  // expect a space after sync
  if (idx >= symbols.size() || symbols[idx] != SYMBOL) {
    return false;
  }
  idx++;

  // decode 7‑byte frame
  uint8_t frame[7] = {0};
  for (uint8_t &byte : frame) {
    for (int bit = 0; bit < 8; bit++) {
      if (idx + 1 >= symbols.size()) return false;
      // interpret mark/space pairs
      if (symbols[idx] == SYMBOL && symbols[idx + 1] == SYMBOL) {
        byte = (byte << 1) | 1;
      } else {
        byte = (byte << 1);
      }
      idx += 2;
    }
  }

  // de‑obfuscate
  for (int i = 6; i >= 1; i--) {
    frame[i] ^= frame[i - 1];
  }

  // CRC check
  uint8_t crc = 0;
  for (int i = 0; i < 7; i++) {
    crc ^= frame[i];
    crc ^= frame[i] >> 4;
  }
  if ((crc & 0xF) != 0) return false;

  // extract fields
  uint8_t command = frame[1] >> 4;
  uint16_t code   = (frame[2] << 8) | frame[3];
  uint32_t addr   = (frame[4] << 16) | (frame[5] << 8) | frame[6];

  ESP_LOGD(TAG, "Received: cmd=%" PRIu8 ", code=%" PRIu16 ", addr=%" PRIu32,
           command, code, addr);

  // update internal state
  this->lastCommand = static_cast<IthoCommand>(command);
  this->code_ = code;
  this->address_ = addr;

  return true;
}


}  // namespace ithofan
}  // namespace esphome
