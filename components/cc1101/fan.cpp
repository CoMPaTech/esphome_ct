#include "esphome/core/log.h"
#include "fan.h"
#include "IthoCC1101.h"
#include "Ticker.h"

namespace esphome {
namespace cc1101fan {

IthoCC1101 rf;
//void ITHOinterrupt() IRAM_ATTR;
void ITHOcheck();

// extra for interrupt handling
//bool ITHOhasPacket = false;
Ticker ITHOticker;
int Timer=0;
int LastIDindex = 0;
int OldLastIDindex = 0;
long LastPublish=0; 
bool InitRunned = false;
IthoPacket pkt;
String LastID = "";

// Timer values for hardware timer in Fan
#define Time1      10*60
#define Time2      20*60
#define Time3      30*60


void CC1101Fan::setup() {
  auto restore = this->restore_state_();
  if (restore.has_value()) {
    ESP_LOGD("cc1101_fan", "restoring");
    restore->apply(*this);
  }

  rf.init();
  this->data_pin_->setup();
  this->data_pin_->pin_mode(gpio::FLAG_INPUT);
//  ITHOticker.attach_ms(100, std::bind(&CC1101Fan::check_pin, this));
  
  //this->data_pin_->attach_interrupt(CC1101Fan::ITHOinterrupt, gpio::TriggerMode::RISING);

  //auto gpio_num = this->data_pin_->get_pin();
  //attachInterrupt(digitalPinToInterrupt(gpio_num), []() {
  //    CC1101Fan::ITHOinterrupt();
  //}, RISING);

  //this->data_pin_->attach_interrupt(CC1101Fan::ITHOinterrupt, RISING);
  // Init CC1101
  //pinMode(D1, INPUT);
  //attachInterrupt(D1, CC1101Fan::ITHOinterrupt, RISING);
  rf.initReceive();
}

//void CC1101Fan::check_pin() {
//  if (this->data_pin_->digital_read()) {
//    CC1101Fan::ITHOinterrupt();
//  }
//}

void CC1101Fan::update() {
    CC1101Fan::ITHOcheck();
/*
    // Only publish if the state has changed
    if (fantimer->state != String(Timer).c_str()) {
        fantimer->publish_state(String(Timer).c_str());
    }

    if (lastid->state != LastID.c_str()) {
        lastid->publish_state(LastID.c_str());
    }
*/
}

void CC1101Fan::publish_state() {
  this->state = this->Speed;
  ESP_LOGD("cc1101_fan", "Publishing state: %d from speed %d", this->state, this->Speed);
  this->state_callback_(); // Notify ESPHome about the state change

}

fan::FanTraits CC1101Fan::get_traits() {
    fan::FanTraits traits;
    traits.set_speed(true);  // The fan supports speed control
    traits.set_supported_speed_count(this->speed_count_);  // Number of speeds
    traits.set_oscillation(false);  // The fan does not support oscillation
    traits.set_direction(false);  // The fan does not support direction control
    return traits;
  //return fan::FanTraits(this->oscillation_id_.has_value(), this->speed_id_.has_value(), this->direction_id_.has_value(),
}

void CC1101Fan::control(const fan::FanCall &call) {
  auto State = call.get_state() ? "ON" : "OFF";
  auto Speed = call.get_speed().has_value() ? *call.get_speed() : -1;
  ESP_LOGD("cc1101_fan", "Call state: %s, speed: %d", State, Speed);
         
  if (call.get_speed().has_value() || ( strcmp(State,"ON") && Speed == -1 ) ) {
    // If fans don't have off, just lowest level
    if ( this->map_off_to_zero_ && Speed == -1 ) {
        ESP_LOGD("cc1101_fan", "Correcting with map off to zero speed to 0");
        Speed = 0;
    }
    if ( Speed == -1 ) {
        ESP_LOGD("cc1101_fan", "Correcting speed to 0");
        Speed = 0;
    }
    if ( Speed > this->speed_count_) {
        ESP_LOGD("cc1101_fan", "Speed %d to high, correcting to %d (speed_count)", Speed, this->speed_count_);
        Speed = this->speed_count_;
    }
    ESP_LOGD("cc1101_fan", "Setting speed to %d", Speed);
    this->set_fan_speed(Speed);
  } else {
    // Ensure we do 'off'
    this->set_fan_speed(0);
  }
}

void CC1101Fan::set_fan_speed(int speed) {
  ESP_LOGD("cc1101_fan", "RF called witht %d while last is %d and speed assumed at %d", speed, this->LastSpeed, this->Speed);
  if (speed != this->LastSpeed ) {
    // Handle speed control
    switch (speed) {
      case 4:
        rf.sendCommand(IthoFull);
        break;
      case 3:
        rf.sendCommand(IthoHigh);
        break;
      case 2:
        rf.sendCommand(IthoMedium);
        break;
      case 1:
        rf.sendCommand(IthoLow);
        break;
      case 0:
        rf.sendCommand(IthoLow);
        break;
    }
    this->LastSpeed = this->Speed;
    this->Speed = speed;
    if ( this->map_off_to_zero_ && speed == 0 ) this->Speed = 1;
    // Timer = 0;
  
    this->publish_state();
  } 
}

void CC1101Fan::send_other_command(uint8_t other_command) {
  switch (other_command) {
    case 0: // join
      ESP_LOGD("cc1101_fan", "RF called witht %d, sending Join", other_command);
      rf.sendCommand(IthoJoin);
      break;
    case 1: // timer 1
      ESP_LOGD("cc1101_fan", "RF called witht %d, sending Timer1", other_command);
      rf.sendCommand(IthoTimer1);
      break;
    case 2: // timer 2
      ESP_LOGD("cc1101_fan", "RF called witht %d, sending Timer2", other_command);
      rf.sendCommand(IthoTimer2);
      break;
    case 3: // timer 3
      ESP_LOGD("cc1101_fan", "RF called witht %d, sending Timer3", other_command);
      rf.sendCommand(IthoTimer3);
      break;
  }
}

void CC1101Fan::set_output(void *output) {
  // No-op: This method is required by the ESPHome build system but is unused.
}

//void IRAM_ATTR CC1101Fan::ITHOinterrupt() {
//	ITHOticker.once_ms(10, CC1101Fan::ITHOcheck);
//}

void CC1101Fan::ITHOcheck() {
  //noInterrupts();
  if (rf.checkForNewPacket()) {
    IthoCommand cmd = rf.getLastCommand();
    IthoPacket pkt = rf.getLastPacket();
    LastID = rf.getLastIDstr();
    switch (cmd) {
      case IthoUnknown:
        break;
      case IthoLow:
        ESP_LOGD("c1101_fan", "1 / Low (or 0 / Off)");
        Timer = 0;
        this->LastSpeed = this->Speed;
        this->Speed = 1;
        break;
      case IthoMedium:
        ESP_LOGD("c1101_fan", "2 / Medium");
        Timer = 0;
        this->LastSpeed = this->Speed;
        this->Speed = 2;
        break;
      case IthoHigh:
        ESP_LOGD("c1101_fan", "3 / High");
        Timer = 0;
        this->LastSpeed = this->Speed;
        this->Speed = 3;
        break;
      case IthoFull:
        ESP_LOGD("c1101_fan", "4 / Full");
        Timer = 0;
        this->LastSpeed = this->Speed;
        this->Speed = 4;
        break;
      case IthoTimer1:
        ESP_LOGD("c1101_fan", "Timer1");
        Timer = Time1;
        this->LastSpeed = this->Speed;
        this->Speed = 3;
        break;
      case IthoTimer2:
        ESP_LOGD("c1101_fan", "Timer2");
        Timer = Time2;
        this->LastSpeed = this->Speed;
        this->Speed = 3;
        break;
      case IthoTimer3:
        ESP_LOGD("c1101_fan", "Timer3");
        Timer = Time3;
        this->LastSpeed = this->Speed;
        this->Speed = 3;
        break;
      case IthoJoin:
        ESP_LOGD("c1101_fan", "IthoJoin spotted");
        break;
      case IthoLeave:
        ESP_LOGD("c1101_fan", "IthoLeave spotted");
        break;
      default:
        ESP_LOGD("c1101_fan", "Other command spotted");
        break;
    }
    this->publish_state();
  }
  //interrupts();
};

} // namespace cc1101fan
} // namespace esphome
