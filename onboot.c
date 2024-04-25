#include <stdio.h>
#include <unistd.h>
//include til mine header filer som jeg har lavet 
#include "display.h"
#include "ethernet.h"
#include "temperatur.h"
#include "timer.h"

//main function som bliver kørt at daemon-service for at køre på startup
int main() {
    //kalder 2 methoder for at initialize displayet også ændre dens baggrund farve til hvid
    display_init();
    display_color_white();
    //opretter en char array og henter dens værdi fra min header fil "ethernet.h"
    char* ip = eth0_ip_get();
    //når boarded starter op har den ikke nogen ip addresse, så for at sikre os at den printer en ip addresse up
    //køre et while loop som tjekker på om den er null, hvis den er kører den igen efter 1 sekundt
    while(ip == NULL) {
        ip = eth0_ip_get();
        sleep(1);
    }
    //så kalder den en function til at printe ip addressen ud
    display_write_ipaddress(ip);

    //henter temperaturen
    double temperatur = temp_get();
    //hvis temperaturen ikke er 0 så printer vi den ud
    if(temperatur != 0) {
        display_write_temperatur(temperatur);
    } else {
        printf("Failed to retrieve temperatur");
    }
    int show_colon = 1;
    //køre et while loop som køre for evigt for at hente tiden hele tiden
    while(1) {
        //henter tiden
        char* time = time_get();
        //laver et if statement til at tjekke om ":" skal vises eller ikke
        if(show_colon == 1) {
            int i = 0, j = 0;
            while (time[i]) {
                if (time[i] != ':') {
                    time[j++] = time[i];
                } else {
                    time[j++] = ' ';
                }
                i++;
            }
            time[j] = '\0';
            show_colon = 2;
        } else {
            show_colon = 1;
        }
        //printer tiden ud til displayet
        display_write_time(time);
        sleep(1);
    }

    return 0;
}