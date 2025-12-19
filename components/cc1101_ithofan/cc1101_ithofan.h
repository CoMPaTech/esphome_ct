#include "esphome.h"
#include "esphome/components/cc1101/cc1101.h"

namespace esphome {
namespace itho_cc1101 {

enum IthoCommand : uint8_t {
  IthoAway   = 3,
  IthoLow    = 4,
  IthoMedium = 5,
  IthoHigh   = 6,
  IthoFull   = 7,
  IthoTimer1 = 8,
  IthoTimer2 = 9,
  IthoTimer3 = 10,
};

class IthoCC1101 : public cc1101::CC1101Component {
 public:
  uint16_t rolling_code{1};
  uint32_t address{0x000000};  // default to zero, can be set

  void send_command(IthoCommand cmd) {
    uint8_t frame[7];
    frame[0] = 0xA7;
    frame[1] = (cmd << 4);
    frame[2] = rolling_code >> 8;
    frame[3] = rolling_code & 0xFF;
    frame[4] = (address >> 16) & 0xFF;
    frame[5] = (address >> 8) & 0xFF;
    frame[6] = address & 0xFF;

    // CRC nibble
    uint8_t crc = 0;
    for (int i = 0; i < 7; i++) {
      crc ^= frame[i];
      crc ^= frame[i] >> 4;
    }
    frame[1] |= (crc & 0x0F);

    // Obfuscation
    for (int i = 1; i < 7; i++)
      frame[i] ^= frame[i - 1];

    rolling_code++;

    std::vector<uint8_t> data(frame, frame + 7);
    this->send_packet_(data);
  }
};

}  // namespace itho_cc1101
}  // namespace esphome

