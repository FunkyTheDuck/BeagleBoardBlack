/*
 * Module...: MCP9808example1.c
 * Version..: 0.1 (Beta - unfinished)
 * Author...: Henrik Thomsen/Mercantec <heth@mercantec.dk
 * Date.....: 24. nov 2020
 *
 * Abstract.: Implementation of Seeed Grove Sensor "I2C High
 *            Accuracy Temperature sensor" based on MCP-9808
 *            Implementet in user space using libi2c
 * Mod. log.: 01-06-2021: Could not compile. Got i2c_smbus_read_word_data undefined.
                              Include <i2c/smbus.h> should be include. (Worked in 2020)

 * License..: Free open software but WITHOUT ANY WARRANTY.
 * Terms....: see http://www.gnu.org/licenses
 *
 * Documentation - see https://mars.merhot.dk/w/index.php/Grove_I2C_High_Accuracy_Temerature_Sensor_-_Seeed
 *
 * REMEMBER: Install the libi2c-dev package to compile this file
 * Compile this example with: gcc i2c_MCP9808-example1.c -o i2c_MCP9808-example1 -li2c
*
 * See also: https://www.kernel.org/doc/Documentation/i2c/dev-interface
 */
#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <linux/i2c-dev.h>
#include <i2c/smbus.h>        // Added 01062021 HeTh
#include <byteswap.h>
#include "display.h"          // Added my own header file to write to display

/* The MCP9808 Digital temperature sensor IC from Microchip is a i2c/SMBus compatible
 * device. To understand this code you need access to MCP9808 Datasheet from Microchip
 *
 * The device is implementet on Seeed Studios Grove I2C high accuracy Temperature Sensor
 *
 * Links and supplemental information can be found on tthe documentation link in this
 * files header.
 *
 * The MCP9808 contains severel registers:
 *
 * Register addresses:
 *  0x00: RFU    - (Reserved for future use) Read only
 *  0x01: CONFIG - Configuration register
 *  0x02: Tupper - Alert temperature Upper boundary trip register
 *  0x03: Tlower - Alert temperature Lower boundary trip register
 *  0x04: Tcrit  - Critical temperature trip register
 *  0x05: Ta     - Temperature register
 *  0x06:        - Manufacurer ID register (Reads 0x54)
 *  0x07:        - Device ID/Revision register
 *  0x08:        - Resolution register
 *  0x09-0x0F    - Reserved....
 */

#define CONFIG_REG      0x01
#define TUPPER_REG      0x02
#define TLOWER_REG      0x03
#define TCRIT_REG       0x04
#define TA_REG          0x05
#define MANID_REG       0x06
#define DEVID_REG       0x07
#define RES_REG         0x08

//Device specific information. (Perhaps better as command options or config file)
#define MPC9808_BUS     "/dev/i2c-2"
#define MPC9808_ADR     0x18


#define DIS69420        0x3e
#define DISCOM          0x40



int i2c_init(char *bus, unsigned int address) {
        int file;

        file = open(bus, O_RDWR);
        if (file < 0) { // If error
                fprintf(stderr, "ERROR: opening %s - %s\n", bus, strerror(errno));
                exit(1);
        }

        if (ioctl(file, I2C_SLAVE, address) == -1 ) { // If error
             fprintf(stderr, "ERROR: setting  address %d on i2c bus %s with ioctl() - %s", address, bus, strerror(errno) );
             exit(1);
        }
        return(file);
}
/* TODO
 * anden c fil med read/write 8/16 og init
 * Der skal være konfigurerbar recovery mulighed returner uint8/16_t data direkte eller exit'er (Er bswapet)
 * Måske funktioner der returnerer int_32 med -1 ved fejl
 */
int main( void ) {
        char revision; //To store MCP9808 revision ID - Perhaps useful in the future
        int file;
        char buf[20];
        int32_t reg32;
        uint16_t * const reg16poi = (uint16_t *) &reg32; //Address reg32 wordwise
        uint8_t  * const reg8poi  = (uint8_t *)  &reg32; //Address reg32 bytewise
        file = i2c_init(MPC9808_BUS, MPC9808_ADR);

        // Read manufactorer ID
        // Note: i2c_smbus_read_word_data returns in big-endian
        //       bit 0-7 = reg8poi[1] and bit 8-15 = reg8poi[0]
        reg32 = i2c_smbus_read_word_data(file, MANID_REG);
        if ( reg32 < 0 ) {
                fprintf(stderr, "ERROR: Read failed  on i2c bus register %d - %s\n",  MANID_REG,strerror(errno) );
                exit(1);
        }
        if ( bswap_16(reg16poi[0]) != 0x0054 ) { // Check manufactorer ID - Big endian 5400 and not 0054
                fprintf(stderr, "Manufactorer ID wrong is 0x%x should be 0x54\n",__bswap_16(reg16poi[0]));
                exit(1);
        }
        // Read device ID and revision
        reg32 = i2c_smbus_read_word_data(file, DEVID_REG);
        if ( reg32 < 0 ) {
                fprintf(stderr, "ERROR: Read failed  on i2c bus register %d - %s\n",  DEVID_REG,strerror(errno) );
                exit(1);
        }
        if ( reg8poi[0] != 0x04 ) { // Check device ID - Big endian 0400 and not 0004
                fprintf(stderr, "Manufactorer ID OK but device ID wrong is 0x%x should be 0x4\n",reg8poi[0]);
                exit(1);
        }
        revision = reg8poi[1];
/*DEBUG*/       //printf("All good :-)\n");
        
        // Read temperature...

        //læser 16 bit fra den angivent register
        reg32 = i2c_smbus_read_word_data(file, TA_REG);
        //tjekker på om registen er større end 0
        if (reg32 < 0) {
            //printer en fejl ud til konsolen hvis den ikke hentede noget
            fprintf(stderr, "ERROR: Read failed on i2c bus register %d - %s\n", TA_REG, strerror(errno));
            exit(1);
        }
        //her konverter de 16 bit til little-endian format
        int temperature_raw = bswap_16(reg16poi[0]); 
        //sætter de 12 laveste bits ud af de 16 til denne double, fordi de første 4 ikke indeholder temperatur data og dividere med 16 fordi det er 16 bits
        double temperature = (temperature_raw & 0x0FFF) / 16.0;
        //printer temperaturen ud
        //printf("Temperature: %.1f°C\n", temperature);
        printf("%.1f \n", temperature);

        display_init();

        display_write_temperatur(temperature);
        display_color_temperatur(temperature);
}