#include "SerialOpen.hpp"

void setSerialPortFlags(struct termios&, long);

int SerialOpen::port(char device[], long baudRate)
{
    // Tries opening the device for reading and writing.
    int port = open(device, O_RDWR);
    if (port < 0)
    {
        std::cerr << "Error "<< errno << " from open: " << strerror(errno) << std::endl;
        return -1;
    }

    // Tries to create a settings struct from the defaults.
    struct termios portSettings;
    memset(&portSettings, 0, sizeof portSettings);
    if(tcgetattr(port, &portSettings) != 0)
    {
        std::cerr << "Error "<< errno << " from tcgetattr: " << strerror(errno) << std::endl;
        return -2;
    }

    // Tries to change settings and apply them to port.
    setSerialPortFlags(portSettings, baudRate);
    if (tcsetattr(port, TCSANOW, &portSettings) != 0)
    {
        std::cerr << "Error "<< errno << " from tcsetattr: " << strerror(errno) << std::endl;
        return -3;
    }

    // If no errors occur.
    return port;
}

void setSerialPortFlags(struct termios& p, long baudRate)
{
    /* Control modes. */
    p.c_cflag &= ~PARENB;  // No parity.
    p.c_cflag &= ~CSTOPB;  // One stop bit.
    p.c_cflag |= CS8;      // 8 bits / byte.
    p.c_cflag &= ~CRTSCTS; // Disable hardware flow control.
    p.c_cflag |= CREAD;    // Turn on read.
    p.c_cflag |= CLOCAL;   // Ignore control lines.

    /* Local modes. */
    p.c_lflag &= ~ICANON;  // Disable canonical mode.
    p.c_lflag &= ~ECHO;    // Disable echo.
    p.c_lflag &= ~ECHOE;   // Disable erasure.
    p.c_lflag &= ~ECHONL;  // Disable new-line echo.
    p.c_lflag &= ~ISIG;    // Disable interpretation of INTR, QUIT and SUSP.

    /* Input modes. */
    // Turn off software flow control.
    p.c_iflag &= ~(IXON | IXOFF | IXANY);
    // Disable any special handling of received bytes
    p.c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL);

    /* Output modes. */
    p.c_oflag &= ~OPOST;   // Prevent special interpretation of output bytes.
    p.c_oflag &= ~ONLCR;   // Prevent conversion of newline to CR/LF.

    /* Waits indefinitely for one byte. */
    p.c_cc[VMIN]  = 1;
    p.c_cc[VTIME] = 0;

    /* Sets baud rate. */
    cfsetispeed(&p, baudRate);
    cfsetospeed(&p, baudRate);
}
