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

int temp_init(char *bus, unsigned int address) {
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
//function til at hente temperaturen
double temp_get() {
    char revision; //To store MCP9808 revision ID - Perhaps useful in the future
        int file;
        char buf[20];
        int32_t reg32;
        uint16_t * const reg16poi = (uint16_t *) &reg32; //Address reg32 wordwise
        uint8_t  * const reg8poi  = (uint8_t *)  &reg32; //Address reg32 bytewise
        file = temp_init(MPC9808_BUS, MPC9808_ADR);

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
        return temperature;
}