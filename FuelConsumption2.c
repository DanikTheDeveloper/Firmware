#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <cstring>

constexpr auto UART4 = "/dev/ttyO4";
constexpr auto BUFFER_SIZE = 256;
constexpr auto SERVER_URL = "http://ballard.com/api/data";

int uart4_filestream = -1;

void setup_uart()
{
    uart4_filestream = open(UART4, O_RDWR | O_NOCTTY);
    if (uart4_filestream == -1)
    {
        std::cerr << "Failed to open UART." << std::endl;
        return;
    }

    struct termios options;
    tcgetattr(uart4_filestream, &options);
    options.c_cflag = B9600 | CS8 | CLOCAL | CREAD;   // Set baud rate
    options.c_iflag = IGNPAR;
    options.c_oflag = 0;
    options.c_lflag = 0;
    tcflush(uart4_filestream, TCIFLUSH);
    tcsetattr(uart4_filestream, TCSANOW, &options);
}

void read_sensor_data(float &fuel_consumption)
{
    if (uart4_filestream != -1)
    {
        char rx_buffer[BUFFER_SIZE];
        int rx_length = read(uart4_filestream, static_cast<void*>(rx_buffer), BUFFER_SIZE - 1);

        if (rx_length < 0)
        {
            std::cerr << "UART RX error" << std::endl;
        }
        else if (rx_length == 0)
        {
            std::cerr << "No data received." << std::endl;
        }
        else
        {
            rx_buffer[rx_length] = '\0';
            std::cout << rx_length << " bytes read : " << rx_buffer << std::endl;
            
            // Assuming sensor data is a float string
            fuel_consumption = std::stof(rx_buffer);
        }
    }
}

void send_data_over_ethernet(float fuel_consumption)
{
    std::stringstream command;
    command << "curl -X POST -H \"Content-Type: application/json\" -d '{\"fuel_consumption\":" << fuel_consumption << "}' " << SERVER_URL;
    system(command.str().c_str());
}

int main()
{
    float fuel_consumption;

    setup_uart();
    
    while (1)
    {
        read_sensor_data(fuel_consumption);
        send_data_over_ethernet(fuel_consumption);

        sleep(1); // Wait for 1 second before reading again
    }
    
    close(uart4_filestream);
    
    return 0;
}
