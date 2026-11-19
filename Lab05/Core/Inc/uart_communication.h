#ifndef UART_COMMUNICATION_H
#define UART_COMMUNICATION_H

#include "stm32f1xx_hal.h"
#include "command_parser.h"

void uart_communication_init(UART_HandleTypeDef *uart,
                             ADC_HandleTypeDef *adc);

void uart_communication_fsm(CommandType command);

#endif
