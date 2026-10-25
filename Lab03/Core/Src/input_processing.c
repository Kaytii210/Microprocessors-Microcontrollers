/*
 * input_processing.c
 *
 *  Created on: Oct 22, 2026
 *      Author: user
 */


#include "main.h"
#include "input_reading.h"
#include "input_processing.h"
#include "output_display.h"

typedef enum {
    MODE_NORMAL = 1,
    MODE_RED,
    MODE_AMBER,
    MODE_GREEN
} ModeState;

typedef enum {
    NS_GREEN = 0,
    NS_AMBER,
    EW_GREEN,
    EW_AMBER
} TrafficState;

typedef struct {
    uint8_t red;
    uint8_t amber;
    uint8_t green;
} TrafficTime;

#define SIDE_AMBER_TIME 2U

static volatile ModeState mode = MODE_NORMAL;
static volatile TrafficState phase = NS_GREEN;
static volatile uint32_t phase_ms = 0;
static volatile uint16_t blink_ms = 0;
static volatile uint16_t error_ms = 0;
static volatile uint8_t edit_value = 5;

// Active values control the current traffic cycle.
// Pending values are applied at the start of the NEXT cycle.
static TrafficTime active = {5, 2, 3};
static TrafficTime pending = {5, 2, 3};

static uint8_t phase_seconds(TrafficState state)
{
    switch (state) {
    case NS_GREEN: return active.green;
    case NS_AMBER: return active.amber;
    case EW_GREEN: return active.red - SIDE_AMBER_TIME;
    case EW_AMBER: return SIDE_AMBER_TIME;
    default: return 1;
    }
}

static uint8_t seconds_left(uint32_t milliseconds)
{
    return (uint8_t)((milliseconds + 999U) / 1000U);
}

static void show_normal(void)
{
    uint32_t ms = phase_ms;
    uint32_t main_left = 0;
    uint32_t side_left = 0;
    uint16_t lights = 0;

    switch (phase) {
    case NS_GREEN:
        lights = GPIO_PIN_4 | GPIO_PIN_5; // LGx, LRy
        main_left = (uint32_t)active.green * 1000U - ms;
        side_left = (uint32_t)(active.green + active.amber) * 1000U - ms;
        break;

    case NS_AMBER:
        lights = GPIO_PIN_3 | GPIO_PIN_5; // LYx, LRy
        main_left = (uint32_t)active.amber * 1000U - ms;
        side_left = main_left;
        break;

    case EW_GREEN:
        lights = GPIO_PIN_2 | GPIO_PIN_7; // LRx, LGy
        main_left = (uint32_t)active.red * 1000U - ms;
        side_left = (uint32_t)(active.red - SIDE_AMBER_TIME) * 1000U - ms;
        break;

    case EW_AMBER:
        lights = GPIO_PIN_2 | GPIO_PIN_6; // LRx, LYy
        main_left = (uint32_t)SIDE_AMBER_TIME * 1000U - ms;
        side_left = main_left;
        break;
    }

    display_set_lights(lights);
    display_set_numbers(seconds_left(main_left), seconds_left(side_left));
}

void app_init(void)
{
    display_set_lights(GPIO_PIN_4 | GPIO_PIN_5);
    display_set_numbers(3, 5);
}

// Called by TIM3 every 1 ms.
void app_tick_1ms(void)
{
    if (mode == MODE_NORMAL) {
        phase_ms++;
        if (phase_ms >= (uint32_t)phase_seconds(phase) * 1000U) {
            phase_ms = 0;
            if (phase == EW_AMBER) {
                phase = NS_GREEN;
                active = pending; // Apply saved settings at a cycle boundary.
            } else {
                phase = (TrafficState)(phase + 1);
            }
        }
        show_normal();
    } else {
        blink_ms++;
        if (blink_ms >= 500U) blink_ms = 0;

        uint16_t lights = 0;
        if (blink_ms < 250U) {
            if (mode == MODE_RED)   lights = GPIO_PIN_2 | GPIO_PIN_5;
            if (mode == MODE_AMBER) lights = GPIO_PIN_3 | GPIO_PIN_6;
            if (mode == MODE_GREEN) lights = GPIO_PIN_4 | GPIO_PIN_7;
        }
        display_set_lights(lights);

        if (error_ms > 0U) {
            error_ms--;
            display_set_error((uint8_t)mode);
        } else {
            display_set_numbers(edit_value, (uint8_t)mode);
        }
    }

    display_refresh();
}


void fsm_for_input_processing(void)
{
    // MODE has the highest priority
    if (button_take_press(BUTTON_MODE)) {
        button_cancel_edit_events();

        uint32_t primask = __get_PRIMASK();
        __disable_irq();

        mode = (mode == MODE_GREEN) ?
               MODE_NORMAL : (ModeState)(mode + 1);

        blink_ms = 0;
        error_ms = 0;

        if (mode == MODE_RED)
            edit_value = pending.red;

        if (mode == MODE_AMBER)
            edit_value = pending.amber;

        if (mode == MODE_GREEN)
            edit_value = pending.green;

        if (!primask)
            __enable_irq();

        return;
    }

    // SET has priority over INCREASE
    if (button_take_press(BUTTON_SET)) {
        button_take_press(BUTTON_INCREASE);

        if (mode == MODE_NORMAL)
            return;

        TrafficTime next = pending;

        if (mode == MODE_RED)
            next.red = edit_value;

        if (mode == MODE_AMBER)
            next.amber = edit_value;

        if (mode == MODE_GREEN)
            next.green = edit_value;

        // EW green = NS red - EW amber (2s)
        // EW red = NS green + NS amber
        uint8_t valid =
            (next.red > SIDE_AMBER_TIME) &&
            ((uint16_t)next.green + next.amber <= 99U);

        uint32_t primask = __get_PRIMASK();
        __disable_irq();

        if (valid)
            pending = next;
        else
            error_ms = 1000U;

        if (!primask)
            __enable_irq();

        return;
    }

    // Increase the selected value
    if (button_take_press(BUTTON_INCREASE) &&
        mode != MODE_NORMAL) {

        uint32_t primask = __get_PRIMASK();
        __disable_irq();

        edit_value = (edit_value >= 99U) ?
                     1U : edit_value + 1U;

        error_ms = 0;

        if (!primask)
            __enable_irq();
    }
}


uint8_t get_current_mode(void)
{
    return (uint8_t)mode;
}

