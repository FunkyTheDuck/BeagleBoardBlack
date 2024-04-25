#include <stdio.h>
#include <unistd.h>
#include "display.h"
#include "ethernet.h"
#include "temperatur.h"
#include "timer.h"


int main() {
    display_init();
    display_color_white();
    char* ip = eth0_ip_get();
    while(ip == NULL) {
        ip = eth0_ip_get();
        sleep(1);
    }
    display_write_ipaddress(ip);

    double temperatur = temp_get();
    if(temperatur != 0) {
        display_write_temperatur(temperatur);
    } else {
        printf("Failed to retrieve temperatur");
    }
    int show_colon = 1;
    while(1) {
        char* time = time_get();
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

        display_write_time(time);
        sleep(1);
    }

    return 0;
}