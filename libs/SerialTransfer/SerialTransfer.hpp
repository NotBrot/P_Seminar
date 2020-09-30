#ifndef __SERIAL_DATA_H__
#define __SERIAL_DATA_H__

#include "Arduino.h"

// Serial timeout (in milliseconds)
#define TIMEOUT 3000UL

namespace SerialTransfer
{

// Serial buffer for SerialTransfer
static char serial_buf[64];

struct SerialData
{
    // If set additional data will be sent after the command (ends with '\n')
    unsigned char has_data : 1;

    unsigned char cmd : 7;

    SerialData() = default;
    SerialData(unsigned char cmd, bool has_data)
        : has_data(has_data), cmd(cmd) {}
    SerialData(unsigned char raw_data)
        : has_data(raw_data >> 7), cmd(raw_data) {}

    // Type conversion to raw data
    operator char() { return has_data >> 7 | cmd; };
};

struct SerialHandler
{
    // Command on which the handler gets called
    unsigned char on_cmd;

    // Funtion that gets called if command on_cmd received
    void (*handler_function)(void) = nullptr;

    SerialHandler() = default;
    SerialHandler(unsigned char on_cmd, void (*handler_function)(void))
        : on_cmd(on_cmd), handler_function(handler_function) {}
};

// Handles incoming commands
bool handle(Stream &port, SerialHandler *handler_list, int n_handler, void (*unknown_cmd_handler)(char));

// Parses data after command
bool parseData(Stream &port);

// Reads a single command
SerialData read(Stream &port);

// Writes a command
void write(Stream &port, SerialData &data);

// Returns current data in serial_buf
char *getBuffer();

} // namespace SerialTransfer

#endif // !__SERIAL_DATA_H__
