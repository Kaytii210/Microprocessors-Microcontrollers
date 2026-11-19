#include "uart_communication.h"
#include <stdio.h>
#include <string.h>

typedef enum {
    WAIT_RST,
    WAIT_ADC,
    WAIT_OK
} CommunicationState;

static CommunicationState state = WAIT_RST;

static UART_HandleTypeDef *uart_handle;
static ADC_HandleTypeDef *adc_handle;

static char packet[24];
static uint32_t last_send_tick = 0;

void uart_communication_init(UART_HandleTypeDef *uart,
                             ADC_HandleTypeDef *adc)
{
    uart_handle = uart;
    adc_handle = adc;
    state = WAIT_RST;
    last_send_tick = 0;
}

void uart_communication_fsm(CommandType command)
{
    switch (state) {
    case WAIT_RST:
        if (command == CMD_RST) {
            state = WAIT_ADC;
        }
        break;

    case WAIT_ADC: {
        if (HAL_ADC_PollForConversion(adc_handle, 1) != HAL_OK) {
            break;
        }

        uint32_t adc_value = HAL_ADC_GetValue(adc_handle);

        snprintf(packet, sizeof(packet), "!ADC=%lu#\r\n",
                 (unsigned long)adc_value);

        if (HAL_UART_Transmit(uart_handle, (uint8_t *)packet,
                              strlen(packet), 100) == HAL_OK) {
            last_send_tick = HAL_GetTick();
            state = WAIT_OK;
        }
        break;
    }

    case WAIT_OK:
        if (command == CMD_OK) {
            state = WAIT_RST;
        } else if (HAL_GetTick() - last_send_tick >= 3000) {
            // Send the same packet without reading ADC again.
            if (HAL_UART_Transmit(uart_handle, (uint8_t *)packet,
                                  strlen(packet), 100) == HAL_OK) {
                last_send_tick = HAL_GetTick();
            }
        }
        break;
    }
}
