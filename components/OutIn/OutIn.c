#include "OutIn.h"

void vInitGPIO()
{
    gpio_reset_pin(PIN_LED);
    gpio_reset_pin(PIN_RESET);

    gpio_set_direction(PIN_LED, GPIO_MODE_OUTPUT);
    gpio_set_direction(PIN_RESET, GPIO_MODE_INPUT);
    gpio_set_pull_mode(PIN_RESET, GPIO_PULLDOWN_ONLY);
    vWriteLed(false);
}

void vWriteLed(bool bState)
{
    gpio_set_level(PIN_LED, bState);
}

bool bReadPin(int pin)
{

    return (bool)gpio_get_level(PIN_RESET);
}