/*
 * output_display.c
 *
 *  Created on: Oct 22, 2026
 *      Author: user
 */


#include "main.h"
#include "output_display.h"

// Segment order: PB0..PB6 = a,b,c,d,e,f,g (common-anode, active LOW).
static const uint8_t segment_code[12] = {
    0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D,
    0x7D, 0x07, 0x7F, 0x6F, 0x79, 0x50
    // 0..9, E, r
};

// PA12..PA15 are the four digit enables, active LOW via PNP.
static const uint16_t digit_pins[4] = {
    GPIO_PIN_12, GPIO_PIN_13, GPIO_PIN_14, GPIO_PIN_15
};

static uint8_t digits[4] = {0, 3, 0, 5};
static uint8_t current_digit = 0;



void display_set_lights(uint16_t lights)
{
    const uint16_t all = GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4 |
                         GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7;
    static uint16_t previous = 0xFFFFU;

    lights &= all;
    if (lights == previous) return;

    // Traffic LEDs are active LOW. Keep unchanged outputs stable.
    HAL_GPIO_WritePin(GPIOA, all & ~lights, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOA, lights, GPIO_PIN_RESET);

    previous = lights;
}

void display_set_numbers(uint8_t main_value, uint8_t side_value)
{
    if (main_value > 99U) main_value = 99U;
    if (side_value > 99U) side_value = 99U;

    digits[0] = main_value / 10U;
    digits[1] = main_value % 10U;
    digits[2] = side_value / 10U;
    digits[3] = side_value % 10U;
}

void display_set_error(uint8_t mode)
{
    digits[0] = 10;  // E
    digits[1] = 11;  // r
    digits[2] = 0;
    digits[3] = mode;
}

void display_refresh(void)
{
    uint8_t code = segment_code[digits[current_digit]];
    const uint16_t all_digits = GPIO_PIN_12 | GPIO_PIN_13 |
                                GPIO_PIN_14 | GPIO_PIN_15;
    const uint16_t all_segments = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 |
                                  GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5 |
                                  GPIO_PIN_6;

    // Disable all digits before changing the segment data.
    HAL_GPIO_WritePin(GPIOA, all_digits, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOB, all_segments, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOB, (uint16_t)(code & 0x7FU), GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOA, digit_pins[current_digit], GPIO_PIN_RESET);

    current_digit = (current_digit + 1U) % 4U;
}

