# SN74HC595 PWM Component for ESPHome

This custom component for ESPHome allows you to control SN74HC595 shift register outputs with PWM (Pulse Width Modulation) capabilities. This means you can not only turn pins on and off but also control the brightness or speed by varying the duty cycle.

## Features

- Control up to 8 outputs per shift register
- Chain multiple shift registers together
- Adjustable PWM frequency
- 8-bit PWM resolution (256 levels)
- Integration with ESPHome's light, fan, and other output components

## Hardware Setup

Connect your SN74HC595 to your ESP32/ESP8266 as follows:

| SN74HC595 Pin | ESP