/*
 * output_display.h
 *
 *  Created on: Oct 22, 2026
 *      Author: user
 */

#ifndef INC_OUTPUT_DISPLAY_H_
#define INC_OUTPUT_DISPLAY_H_

#include <stdint.h>

void display_set_lights(uint16_t lights);
void display_set_numbers(uint8_t main_value, uint8_t side_value);
void display_set_error(uint8_t mode);
void display_refresh(void);

#endif /* INC_OUTPUT_DISPLAY_H_ */
