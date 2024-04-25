#pragma once

#include <Arduino.h>
#include <Stream.h>

class ARA_ESP
{
  public:
  void begin(Stream& serial);
  void roll(float roll);
  void pitch(float pitch);
  void yaw(float yaw);
  void throttle(float throttle);
  void arm(int arm);
  void nav(int nav);
  uint16_t get_channel(int channel);
  
  void main_f();
  bool flag_data;
  private:

  uint16_t ROLL;
  uint16_t PITCH;
  uint16_t YAW;
  uint16_t THROTTLE;
  uint16_t AUX1;
  uint16_t AUX2;
  hw_timer_t *My_timer;
  
};

extern ARA_ESP esp;
