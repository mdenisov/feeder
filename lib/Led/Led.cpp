#include <Arduino.h>

#include "Led.h"

Led::Led(uint8_t pin) { _pin = pin; }

void Led::begin() {
  pinMode(_pin, OUTPUT);
  digitalWrite(_pin, 0);
}

boolean Led::isEnabled() { return _enabled; }

void Led::blink(uint8_t count) {
  if (setMode(ledMode::BLINK)) {
    _blinkCount = count;
  }
}

void Led::pulse() {
  if (_mode == ledMode::PULSE) {
    return;
  }

  setMode(ledMode::PULSE);
}

void Led::smooth() { setMode(ledMode::SMOOTH); }

void Led::on() { setMode(ledMode::ALWAYS); }

void Led::off() {
  reset();
  digitalWrite(_pin, 0);
}

void Led::tick() {
  if (!isEnabled()) {
    return;
  }

  uint32_t now = millis();

  switch (_mode) {
  case ledMode::ALWAYS:
    digitalWrite(_pin, 1);
    break;
  case ledMode::BLINK:
    if (_blinkCount >= 0) {
      if (now - _timer > 200) {
        _timer = now;
        _blinkCount--;
        uint8_t ledState = digitalRead(_pin);
        digitalWrite(_pin, !ledState);
      }
    } else {
      _blinkCount = 0;
      off();
    }
    break;
  case ledMode::PULSE:
    if (now - _timer > 300) {
      _timer = now;
      uint8_t ledState = digitalRead(_pin);
      digitalWrite(_pin, !ledState);
    }
    break;
  case ledMode::SMOOTH:
    int periode = 2000;
    uint8_t value = 128 + 127 * cos(2 * PI / periode * now);
    // uint8_t value = 128 + 127 * cos(2 * PI / periode * (500 - now));

    analogWrite(_pin, value);
    break;
  }
}

// Private
void Led::reset() {
  _blinkCount = 0;
  _blinkVal = 0;
  _blinkDir = true;
};

void Led::enable() { _enabled = true; };

boolean Led::setMode(ledMode mode) {
  if (_mode == mode) {
    if (mode == ledMode::BLINK && _blinkCount) {
      return false;
    }

    return false;
  }

  _mode = mode;

  reset();
  enable();

  return true;
};