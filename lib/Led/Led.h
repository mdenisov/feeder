#ifndef Led_h
#define Led_h

#include <Arduino.h>

enum class ledMode {
  ALWAYS,
  BLINK,
  PULSE,
  SMOOTH,
};

class Led {
public:
  Led(uint8_t pin);
  void begin();
  void blink(uint8_t count);
  void pulse();
  void smooth();
  void on();
  void off();
  void tick();
  boolean isEnabled();

private:
  uint8_t _pin;
  ledMode _mode = ledMode::ALWAYS;
  uint32_t _timer = 0;
  uint8_t _blinkCount = 0;
  uint8_t _blinkVal = 0;
  bool _blinkDir = true;
  bool _enabled = false;

  void reset();
  void enable();
  boolean setMode(ledMode mode);
};
#endif