#include "esphome/core/component.h"
#include "esphome/core/hal.h"
#include "esphome/components/fan/fan.h"
#include "esphome/core/gpio.h"

namespace esphome {
namespace cc1101fan {

class CC1101Fan : public PollingComponent, public fan::Fan {
 public:
  CC1101Fan(int speed_count, bool map_off_to_zero) : speed_count_(speed_count), map_off_to_zero_(map_off_to_zero) {}
  void set_data_pin(GPIOPin *data_pin) { data_pin_ = data_pin; }
  void setup() override;
  void update() override;
  void set_preset_modes(const std::set<std::string> &presets) { this->preset_modes_ = presets; }
  fan::FanTraits get_traits() override;
  void set_output(void *output);
  void set_fan_speed(int speed);
  void send_other_command(uint8_t other_command);
  static void ITHOinterrupt();
  static void ITHOcheck();

 protected:
  GPIOPin *data_pin_;
  void control(const fan::FanCall &call) override;
  void write_state_();
  void publish_state();
  int speed_count_{};
  bool map_off_to_zero_{};
  //fan::FanTraits traits_;
  std::set<std::string> preset_modes_{};
  int Speed = -1;
  int LastSpeed = -1;
  //static CC1101Fan *instance_;
};

} // namespace cc1101fan
} // namespace esphome

