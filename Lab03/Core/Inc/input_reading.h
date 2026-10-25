/*
 * input_reading.h
 *
 *  Created on: Oct 22, 2026
 *      Author: user
 */

#ifndef INC_INPUT_READING_H_
#define INC_INPUT_READING_H_

#include <stdint.h>

#define BUTTON_MODE      0
#define BUTTON_INCREASE  1
#define BUTTON_SET       2

void button_reading(void);
uint8_t button_take_press(uint8_t index);
void button_cancel_edit_events(void);

#endif /* INC_INPUT_READING_H_ */
