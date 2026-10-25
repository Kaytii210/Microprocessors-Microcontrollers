/*
 * input_reading.c
 *
 *  Created on: Oct 22, 2026
 *      Author: user
 */

#include "main.h"
#include "input_reading.h"

#define NUMBER_OF_BUTTONS  3
#define HOLD_START_TICKS  100  // 100 x 10 ms = 1 second
#define HOLD_REPEAT_TICKS  50  // 50 x 10 ms = 500 ms

static const uint16_t button_pins[NUMBER_OF_BUTTONS] = {
    GPIO_PIN_13, GPIO_PIN_14, GPIO_PIN_15
};

static GPIO_PinState last_raw[NUMBER_OF_BUTTONS] = {
    GPIO_PIN_SET, GPIO_PIN_SET, GPIO_PIN_SET
};

static GPIO_PinState stable[NUMBER_OF_BUTTONS] = {
    GPIO_PIN_SET, GPIO_PIN_SET, GPIO_PIN_SET
};

static volatile uint8_t press_count[NUMBER_OF_BUTTONS] = {0};
static uint16_t hold_ticks = 0;
static uint8_t ignore_increase_until_release = 0;

// Called by TIM2 every 10 ms.

void button_reading(void)
{
    for (uint8_t i = 0; i < NUMBER_OF_BUTTONS; i++) {
        GPIO_PinState raw = HAL_GPIO_ReadPin(GPIOB, button_pins[i]);

        if (raw == last_raw[i] && raw != stable[i]) {
            stable[i] = raw;

            if (raw == GPIO_PIN_RESET && press_count[i] < 255U) {
                if (i != BUTTON_INCREASE ||
                    !ignore_increase_until_release) {
                    press_count[i]++;
                }
            }

            if (i == BUTTON_INCREASE) {
                hold_ticks = 0;

                if (raw == GPIO_PIN_SET)
                    ignore_increase_until_release = 0;
            }
        }

        last_raw[i] = raw;
    }

    // Auto-repeat only for INCREASE
    if (stable[BUTTON_INCREASE] == GPIO_PIN_RESET &&
        !ignore_increase_until_release) {

        if (hold_ticks < 65535U)
            hold_ticks++;

        if (hold_ticks >= HOLD_START_TICKS &&
            (hold_ticks - HOLD_START_TICKS) % HOLD_REPEAT_TICKS == 0U &&
            press_count[BUTTON_INCREASE] < 255U) {
            press_count[BUTTON_INCREASE]++;
        }
    } else {
        hold_ticks = 0;
    }
}


uint8_t button_take_press(uint8_t index)
{
    if (index >= NUMBER_OF_BUTTONS) return 0;

    uint32_t primask = __get_PRIMASK();
    __disable_irq();
    uint8_t pressed = press_count[index] != 0U;
    if (pressed) press_count[index]--;
    if (!primask) __enable_irq();
    return pressed;
}


void button_cancel_edit_events(void)
{
    uint32_t primask = __get_PRIMASK();
    __disable_irq();

    press_count[BUTTON_INCREASE] = 0;
    press_count[BUTTON_SET] = 0;
    hold_ticks = 0;

    if (stable[BUTTON_INCREASE] == GPIO_PIN_RESET ||
        HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_14) == GPIO_PIN_RESET) {
        ignore_increase_until_release = 1;
    }

    if (!primask)
        __enable_irq();
}


