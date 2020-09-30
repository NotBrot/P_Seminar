#include "SerialTransfer.hpp"

bool SerialTransfer::handle(Stream &port, SerialHandler *handler_list, int n_handler, void (*unknown_cmd_handler)(char))
{
    bool error = false;

    SerialData data;

    while (port.available() > 0)
    {
        // Get command
        data = read(port);

        if (n_handler > 0)
        {
            SerialHandler serial_handler;

            // Find correct handler function
            for (int i = 0; i < n_handler; i++)
            {
                if (handler_list[i].on_cmd == data.cmd)
                {
                    serial_handler = handler_list[i];
                    break;
                }
            }

            if (serial_handler.handler_function && !data.has_data)
            {
                // Handler hasn't got extra data, simply call the handler
                serial_handler.handler_function();
            }
            else if (serial_handler.handler_function && data.has_data)
            {
                if (!parseData(port))
                    // Error parsing data
                    error = true;
            }
            else
            {
                // No Handler found
                unknown_cmd_handler(data);
                error = true;
            }
        }
        else
        {
            // No handler specified, call unknown cmd handler
            unknown_cmd_handler(data);
        }
        
    }

    return !error;
}

bool SerialTransfer::parseData(Stream &port)
{
    // millis() of last received char
    unsigned long last_receive = millis();

    char in_char;

    // Pointer to current index of serial_buf
    static char *serial_ptr = serial_buf;

    // // Reset serial_ptr
    // serial_ptr = serial_buf;

    for (;;)
    {
        // Wait for incoming data
        while (port.available() <= 0)
            // Check if timeout reached
            if (millis() - last_receive > TIMEOUT)
                return false;

        in_char = port.read();
        last_receive = millis();

        // Check for end of data
        if (in_char == '\n')
        {
            // Terminate string
            *serial_ptr = 0;

            break;
        }

        // Append to serial_buf and increment serial_ptr
        *serial_ptr++ = in_char;

        // Check if serial_ptr in bounds
        if (serial_ptr > serial_buf + sizeof(serial_buf))
            return false;
    }

    // Check if string empty
    if (serial_buf[0] == 0)
        return false;

    return true;
}

SerialTransfer::SerialData SerialTransfer::read(Stream &port)
{
    SerialData data;

    if (port.available() > 0)
        // If data is available, use implicit type conversion from int to SerialData
        data = port.read();

    return data;
}

void SerialTransfer::write(Stream &port, SerialData &data)
{
    // Write data to port using implicit type conversion from SerialData to char
    port.write(data);
}

char *SerialTransfer::getBuffer()
{
    return serial_buf;
}
