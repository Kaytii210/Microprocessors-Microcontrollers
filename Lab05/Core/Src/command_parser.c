#include "command_parser.h"
#include <string.h>

#define RX_BUFFER_SIZE 64

static volatile uint8_t rx_buffer[RX_BUFFER_SIZE];
static volatile uint8_t rx_head = 0;
static volatile uint8_t rx_tail = 0;

typedef enum {
    WAIT_START,
    READ_COMMAND
} ParserState;

static ParserState state = WAIT_START;
static char command[8];
static uint8_t command_length = 0;

void command_parser_receive_byte(uint8_t byte)
{
    uint8_t next = (rx_head + 1) % RX_BUFFER_SIZE;

    if (next != rx_tail) {
        rx_buffer[rx_head] = byte;
        rx_head = next;
    }
}

CommandType command_parser_fsm(void)
{
    while (rx_tail != rx_head) {
        uint8_t byte = rx_buffer[rx_tail];
        rx_tail = (rx_tail + 1) % RX_BUFFER_SIZE;

        if (byte == '!') {
            state = READ_COMMAND;
            command_length = 0;
            continue;
        }

        if (state != READ_COMMAND) continue;

        if (byte == '#') {
            command[command_length] = '\0';
            state = WAIT_START;

            if (strcmp(command, "RST") == 0) return CMD_RST;
            if (strcmp(command, "OK") == 0) return CMD_OK;
        } else if (command_length < sizeof(command) - 1) {
            command[command_length++] = (char)byte;
        } else {
            state = WAIT_START;
        }
    }

    return CMD_NONE;
}
