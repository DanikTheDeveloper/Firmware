#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>

#define UART_PATH "/dev/ttyO4"
#define MAX_SIZE 256
#define WEB_SERVER "http://ballard.com/api/data"

int uartConnection = -1;

void init_uart() // function to initialize UART
{
    uartConnection = open(UART_PATH, O_RDWR | O_NOCTTY);
    if (uartConnection == -1)
    {
        printf("Oops! Failed to open UART.\n");
    }

    struct termios options;
    tcgetattr(uartConnection, &options);
    options.c_cflag = B9600 | CS8 | CLOCAL | CREAD;   // setting baud rate
    options.c_iflag = IGNPAR;
    options.c_oflag = 0;
    options.c_lflag = 0;
    tcflush(uartConnection, TCIFLUSH);
    tcsetattr(uartConnection, TCSANOW, &options);
}

void read_data(float *sensor_data) // function to read sensor data
{
    if (uartConnection != -1)
    {
        unsigned char buffer[MAX_SIZE];
        int len = read(uartConnection, (void*)buffer, MAX_SIZE - 1);

        if (len < 0)
        {
            printf("UART RX error\n");
        }
        else if (len == 0)
        {
            printf("No data received.\n");
        }
        else
        {
            buffer[len] = '\0';
            printf("Read %i bytes : %s\n", len, buffer);
            
            // assuming sensor data is a float string
            *sensor_data = atof(buffer);
        }
    }
}

void send_data(float sensor_data) // function to send data over Ethernet
{
    char command[MAX_SIZE];

    snprintf(command, sizeof(command), "curl -X POST -H \"Content-Type: application/json\" -d '{\"sensor_data\":%f}' %s", sensor_data, WEB_SERVER);
    system(command);
}

int main()
{
    float sensor_data;

    init_uart(); // initialize UART
    
    while (1)
    {
        read_data(&sensor_data); // read sensor data
        send_data(sensor_data); // send data

        sleep(1); // wait for 1 second before reading again
    }
    
    close(uartConnection); // close UART connection
    
    return 0;
}
