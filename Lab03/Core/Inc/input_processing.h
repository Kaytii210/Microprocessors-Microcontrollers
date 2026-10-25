/*
 * input_processing.h
 *
 *  Created on: Oct 22, 2026
 *      Author: user
 */

#ifndef INC_INPUT_PROCESSING_H_
#define INC_INPUT_PROCESSING_H_

#include <stdint.h>

void app_init(void);
void app_tick_1ms(void);
void fsm_for_input_processing(void);
uint8_t get_current_mode(void);
#endif /* INC_INPUT_PROCESSING_H_ */
