/*

    For use in Arduino projects. Functions in the SerialOpen namespace open a port for
    comunication using the device name and baud rate. Assumes default communication protocol
    (i.e. 8 data bits, no parity bit, one stop bit.) Returns a negative number if there was
    an error while opening a port.

    Information on serial initialization taken from:
    <https://blog.mbedded.ninja/programming/operating-systems/linux/linux-serial-ports-using-c-cpp/>.

*/

#ifndef _SERIAL_OPEN_BLOCK
#define _SERIAL_OPEN_BLOCK

#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <unistd.h>
#include <iostream>

namespace SerialOpen
{
    const long DEFAULT_BAUD_RATE = 9600;

    int        port(char  [], long); // Device, baud rate.
    inline int port(char d[]) { return port(d, DEFAULT_BAUD_RATE); }
}

#endif
