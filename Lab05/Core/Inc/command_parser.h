#ifndef COMMAND_PARSER_H
#define COMMAND_PARSER_H

#include <stdint.h>

typedef enum {
    CMD_NONE,
    CMD_RST,
    CMD_OK
} CommandType;

void command_parser_receive_byte(uint8_t byte);
CommandType command_parser_fsm(void);

#endif
