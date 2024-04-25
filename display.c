#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <linux/i2c-dev.h>
#include <i2c/smbus.h>       
#include <byteswap.h>

#define I2C_BUS "/dev/i2c-2"  // Path to the I2C device
#define LCD_ADDRESS 0x3e      // Address of the LCD module

#define RGB_CHIP_ADR 0x62

int i2c_fd;
int i2c_rgb;

// Function to send command to LCD
void lcd_command(int fd, unsigned char command) {
    unsigned char buf[2];
    buf[0] = 0x80;    // Control byte for command
    buf[1] = command; // command to execute / bit to change
    if (write(fd, buf, 2) != 2) // write to display, 2 is size
    {
        perror("Write failed");
        exit(1);
    }
}
void write_data(int fd, int x, int y, unsigned char display_register, char data) {
    unsigned char buf[2]; // data to Display
    
    if(x == 1) {
        i2c_smbus_write_word_data(fd, 0x00, y + 0x80);
    } else if(x == 2) {
        i2c_smbus_write_word_data(fd, 0x00, y + 0xC0);
    }

    buf[0] = display_register; // Control byte for data
    buf[1] = data; // Char to display
    if (write(fd, buf, 2) != 2) // write to display, 2 is size
    {
        perror("Write failed \n");
        exit(1);
    }
}

void lcd_data(int fd, unsigned char data) {
    unsigned char buf[2]; // data to Display
    buf[0] = 0x40; // Control byte for data
    buf[1] = data; // Char to display
    if (write(fd, buf, 2) != 2) // write to display, 2 is size
    {
        perror("Write failed");
        exit(1);
    }
}
void turn_display_color(unsigned char rgb_register, unsigned char data) {
    unsigned char buf[2]; // data to Display
    buf[0] = rgb_register; // Control byte for data
    buf[1] = data;
    if (write(i2c_rgb, buf, 2) != 2) // write to display, 2 is size
    {
        printf("Write failed");
        exit(1);
    }
}

int display_init() {
    // Open the I2C bus
    if ((i2c_fd = open(I2C_BUS, O_RDWR)) < 0) {
        perror("Failed to open the i2c bus");
        exit(1);
    }
    // Set the I2C slave address (ioctl = input / output, Control?)
    if (ioctl(i2c_fd, I2C_SLAVE, LCD_ADDRESS) < 0) {
        perror("Failed to set the address");
        exit(1);
    }

    if((i2c_rgb = open(I2C_BUS, O_RDWR)) < 0) {
        perror("rgb open failed");
        exit(1);
    }
    if(ioctl(i2c_rgb, I2C_SLAVE, RGB_CHIP_ADR) < 0) {
        perror("rgb open failed 2");
        exit(1);
    }

     // Initialization sequence
    lcd_command(i2c_fd, 0x38); // 8-bit, 2-line mode
    lcd_command(i2c_fd, 0x39); // Enable 4-bit mode
    lcd_command(i2c_fd, 0x0c); // Display control
    lcd_command(i2c_fd, 0x01); // Clear display
    lcd_command(i2c_fd, 0x38); // Function set (8-bit)
    lcd_command(i2c_fd, 0x6c); // Follower control

    return(0);
}

void display_write_temperatur(double temperature) {
    char temp_text[20];
    sprintf(temp_text, "%.1fC", temperature);
    printf("%s \n", temp_text);
    for (int i = 0; i < strlen(temp_text); i++) {
        //lcd_data(i2c_fd, temp_text[i]);
        write_data(i2c_fd, 2, i, 0x40, temp_text[i]);
    }
}
void display_color_temperatur(int temp) {
    //turn on sleep mood
    turn_display_color(0x00, 0x00);
    if(temp > 25) {
        turn_display_color(0x08, 0x10);
    } else if(temp > 15 & temp < 25) {
        turn_display_color(0x08, 0x04);
    } else if(temp > 5 & temp < 15) {
        turn_display_color(0x08, 0x01);
    } else {
        turn_display_color(0x08, 0x15);
    }

}
void display_color_white() {
    turn_display_color(0x00, 0x00);
    turn_display_color(0x08, 0x15);
}
void display_write_ipaddress(const char* ip) {
    printf("%s \n", ip);
    for (int i = 0; i < strlen(ip); i++) {
        write_data(i2c_fd, 1, i, 0x40, ip[i]);
    }
}
void display_write_time(const char* time) {
    printf("%s \n", time);
    for(int i = 0; i < strlen(time); i++) {
        write_data(i2c_fd, 2, i + 11, 0x40, time[i]);
    }
}