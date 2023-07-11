#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>

#define UART4 "/dev/ttyO4"
#define BUFFER_SIZE 256
#define SERVER_URL "http://ballard.com/api/data"

int uart4_filestream = -1;

void setup_uart()
{
    uart4_filestream = open(UART4, O_RDWR | O_NOCTTY);
    if (uart4_filestream == -1)
    {
        printf("Failed to open UART.\n");
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

void read_sensor_data(float *fuel_consumption)
{
    if (uart4_filestream != -1)
    {
        unsigned char rx_buffer[BUFFER_SIZE];
        int rx_length = read(uart4_filestream, (void*)rx_buffer, BUFFER_SIZE - 1);

        if (rx_length < 0)
        {
            printf("UART RX error\n");
        }
        else if (rx_length == 0)
        {
            printf("No data received.\n");
        }
        else
        {
            rx_buffer[rx_length] = '\0';
            printf("%i bytes read : %s\n", rx_length, rx_buffer);
            
            // Assuming sensor data is a float string
            *fuel_consumption = atof(rx_buffer);
        }
    }
}

void send_data_over_ethernet(float fuel_consumption)
{
    char command[BUFFER_SIZE];

    snprintf(command, sizeof(command), "curl -X POST -H \"Content-Type: application/json\" -d '{\"fuel_consumption\":%f}' %s", fuel_consumption, SERVER_URL);
    system(command);
}

int main()
{
    float fuel_consumption;

    setup_uart();
    
    while (1)
    {
        read_sensor_data(&fuel_consumption);
        send_data_over_ethernet(fuel_consumption);

        sleep(1); // Wait for 1 second before reading again
    }
    
    close(uart4_filestream);
    
    return 0;
}
