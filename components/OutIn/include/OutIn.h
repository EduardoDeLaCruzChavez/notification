#ifndef DRIVER_OUT_H_
#define DRIVER_OUT_H_
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"

#define PIN_LED 2
#define PIN_RESET 4

void vInitGPIO();
void vWriteLed(bool bState);
bool bReadPin(int pin);
#endif