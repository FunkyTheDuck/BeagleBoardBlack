#include <stdio.h>
#include <stdlib.h>
#include <time.h>

//henter tiden fra UTC-2
char* time_get() {
    time_t currentTime;
    struct tm *localTime;
    static char timeString[6]; 
    //sætter tiden til UTC-2
    putenv("TZ=UTC-2");
    //henter tiden 
    time(&currentTime);
    localTime = localtime(&currentTime);
    //formatter stringen til kun timer og minutter
    strftime(timeString, sizeof(timeString), "%H:%M", localTime);
    return timeString;
}